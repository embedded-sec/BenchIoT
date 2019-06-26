## Bootloader

The bootloader is an application that loads another application of using TCP.
It copies it to flash and then executes it.


On power up the bootloader, starts a TCP server.  The client (firmware_tcp_tx.py)
performs a handshake, and sends the firmware size, then the firmware.
The server will then send either a done or a fail response.  Done if loaded
correctly and fail if not.  The bootloader will then execute the firmware if loaded correctly.


When loaded the flash layout looks something like the following

```
+--------------+
|              |
| Application  |
|              |
+--------------+ FLASH_BASE_ADDR + Size of Bootloader rounded up to nearest page size (0x20000)
| Bootloader   |
+--------------+ FLASH_BASE_ADDR
```

### Prepare the application to be loaded
The application must be linked to start from an address above the Bootloader
as shown above. This address is POST_APPLICATION_ADDR in main.cpp. The details
are board specific, but generally just making the Flash in the linker script
smaller and start from an offset from Flash base address so Application and
Bootloader do not share a page in Flash and start address of flash is aligned on
1KB boundary so as to satisfy the ARMv7-M alignement requirements for the vector
table.

## Testing Bootloader

Connect board to computer using Ethernet cable and set your interface ip address
to 192.168.0.11, gateway to 255.255.255.0.  Target board will be set to
192.168.0.10

In all instructions below replace YOUR_BOARD with your target board (e.g. K64F)

Build Test application
```
cd ../blinky
MAKE all BOARD={YOUR_BOARD}  BOOTLOADER=1
```
This will create binary in blinky/build/blink.bin

```
cd ../bootloader
MAKE all BOARD={YOUR_BOARD}
```

Load bootloader onto board using GDB and reset board

Load test applications
```
python firmware_tcp_tx.py -f ../blinky/build/blinky.bin
```
Should exit indicating success, if not reset board and try again.

On successful loading of the application, the board should blink an LED.
