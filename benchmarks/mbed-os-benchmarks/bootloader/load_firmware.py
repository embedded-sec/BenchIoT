
from struct import pack, unpack
import binascii

import socket


HOST = '192.168.0.10'
PORT = 1337
BUFF_SIZE = 1024

START_TOKEN = "init"
DONE_TOKEN = "done"
FAIL_TOKEN = "fail"



def create_test_application(load_addr=0x08002000, size=64*1024):
    '''
        Creates a test application that simply returns to the bootloader.
        Creates and ISR Table that point to a infinte loop, except reset
        vector that points to two instructions
        ' mov sp, r3'
        ' bx lr'
    '''
    SP_ADDR = 0x20050000  #  Address of stack for loaded application
    fw_list = []
    fw_list.append(pack("<I",SP_ADDR))
    fw_list.append(pack("<I",load_addr+1025))

    # build rest of ISR
    for isr in xrange(2,256):
        fw_list.append(pack("<I",load_addr+1029)) # 4 bytes after end of ISR

    # Add Code
    fw_list.append('\x9d\x46\x70\x47')  # mov sp,r3; bx lr
    fw_list.append('\xfe\xbf\xff\xf7')  # b.w

    # Fill rest with garbage
    i = 0
    #TODO when bootloader does check sum update to be random data
    while (len(fw_list)< size / 4):
        fw_list.append(pack("<I",i))
        i += 1

    return ''.join(fw_list)


def tx(filename):
    with open(filename,'rb') as fw_file:
        fw_data = fw_file.read()
    tx_data(fw_data)


def tx_data(fw_data):
    client = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
    client.settimeout(50)
    client.connect(( HOST, PORT ))
    client.settimeout(None)

    print "Sending Start Token:", START_TOKEN
    client.send(START_TOKEN)
    data = client.recv(len(START_TOKEN))

    if data and data == START_TOKEN:
        print "Got Start Token:", data
        client.send(pack("<I", len(fw_data)))
        print "Sent Length: ", len(fw_data)
        print "Sending FW: ", len(fw_data)
        for i in xrange(0,len(fw_data), 128):
            client.send(fw_data[i:i+128])
        data = client.recv(len(DONE_TOKEN))
        if data and data == DONE_TOKEN:
            print "Sent Successfully, Token: ", data
        else:
            print "Transmission Failed, Token: ", data


def rx():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('', PORT))
    s.listen(1)

    conn, addr = s.accept()
    print 'Connection address:', addr

    data = conn.recv(5)
    print data
    if (data and data == START_TOKEN):
        conn.send(START_TOKEN)
        data = conn.recv(4)
        size = unpack('<I', data)[0]
        print "Size: ",size

        received_count = 0
        with open("outfile.bin",'wb') as outfile:
            while (received_count < size):
                request = size - received_count
                if request > BUFF_SIZE:
                    request = BUFF_SIZE
                data = conn.recv(request)
                if (data):
                    received_count += len(data)
                    print "Received %i: %s..."% (len(data),
                           binascii.hexlify(data[0:10]))
                    outfile.write(data)
                else:
                    print ("Failed")
                    conn.send(FAIL_TOKEN)
                    conn.close()
                    return
        conn.send(DONE_TOKEN)  # echo
        print "Done"
    else:
        conn.send(FAIL_TOKEN)
    conn.close()


if __name__ == "__main__":
    from argparse import ArgumentParser
    arg_parser = ArgumentParser()
    arg_parser.add_argument('-f','--filename',metavar="FILE",
                        help='Firmware file to transmit (use ' + \
                        'arm-none-eabi-objcopy -O binary <file.elf> <outfile>)')
    arg_parser.add_argument('--start_addr', default=0x08020000, type=int,
                        help='Start Address for generated test firmware')
    arg_parser.add_argument('--size', default=16*1024, type=int,
                        help='Size of generated firmware to transmit')


    args = arg_parser.parse_args()
#
    if args.filename:
        tx(args.filename)
    else:
        fw_data = create_test_application(args.start_addr, args.size)
        with open('gen_fw.bin', 'wb') as outfile:
            outfile.write(fw_data)
        tx_data(fw_data)
