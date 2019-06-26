/* This is a simple atcp-echo server using mbed-os
*/

#include "bootloader.h"
#include "mbed.h"
#include "TCPSocket.h"
#include "EthernetInterface.h"
#include "TCPSocket.h"
#include "SocketAddress.h"

#if defined TARGET_STM32F407
    #define POST_APPLICATION_ADDR 0x08040000  //Gives 128KB for bootloader
#elif defined TARGET_MCU_K64F
   #define POST_APPLICATION_ADDR 0x00020000  //Gives 128KB for bootloader
#elif defined TARGET_STM32F469NI
      #define POST_APPLICATION_ADDR 0x08040000  //Gives 256KB for bootloader
#else
    #error  POST_APPLICATION_ADDR must be defined for target
#endif


#define INIT_TOKEN 0x74696e69
#define FAIL_TOKEN 0x6c696166
#define DONE_TOKEN 0x656e6f64
#define TOKEN_NUM_BYTES 4


Serial pc(USBTX, USBRX, NULL, 9600);
__attribute__((section(".SECURE_DATA"))) FlashIAP flash;


enum LOADER_STATE{LD_ENTRY, LD_GET_PAGE, \
                         LD_WRITE_PAGE, LD_DONE, LD_ERROR};

bool init_socket(EthernetInterface & eth, TCPSocket &socket, \
	               TCPServer &server, SocketAddress &sockaddr);
uint32_t erase_sector_if_needed(uint32_t cur_sector, uint32_t write_addr, \
                               uint32_t page_size);
int32_t get_and_write_page(TCPSocket &socket, uint32_t fw_size,\
                           uint32_t page_addr, char * page_buf,\
                           uint32_t page_size);
bool apply_update(TCPSocket &socket, uint32_t address);

void enable_bootloader_rw_mpu(void);
void enable_bootloader_rx_mpu(void);

void benchmarkMain(void) {
  	EthernetInterface eth;
  	TCPSocket socket;
  	TCPServer tcpserver;
  	SocketAddress sockaddr;
    bool loaded_successfully = false;
  	int i;

    // initialize Ethernet/TCP
    iot2InitTCP(&eth, &socket, &tcpserver, &sockaddr);


    enable_bootloader_rw_mpu(); // enable R/W


  	loaded_successfully = apply_update(socket, POST_APPLICATION_ADDR);

    // enable R/X
    enable_bootloader_rx_mpu(); // enable R/W

  	i = socket.close();
  	if (i < 0){
  		pc.printf("[-] ERROR: close socket = %d\r\n", i);
  	}
  	pc.printf("[+] Closed the socket\r\n");
    // disconnect Ethernet
    eth.disconnect();
  	// May need to disable the ethernet here
    if (loaded_successfully){
  	   mbed_start_application(POST_APPLICATION_ADDR);
    }
    else{
      pc.printf("[+] Loading Failed\r\n");
      iot2Error();
    }

    pc.printf("Returned from Loaded Application \r\n");
}


bool apply_update(TCPSocket &socket,uint32_t address)
{

    ssize_t rx_size;
    uint32_t bytes_rx = 0;
    uint32_t page_addr;
    uint32_t fw_size = 0;
    uint32_t token_buf[1];
    int32_t bytes_written;
    uint32_t cur_sector;
    bool success = false;
    
    iot2CallSVC(IOT2_RESERVED_SVC,ENABLE_SECURE_DATA_SVC_NUM);// enable secure region
    // call an SVC to elavate privileges
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_ELAVATE_PRIV_SVC_NUM);

    flash.init();
    // Get Page Size
    uint32_t page_size = flash.get_page_size();
    if (page_size == 1){
      //(Some devices, e.g., STM32F4, don't use pages and can
      // be programmed on byte at a time.  Doing one byte at a time
      // would be very slow
      page_size = 1024;
    }
    char *page_buffer = new char[page_size];

    rx_size = iot2recv(&socket,(char *)token_buf, TOKEN_NUM_BYTES);
    if (rx_size == 4 && token_buf[0] == INIT_TOKEN){
      token_buf[0] = INIT_TOKEN;
      iot2send(&socket, (char *) token_buf, TOKEN_NUM_BYTES);
      rx_size = iot2recv(&socket, (char *)token_buf, TOKEN_NUM_BYTES);
      if (rx_size == 4){
        fw_size = token_buf[0];
        bytes_rx = 0;
        page_addr = address;
        cur_sector = address;
        while (bytes_rx < fw_size){

            cur_sector = erase_sector_if_needed(cur_sector, page_addr, page_size);
            bytes_written = get_and_write_page(socket, fw_size, page_addr,
                                           page_buffer, page_size);

            if (bytes_written <= 0){
              // If no bytes written or negative error occured
              success = false;
              break;
            }

            page_addr += bytes_written;
            bytes_rx += bytes_written;
            pc.printf("Bytes Written: %i\r\n", bytes_rx);

        }
      }

      if (bytes_rx == fw_size){
        success = true;
      }

    }

    token_buf[0] = success ? DONE_TOKEN : FAIL_TOKEN;
    iot2send(&socket, (char *)token_buf,TOKEN_NUM_BYTES);
    delete[] page_buffer;
    flash.deinit();
    iot2CallSVC(IOT2_RESERVED_SVC,DISABLE_SECURE_DATA_SVC_NUM);// disable secure region
    // call an svc to de-elavate privileges
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_DE_ELAVATE_PRIV_SVC_NUM);
    return success;

}

/**
  erase_sector_if_needed(uint32_t cur_sector, uint32_t write_addr)

  Erases the sector if has not already been erased, and advances
  the current sector if this is the last page in the current sector.

  uint32_t cur_sector  Address of current sector
  uint32_t write_addr  Address to be written
  uint32_t page_size       Size of page

  return value
    address of current sector,
*/
uint32_t erase_sector_if_needed(uint32_t cur_sector, uint32_t write_addr, \
                                uint32_t page_size){

    static bool sector_erased = false;
    uint32_t next_sector;

    next_sector = cur_sector + flash.get_sector_size(cur_sector);
    if (write_addr >= cur_sector && write_addr < next_sector){
          if (!sector_erased) {
            flash.erase(cur_sector, flash.get_sector_size(cur_sector));
            sector_erased = true;
          }
    }

    if (write_addr + page_size >= next_sector) {
        sector_erased = false;
        cur_sector = next_sector;
    }
    return cur_sector;
}

/**
    get_and_write_page

    Reads upto a page using socket, assumes page_buf is of
    atleast size page_size, and writes it to flash

    args:
      TCPSocket &socket  A connected TCP socket
      uint32_t fw_size   Size of fw to be received
      uint32_t page_addr   Address of flash page to be written to
      char * page_buf      Buffer of char of at least size page_size
      uint32_t pages_size  Size of a page of flash

    returns:
      int32_t  size of data written, negative value on error

*/
int32_t get_and_write_page(TCPSocket &socket, uint32_t fw_size,\
                            uint32_t page_addr, char * page_buf,\
                            uint32_t page_size ){

    static uint32_t bytes_rx = 0;
    ssize_t rx_size = 0;
    ssize_t rxed_bytes = 0;
    uint32_t fw_read_size;

    fw_read_size = fw_size - bytes_rx;
    if (fw_read_size >=page_size){
        fw_read_size = page_size;
    }else{
      // Fills any part of the page with 0's
      memset(page_buf, 0, page_size);
    }

    while (rxed_bytes < fw_read_size){
      rx_size = iot2recv(&socket, &(page_buf[rxed_bytes]),fw_read_size - rxed_bytes);
      if (rx_size < 0){
        while(1);
        return -1;
      }
      rxed_bytes += rx_size;
    }
    flash.program(page_buf, page_addr, page_size);
    pc.printf("Wrote Page =0x%08x : Size :%i \r\n", page_addr, page_size);
    bytes_rx += rx_size;

    return rxed_bytes;
}


void enable_bootloader_rw_mpu(void){

    // call an SVC to elavate privileges
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_ELAVATE_PRIV_SVC_NUM);

    MPU_Region_InitTypeDef MPU_InitStruct;
    // diable mpu
    __DMB();
    // disable faults (mem, usage, and bus)
    SCB->SHCSR &= ~(SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk); 

    // disable MPU control register
    MPU->CTRL = 0;

    // Secure data region
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = (POST_APPLICATION_ADDR);//CCRAM_ADDRESS_START;
    MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RW ;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER5;//RAM_REGION_NUMBER;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);


    // enable the MPU, will enable the background region to maintain
    // correct functionality for Mbed
    MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk;
    __DSB();
    __ISB();
    // check dep
    iot2CheckPrivCodeAndDEP();
    // call an svc to de-elavate privileges
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_DE_ELAVATE_PRIV_SVC_NUM);

}

void enable_bootloader_rx_mpu(void){
    // call an SVC to elavate privileges
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_ELAVATE_PRIV_SVC_NUM);

    MPU_Region_InitTypeDef MPU_InitStruct;
    // diable mpu
    __DMB();
    // disable faults (mem, usage, and bus)
    SCB->SHCSR &= ~(SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk); 

    // disable MPU control register
    MPU->CTRL = 0;

    // Secure data region
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = (POST_APPLICATION_ADDR);//CCRAM_ADDRESS_START;
    MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO_URO;//MPU_REGION_PRIV_RO ;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER5;//RAM_REGION_NUMBER;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);


    // enable the MPU, will enable the background region to maintain
    // correct functionality for Mbed
    MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk| 
                  SCB_SHCSR_USGFAULTENA_Msk| 
                  SCB_SHCSR_BUSFAULTENA_Msk;
    __DSB();
    __ISB();
    // check dep
    iot2CheckPrivCodeAndDEP();
    // call an svc to de-elavate privileges
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_DE_ELAVATE_PRIV_SVC_NUM);

}