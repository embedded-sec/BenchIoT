import socket
import sys
import time
import random
import string
import os
from argparse import ArgumentParser

#######################################################################
#                              GLOBALS                                #
#######################################################################


# iot2 special messages
IOT2_START_COLLECTING_RESULTS_MSG = "[IoT2] collect_results: begin"
IOT2_END_TCP_DRIVER_MSG = "[IoT2] END_NETWORK_DRIVER"

PACKET_SIZE = 256

# number of images to send, and then receive (send NUM_IMGS and recv NUM_IMGS)
NUM_IMGS = 20

# cmds
PUT_CMD = "put"
GET_CMD = "get"
LOGIN_CMD = "login"
PASS_CMD = "pass"

# image names to simplify verification and send/recv
IMAGE_NAME = "IMAGE"
RESULT_IMAGE_NAME = "COMPR"
JPG_EXT = '.JPG'

# connectd display directory name, to be used for benchmarks and
# verification directory
CONN_DISP_DIR = "ConnDisp/"
CONN_DISP_RESULTS_DIR = "ConnDisp_results/"

#######################################################################
#                              FUNCTIONS                              #
#######################################################################

def gen_filelist(file_list_path, file_list, file_ext):
    """
    Returns a list of files with the given extension for the given directory
    :param file_list_path: Path to the files
    :param file_list: Variable to hold the list of file names (i.e. the whole path)
    :param file_ext: The extension of the files
    :return: List of files with the given extension
    """
    # check if path exists
    if not os.path.exists(file_list_path):
        # path does not exist, print a note and return the same file list
        print("[-] Directory  <%s> does not exist, returning empty file list" % file_list_path)
        return file_list

    for filename in os.listdir(file_list_path):
        if filename.endswith(file_ext):
            file_list.append(os.path.join(file_list_path, filename))
    # sort the file list before returning
    file_list = sorted(file_list, key=str.lower)
    return file_list

def bench_recv(clnt_socket, packet_size, fd):
    """
    This function is used to receive the benchmark results and log them to a file
    to verify the correctness of the output
    :param clnt_socket: socket object
    :param packet_size: unify packet size for all packets in benchmark
    :param fd: file descriptor
    :return: None
    """
    msg = clnt_socket.recv(packet_size)
    msg = msg.replace('\0', '')
    if msg == IOT2_END_TCP_DRIVER_MSG:
        line_block = "*"*80 + "\n"
        fd.write(line_block)
    else:
        fd.write(msg + "\n")
    msg_size = len(msg)
    return msg, msg_size


def bench_send(clnt_socket, server_msg):
    # append server msg to maintain PACKET_SIZE
    append_size = PACKET_SIZE - len(server_msg)
    append_msg = "\0" * append_size
    server_msg = server_msg + append_msg
    # send msg
    bytes_sent = clnt_socket.send(server_msg)
    # check that all msg has been sent
    if bytes_sent != len(server_msg):
        print("[-] tcp_driver ERROR: bytes_sent != server_msg")
    return


def send_img_file(client, filepath, file_size):
    with open(filepath, 'r') as fd:
        curr_size = 0
        while curr_size < file_size:
            if (file_size - curr_size) > PACKET_SIZE:
                file_block = fd.read(PACKET_SIZE)
            else:
                file_block = fd.read(file_size - curr_size)
            #bench_send(client, file_block)
            bytes = client.send(file_block)
            if bytes != len(file_block):
                print("[-] tcp_driver ERROR: bytes_sent != file_block")
            curr_size += len(file_block)
            print("curr_size = %d" % curr_size)
            ack = client.recv(4)
            #print(ack)
    return


# send all images, this will call send_img_file NUM_IMGS
def send_all_imgs(client):
    curr_path = os.path.dirname(os.path.realpath(__file__))
    full_path = curr_path + '/' + CONN_DISP_DIR
    jpg_list = []
    jpg_list = gen_filelist(full_path, jpg_list, '.jpg')
    # loop though images, send put cmd then send image
    cntr = 0
    for i in range(NUM_IMGS):
        jpg_image = jpg_list[i]
        print("-" * 80)
        print("Sending: %s" % jpg_image)
        print("-" * 80)
        # get file size
        file_size = os.path.getsize(jpg_image)
        # send cmd
        msg = PUT_CMD + " " + IMAGE_NAME + "%03d%s %d" % (cntr, JPG_EXT, file_size)
        bench_send(client, msg)
        send_img_file(client, jpg_image, file_size)
        cntr += 1
        print("[Finished sending image #%d]" % cntr)
    return


def get_img_file(filepath, client):
    # get reply with size info
    msg = client.recv(PACKET_SIZE)
    msg = msg.replace('\0', '')
    file_size = int(msg.split(' ')[1].split(':')[1])
    with open(filepath, 'w') as fd:
        curr_size = 0
        while curr_size < file_size:
            if (file_size - curr_size) > PACKET_SIZE:
                file_block = client.recv(PACKET_SIZE)
                fd.write(file_block)
            else:
                file_block = client.recv(file_size - curr_size)
                fd.write(file_block)
            #print("curr_size = %d" % curr_size)
            curr_size += len(file_block)
            bench_send(client, "ack")
            #print(ack)
    print("END...........")
    return


def get_all_imgs(client):
    curr_path = os.path.dirname(os.path.realpath(__file__))
    full_path = curr_path + '/' + CONN_DISP_RESULTS_DIR
    # loop through the number of images
    for i in range(NUM_IMGS):
        # send get request
        req_msg = GET_CMD + " " + RESULT_IMAGE_NAME + "%03d%s" % (i, JPG_EXT)
        bench_send(client, req_msg)
        result_img = full_path + RESULT_IMAGE_NAME + "%03d%s" % (i, JPG_EXT)
        print("-" * 80)
        print("Receiving: %s" % result_img)
        print("-" * 80)
        get_img_file(result_img, client)
        print("[r%d]")


def bench_tcp_driver(host_ip, port, verification_file, benchmark_type):
    print("host_ip = %s, port= %s" %(host_ip, port))
    clnt_flag = True
    attempts = 0
    # for connected display, we need a directory for the results
    # thus the verification file is removed from here
    while clnt_flag:
        try:
            attempts += 1
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client.settimeout(10)
            client.connect((host_ip, port))
            client.settimeout(None)
            if benchmark_type == 0:
                client.send("ack\0")
            # send images
            send_all_imgs(client)
            # recv signal msg to start receiving results
            #ack = client.recv(4)
            # recv result images
            #get_all_imgs(client)

            client.close()
            clnt_flag = False

        except Exception as msg:
            print(" [%i] ERROR: %s, reconnecting" % (attempts, msg))
            pass
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
    net_driver("192.168.0.10", 1337, 1, "nothing.txt", "nothing", "nothing", benchmark_type)
