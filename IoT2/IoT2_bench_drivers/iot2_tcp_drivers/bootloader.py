from struct import pack, unpack
import binascii
import time
import socket


HOST = '192.168.0.10'
PORT = 1337
BUFF_SIZE = 1024

START_TOKEN = "init"
DONE_TOKEN = "done"
FAIL_TOKEN = "fail"
ACK_TOKEN = "ack!"

# bootloader configuration [iot2-debug]: add this to config file
START_ADDR = 0x00040000#0x08040000
SIZE = 16*1024


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


def tx_data(fw_data, client, benchmark_type):

    print "Sending Start Token:", START_TOKEN
    client.send(START_TOKEN)
    data = client.recv(len(START_TOKEN))

    if data and data == START_TOKEN:
        print "Got Start Token:", data
        client.send(pack("<I", len(fw_data)))
        print "Sent Length: ", len(fw_data)
        print "Sending FW: ", len(fw_data)
        if benchmark_type == 1:
            for i in xrange(0,len(fw_data), 128):
                #print("@ send loop, i = %d" % i)
                client.send(fw_data[i:i+128])
        else:
            print "Baremetal"
            for i in xrange(0,len(fw_data), 1024):
                #print("@ send loop, i = %d" % i)
                client.send(fw_data[i:i+1024])
                time.sleep(0.5)
        print "Firmware Sent"
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


def bench_tcp_driver(host_ip, port, verification_file, benchmark_type):
    print("host_ip = %s, port= %s" %(host_ip, port))
    clnt_flag = True
    attempts = 0
    # open results file
    fd = open(verification_file, 'w')
    while clnt_flag:
        try:
            # the same setup used for all benchmarks
            attempts += 1
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client.settimeout(10)
            client.connect((host_ip, port))
            client.settimeout(None)
            if benchmark_type == 0:
                client.send("ack\0")
            # bootloader driver main function
            fw_data = create_test_application(START_ADDR, SIZE)
            with open('gen_fw.bin', 'wb') as outfile:
                outfile.write(fw_data)
            tx_data(fw_data, client, benchmark_type)

            client.close()
            clnt_flag = False

        except Exception as msg:
            print(" [%i] ERROR: %s, reconnecting" % (attempts, msg))
            pass
    # close verification file
    fd.close()
    return



def net_driver(host_ip, port, iterations, verification_file_path, bootloader_file,
               board_name, benchmark_type, iot2_synch_obj=None):
    """
    This is the main function for the tcp_driver. It is generic so most likely no change is needed here. The main
    function to change for each benchmark is the internal bench_tcp_driver within each file.
    :param host_ip: IP of the PC
    :param port: port number to connect to.
    :param iterations: Number of times to run the benchmark
    :param verification_path: Where to write the results (if there any) to verify them with the ground truth.
    :param bootloader_file: For the bootloader application, this should be the binary file to load.
    :param board_name: Name of the board used. Note that there is "/" in the end.
    :param benchmark_type: 1: OS benchmarks, 0: bare-metal
    :return: None
    """
    iteration_cntr = 0
    while iteration_cntr < iterations:
        if iot2_synch_obj:
            while iot2_synch_obj.get_tcp_proc_flag() == 0:
                if iot2_synch_obj.get_benchmark_status() == 1:
                    iot2_synch_obj.set_tcp_proc_flag()
                    print("TCP FLAG IS SET!!")
        print("*"*80)
        print("TCP driver started, iter_cntr = %d, ost_ip = %s, port= %s" % (iteration_cntr, host_ip, port))
        print("*"*80)
        bench_tcp_driver(host_ip, port, verification_file_path, benchmark_type)
        iteration_cntr += 1
        # reset tcp_proc_flag for the next iteration
        if iot2_synch_obj:
            iot2_synch_obj.reset_tcp_proc_flag()

    return


#net_driver("192.168.0.10", 1337, 1, "nothing.txt", "nothing", "nothing")


if __name__ == "__main__":
    from argparse import ArgumentParser
    arg_parser = ArgumentParser()
    arg_parser.add_argument('-f','--filename',metavar="FILE",
                        help='Firmware file to transmit (use ' + \
                        'arm-none-eabi-objcopy -O binary <file.elf> <outfile>)', required=False)
    arg_parser.add_argument('--start_addr', default=0x08020000, type=int,
                        help='Start Address for generated test firmware', required=False)
    arg_parser.add_argument('--size', default=16*1024, type=int,
                        help='Size of generated firmware to transmit', required=False)
    arg_parser.add_argument('--test', dest='run_quick_test' , default=False,
                            action='store_true',
                            help='Flag to run a quick manual test using the hardcoded values')
    arg_parser.add_argument('--bm', dest='run_baremetal' , default=False,
                            action='store_true',
                            help='Flag to choose type of benchmark, either with OS or bare-metal')

    args = arg_parser.parse_args()
    if args.filename:
        tx(args.filename)
    elif args.run_quick_test:
        benchmark_type = 1 # OS benchmark
        if args.run_baremetal:
            benchmark_type = 0
        net_driver("192.168.0.10", 1337, 1, "nothing.txt", "nothing", "nothing", benchmark_type)
    else:
        fw_data = create_test_application(args.start_addr, args.size)
        with open('gen_fw.bin', 'wb') as outfile:
            outfile.write(fw_data)
        tx_data(fw_data)
