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
/*
* --------------------------------------------------------------------------------
* --------------------------------------------------------------------------------
*
* This class uses the SDIO interface to communication with the uSD card. It follows
* as similar desing as the SDIOuSDBlockDevice class to ease switching between the two
* interfaces. Note that this is not portable but rather requires mapping the hooks
* to the calls for each board provided library.
*
* --------------------------------------------------------------------------------
* --------------------------------------------------------------------------------
*/
#ifndef MBED_SDIOUSD_BLOCK_DEVICE_H 
#define MBED_SDIOUSD_BLOCK_DEVICE_H


#include "BlockDevice.h"
#include "sd_diskio.h"
#include "mbed.h"


/** Access an SD Card using SDIO [mbed-debug]: modify the example after completion
 *
 * @code
 * #include "mbed.h"
 * #include "SDIOuSDBlockDevice.h"
 *
 * SDIOuSDBlockDevice sd(p5, p6, p7, p12); // mosi, miso, sclk, cs
 * uint8_t block[512] = "Hello World!\n";
 *
 * int main() {
 *     sd.init();
 *     sd.write(block, 0, 512);
 *     sd.read(block, 0, 512);
 *     printf("%s", block);
 *     sd.deinit();
 * }
 */
class SDIOuSDBlockDevice : public BlockDevice {
public:
    /** Lifetime of an SD card
     */
    SDIOuSDBlockDevice();
    virtual ~SDIOuSDBlockDevice();

    /** Initialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual int init();

    /** Deinitialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual int deinit();

    /** Read blocks from a block device
     *
     *  @param buffer   Buffer to write blocks to
     *  @param addr     Address of block to begin reading from
     *  @param size     Size to read in bytes, must be a multiple of read block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int read(void *buffer, bd_addr_t addr, bd_size_t size);

    /** Program blocks to a block device
     *
     *  The blocks must have been erased prior to being programmed
     *
     *  @param buffer   Buffer of data to write to blocks
     *  @param addr     Address of block to begin writing to
     *  @param size     Size to write in bytes, must be a multiple of program block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int program(const void *buffer, bd_addr_t addr, bd_size_t size);

    /** Erase blocks on a block device
     *
     *  The state of an erased block is undefined until it has been programmed
     *
     *  @param addr     Address of block to begin erasing
     *  @param size     Size to erase in bytes, must be a multiple of erase block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int erase(bd_addr_t addr, bd_size_t size);

    /** Get the size of a readable block
     *
     *  @return         Size of a readable block in bytes
     */
    virtual bd_size_t get_read_size() const;

    /** Get the size of a programable block
     *
     *  @return         Size of a programable block in bytes
     *  @note Must be a multiple of the read size
     */
    virtual bd_size_t get_program_size() const;

    /** Get the size of a eraseable block
     *
     *  @return         Size of a eraseable block in bytes
     *  @note Must be a multiple of the program size
     */
    virtual bd_size_t get_erase_size() const;

    /** Get the total size of the underlying device
     *
     *  @return         Size of the underlying device in bytes
     */
    virtual bd_size_t size() const;

    /** Enable or disable debugging
     *
     *  @param          State of debugging
     */
    virtual void debug(bool dbg);

private:
    uint32_t _sectors;

    uint32_t _init_sck;
    uint32_t _transfer_sck;

    //SPI _spi;
    //DigitalOut _cs;
    unsigned _block_size;
    bool _is_initialized;
    bool _dbg;
    //[mbed-debug]: Added the member below to ease collecting SD card information
    BSP_SD_CardInfo SDIO_CardInfo;
    
//[mbed-debug]: This modification is made to enable this class to be used
// both for mbed-sdk & and mbed-os, it maps the mutex according to the 
// macro indicating the presence of an OS. Check <platform/PlatformMutex.h>
// for details.
#ifdef MBED_CONF_RTOS_PRESENT
    Mutex _lock;
#else
    PlatformMutex _lock;
#endif
};


#endif  /* MBED_SDIOUSD_BLOCK_DEVICE_H */
