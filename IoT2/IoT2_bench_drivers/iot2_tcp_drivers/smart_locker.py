import socket
import sys
import time
import random
from argparse import ArgumentParser

# IoT2 imports
#from .. import iot2_settings
#import setup_result_collection
#import iot2_measure_static_flash_and_ram as iot2_helper
#import iot2_result_collector

number_of_packets = 1


#######################################################################
#                              GLOBALS                                #
#######################################################################


# iot2 special messages
IOT2_START_COLLECTING_RESULTS_MSG = "[IoT2] collect_results: begin"
IOT2_END_TCP_DRIVER_MSG = "[IoT2] END_NETWORK_DRIVER"

PACKET_SIZE = 128

# Number of packets to receive before sending inquiries
# 100 - 1 for 0 indexing
NUM_PACKAGES = 100

# used to setup the number of inquiries to send to the smart locker
# 1/3 correct inquiries, where the package for the person exists
# 1/3 correct inquiries, but NO package for the person exists
# 1/3 wrong inquiries (mis-formatted)
NUM_INQUIRIES = 30

#######################################################################
#                              FUNCTIONS                              #
#######################################################################


def send_smart_locker_inquiries(clnt_socket, fd):
    correct_name_prefix = "Person"
    wrong_name_prefix = "Nobody"
    correct_req_prefix = "CONTAIN_PACKAGE_FOR? "
    wrong_req_prefix = "NONE_REQ"
    for i in range(NUM_INQUIRIES):
        server_msg = ""
        suffix_num = "".join("_%03d" %i)
        inquiry_type = i % 3    # since we have 3 types of inquiries
        '''
        Wrong inquiries -> inquiry_type = 0
        Correct inquiries, correct name -> inquiry_type = 1
        Correct inquiries, wrong name -> inquiry_type = 2
        '''
        if inquiry_type == 0:
            server_msg = wrong_req_prefix
        elif inquiry_type == 1:
            server_msg = correct_req_prefix + correct_name_prefix + suffix_num
        else:
            server_msg = correct_req_prefix + wrong_name_prefix + suffix_num

        # append server msg to maintain PACKET_SIZE
        append_size = PACKET_SIZE - len(server_msg)
        append_msg = "\0"*append_size
        server_msg = server_msg + append_msg
        # send msg
        bytes_sent = clnt_socket.send(server_msg)
        # check that all msg has been sent
        if bytes_sent != len(server_msg):
            print("[-] SMART-LOCKER ERROR: bytes_sent != server_msg")
        #print("*"*80)
        #print("sent: %s" % server_msg)
        # recv response and log it to result file
        inq_response = clnt_socket.recv(PACKET_SIZE)
        #print("recv %s" %inq_response)
        #print("*" * 80)
        inq_response = inq_response.replace('\0', '')
        fd.write(inq_response)
    return


def bench_recv(clnt_socket, packet_size, fd):
    msg_size = 0
    #while msg_size != packet_size:
    msg = clnt_socket.recv(packet_size)
    #print("."*20)
    #print(len(msg))#msg.split())
    #print("."*20)
    msg = msg.replace('\0', '')
    if msg == IOT2_END_TCP_DRIVER_MSG:
        line_block = "*"*80 + "\n" + "*"*80 + "\n" + "*"*80 + "\n"
        fd.write(line_block)
    else:
        fd.write(msg)
    msg_size = len(msg)
    return msg, msg_size


def bench_tcp_driver(host_ip, port, verification_file, benchmark_type):
    #print("host_ip = %s, port= %s" %(host_ip, port))
    clnt_flag = True
    attempts = 0
    # open results file
    fd = open(verification_file, 'w')
    while clnt_flag:
        try:
            attempts += 1
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client.settimeout(10)
            client.connect((host_ip, port))
            client.settimeout(None)
            print("[+] TCP driver connected!")
            if benchmark_type == 0:
                client.send("ack\0")
                client.send("ack\0")
            packet_cntr = 0
            cmd_period = 0
            response = ""
            response_size = 0
            # counts packets to track whether to send/recv for the benchmark
            packet_cntr = 0
            while response != IOT2_END_TCP_DRIVER_MSG:
                # client.send(msg + str(i + 1))
                response, response_size = bench_recv(client, PACKET_SIZE, fd)
                if benchmark_type == 0:
                    client.send("ack\0")
                #print("cntr[%d]" % packet_cntr)
                packet_cntr += 1
                # fot the smart locker, the benchmark expects inquiries after 100
                # packages have been dropped
                #print("_" * 80)
                #print("CLIENT(%d): %s" % (response_size, response))
                #print("_" * 80)
                if packet_cntr == NUM_PACKAGES:
                    #print("@send inq [%d]" % number_of_packets)
                    send_smart_locker_inquiries(client, fd)
                # check if it is time to send request full file log
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
        print("TCP driver started, iter_cntr = %d, host_ip = %s, port= %s" % (iteration_cntr, host_ip, port))
        print("*"*80)
        bench_tcp_driver(host_ip, port, verification_file_path, benchmark_type)
        iteration_cntr += 1
        # reset tcp_proc_flag for the next iteration
        if iot2_synch_obj:
            iot2_synch_obj.reset_tcp_proc_flag()
        #time.sleep(3)

    return


if __name__ == "__main__":
    arg_parser = ArgumentParser()
    arg_parser.add_argument('--bm', dest='run_baremetal' , default=False,
                            action='store_true',
                            help='Flag to choose type of benchmark, either with OS or bare-metal')
    arg_parser.add_argument('--n', dest='iterations', nargs='?', const=1, type=int, default=1,
                            help='Flag to choose the number of iterations')
    args = arg_parser.parse_args()
    benchmark_type = 1  # OS benchmark
    if args.run_baremetal:
        benchmark_type = 0
    print("-------------")
    print(args.iterations)
    print("-------------")
    net_driver("192.168.0.10", 1337, args.iterations, "nothing.txt", "nothing", "nothing", benchmark_type)
