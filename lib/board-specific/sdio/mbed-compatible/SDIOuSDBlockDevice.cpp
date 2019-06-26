/* mbed Microcontroller Library
 * Copyright (c) 2006-2012 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* Introduction
 * ------------
 * SD and MMC cards support a number of interfaces, but common to them all
 * is one based on SPI. This is the one I'm implmenting because it means
 * it is much more portable even though not so performant, and we already
 * have the mbed SPI Interface!
 *
 * The main reference I'm using is Chapter 7, "SPI Mode" of:
 *  http://www.sdcard.org/developers/tech/sdcard/pls/Simplified_Physical_Layer_Spec.pdf
 *
 * SPI Startup
 * -----------
 * The SD card powers up in SD mode. The SPI interface mode is selected by
 * asserting CS low and sending the reset command (CMD0). The card will
 * respond with a (R1) response.
 *
 * CMD8 is optionally sent to determine the voltage range supported, and
 * indirectly determine whether it is a version 1.x SD/non-SD card or
 * version 2.x. I'll just ignore this for now.
 *
 * ACMD41 is repeatedly issued to initialise the card, until "in idle"
 * (bit 0) of the R1 response goes to '0', indicating it is initialised.
 *
 * You should also indicate whether the host supports High Capicity cards,
 * and check whether the card is high capacity - i'll also ignore this
 *
 * SPI Protocol
 * ------------
 * The SD SPI protocol is based on transactions made up of 8-bit words, with
 * the host starting every bus transaction by asserting the CS signal low. The
 * card always responds to commands, data blocks and errors.
 *
 * The protocol supports a CRC, but by default it is off (except for the
 * first reset CMD0, where the CRC can just be pre-calculated, and CMD8)
 * I'll leave the CRC off I think!
 *
 * Standard capacity cards have variable data block sizes, whereas High
 * Capacity cards fix the size of data block to 512 bytes. I'll therefore
 * just always use the Standard Capacity cards with a block size of 512 bytes.
 * This is set with CMD16.
 *
 * You can read and write single blocks (CMD17, CMD25) or multiple blocks
 * (CMD18, CMD25). For simplicity, I'll just use single block accesses. When
 * the card gets a read command, it responds with a response token, and then
 * a data token or an error.
 *
 * SPI Command Format
 * ------------------
 * Commands are 6-bytes long, containing the command, 32-bit argument, and CRC.
 *
 * +---------------+------------+------------+-----------+----------+--------------+
 * | 01 | cmd[5:0] | arg[31:24] | arg[23:16] | arg[15:8] | arg[7:0] | crc[6:0] | 1 |
 * +---------------+------------+------------+-----------+----------+--------------+
 *
 * As I'm not using CRC, I can fix that byte to what is needed for CMD0 (0x95)
 *
 * All Application Specific commands shall be preceded with APP_CMD (CMD55).
 *
 * SPI Response Format
 * -------------------
 * The main response format (R1) is a status byte (normally zero). Key flags:
 *  idle - 1 if the card is in an idle state/initialising
 *  cmd  - 1 if an illegal command code was detected
 *
 *    +-------------------------------------------------+
 * R1 | 0 | arg | addr | seq | crc | cmd | erase | idle |
 *    +-------------------------------------------------+
 *
 * R1b is the same, except it is followed by a busy signal (zeros) until
 * the first non-zero byte when it is ready again.
 *
 * Data Response Token
 * -------------------
 * Every data block written to the card is acknowledged by a byte
 * response token
 *
 * +----------------------+
 * | xxx | 0 | status | 1 |
 * +----------------------+
 *              010 - OK!
 *              101 - CRC Error
 *              110 - Write Error
 *
 * Single Block Read and Write
 * ---------------------------
 *
 * Block transfers have a byte header, followed by the data, followed
 * by a 16-bit CRC. In our case, the data will always be 512 bytes.
 *
 * +------+---------+---------+- -  - -+---------+-----------+----------+
 * | 0xFE | data[0] | data[1] |        | data[n] | crc[15:8] | crc[7:0] |
 * +------+---------+---------+- -  - -+---------+-----------+----------+
 */



#include "SDIOuSDBlockDevice.h"
#include "mbed_debug.h"

#define SD_COMMAND_TIMEOUT 5000

#define SD_DBG             0

#define SD_BLOCK_DEVICE_ERROR_WOULD_BLOCK        -5001	/*!< operation would block */
#define SD_BLOCK_DEVICE_ERROR_UNSUPPORTED        -5002	/*!< unsupported operation */
#define SD_BLOCK_DEVICE_ERROR_PARAMETER          -5003	/*!< invalid parameter */
#define SD_BLOCK_DEVICE_ERROR_NO_INIT            -5004	/*!< uninitialized */
#define SD_BLOCK_DEVICE_ERROR_NO_DEVICE          -5005	/*!< device is missing or not connected */
#define SD_BLOCK_DEVICE_ERROR_WRITE_PROTECTED    -5006	/*!< write protected */

SDIOuSDBlockDevice::SDIOuSDBlockDevice()
{
    //_cs = 1;
    _is_initialized = 0;
    // Set default to 100kHz for initialisation and 1MHz for data transfer
    _init_sck = 100000; //[mbed-debug]: check again if these are needed at all
    _transfer_sck = 1000000;
}

SDIOuSDBlockDevice::~SDIOuSDBlockDevice()
{
    if (_is_initialized) {
        deinit();
    }
}

#define R1_IDLE_STATE           (1 << 0)
#define R1_ERASE_RESET          (1 << 1)
#define R1_ILLEGAL_COMMAND      (1 << 2)
#define R1_COM_CRC_ERROR        (1 << 3)
#define R1_ERASE_SEQUENCE_ERROR (1 << 4)
#define R1_ADDRESS_ERROR        (1 << 5)
#define R1_PARAMETER_ERROR      (1 << 6)

// Types
//  - v1.x Standard Capacity
//  - v2.x Standard Capacity
//  - v2.x High Capacity
//  - Not recognised as an SD Card
#define SDCARD_FAIL 0
#define SDCARD_V1   1
#define SDCARD_V2   2
#define SDCARD_V2HC 3


int SDIOuSDBlockDevice::init()
{
    BYTE pdrv = 0;
    // map the initialization from sd_diskio.c 
    _lock.lock();
    int err;

    if(SD_initialize(pdrv)){
        err = BD_ERROR_OK;
    }

    _is_initialized = (err == BD_ERROR_OK);
    if (!_is_initialized) {
        debug_if(_dbg, "Fail to initialize card\n");
        _lock.unlock();
        return err;
    }

    // Get the # of blocks for the SD card. This is specific to the STMF479I-Evaluation board
    BSP_SD_GetCardInfo(&this->SDIO_CardInfo);
    _sectors =  SDIO_CardInfo.BlockNbr;
    
    _lock.unlock();
    return BD_ERROR_OK;
}

int SDIOuSDBlockDevice::deinit()
{
    return 0;
}

//[mbed-debug]: does not appear to be used, will be commented and left empty
int SDIOuSDBlockDevice::program(const void *b, bd_addr_t addr, bd_size_t size)
{
    return 0;
    /*
    if (!is_valid_program(addr, size)) {
        return SD_BLOCK_DEVICE_ERROR_PARAMETER;
    }

    _lock.lock();
    if (!_is_initialized) {
        _lock.unlock();
        return SD_BLOCK_DEVICE_ERROR_NO_INIT;
    }

    const uint8_t *buffer = static_cast<const uint8_t*>(b);
    while (size > 0) {
        bd_addr_t block = addr / 512;
        // set write address for single block (CMD24)
        if (_cmd(24, block * _block_size) != 0) {
            _lock.unlock();
            return BD_ERROR_DEVICE_ERROR;
        }

        // send the data block
        _write(buffer, 512);
        buffer += 512;
        addr += 512;
        size -= 512;
    }
    _lock.unlock();
    return 0;
    */
}

int SDIOuSDBlockDevice::read(void *b, bd_addr_t addr, bd_size_t size)
{
    // [mbed-debug]: To avoid modifying mbed's source code (i.e., becuase of pdrv variable
    // which is not used), the mapping will be directly to the BSP_* c functions
    // the below is equivalent to SD_read.
    DRESULT res = RES_ERROR;

    if(BSP_SD_ReadBlocks((uint32_t*)b,
                       (uint32_t) (addr),
                       size, SDMMC_DATATIMEOUT) == MSD_OK)
    {
        /* wait until the read operation is finished */
        while(BSP_SD_GetCardState()!= MSD_OK)
        {
        }
        res = RES_OK;
    }

    return res;
    /*
    if (!is_valid_read(addr, size)) {
        return SD_BLOCK_DEVICE_ERROR_PARAMETER;
    }

    _lock.lock();
    if (!_is_initialized) {
        _lock.unlock();
        return SD_BLOCK_DEVICE_ERROR_PARAMETER;
    }
    
    uint8_t *buffer = static_cast<uint8_t *>(b);
    while (size > 0) {
        bd_addr_t block = addr / 512;
        // set read address for single block (CMD17)
        if (_cmd(17, block * _block_size) != 0) {
            _lock.unlock();
            return BD_ERROR_DEVICE_ERROR;
        }
        
        // receive the data
        _read(buffer, 512);
        buffer += 512;
        addr += 512;
        size -= 512;
    }
    _lock.unlock();
    return 0;
    */
}

//[mbed-debug]: The methods below appear to be for mbed checks when using SPI.
// as they ensure the size is 512. They were modified to refler the actual values
// of the SD card except functions unused or ones their use is not clear at the moment
// like get_program_size()

int SDIOuSDBlockDevice::erase(bd_addr_t addr, bd_size_t size)
{
    return 0;
}

bd_size_t SDIOuSDBlockDevice::get_read_size() const
{
    return 512;
}

bd_size_t SDIOuSDBlockDevice::get_program_size() const
{
    return 512;
}

bd_size_t SDIOuSDBlockDevice::get_erase_size() const
{
    return 512;
}

//[mbed-debug]: modified to reflect actual size of the SD card. Could cause
// errors as mbed has fixed sizes in the above functions.
bd_size_t SDIOuSDBlockDevice::size() const
{
    /*bd_size_t sectors = 0;
    if(_is_initialized) {
    	sectors = _sectors;
    }
    return 512*sectors;
    */
    bd_size_t sectors = 0;
    if(_is_initialized) {
        sectors = _sectors;
    }
    return sectors*(this->SDIO_CardInfo.BlockSize);
}

void SDIOuSDBlockDevice::debug(bool dbg)
{
    _dbg = dbg;
}




