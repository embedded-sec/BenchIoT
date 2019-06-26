import socket
import sys
import time
import random
import string
from argparse import ArgumentParser

#######################################################################
#                              GLOBALS                                #
#######################################################################


# iot2 special messages
IOT2_START_COLLECTING_RESULTS_MSG = "[IoT2] collect_results: begin"
IOT2_END_TCP_DRIVER_MSG = "[IoT2] END_NETWORK_DRIVER"

PACKET_SIZE = 64

# Number of packets to receive before sending inquiries
# 100 - 1 for 0 indexing
NUM_PACKAGES = 99

# used to setup the number of inquiries to send to the smart locker
# 1/3 correct inquiries, where the package for the person exists
# 1/3 correct inquiries, but NO package for the person exists
# 1/3 wrong inquiries (miss-formatted)
NUM_INQUIRIES = 100

# smart light cmds
SET_LIGHT_ON_PERIOD_CMD = "LIGHT_ON_PERIOD "
SET_LIGHT_OFF_PERIOD_CMD = "LIGHT_OFF_PERIOD "
TURN_LIGHT_ON_CMD = "TURN_LIGHT_ON"
TURN_LIGHT_OFF_CMD = "TURN_LIGHT_OFF"
KEEP_LIGHT_ON_CMD = "KEEP_LIGHT_ON"
INVLAID_RANDOM_CMD = "knock knock how is there? a bad cmd"

INVALID_CMD_LIST = [
                    "qIjHiRBabTFAxdIcTS9:lUtvD4LhQlmVksnMx 30 XgVp5",
                    "Yq9W70sSZ5JvNzrLDh",
                    "azNjJ1df5GK1x17CNKWt0znWxr 3",
                    "8YQgvJGVz",
                    "TyL5Y1NDr:YfmRxld5JY1yfHfjCwKh1iN4GUD0F44",
                    "fbRt4Nk5ISmX:AqGsf6KOBOe6B1X2aPZT5o9gunzPmvkn8dgqJ3",
                    "FKm iuhx33Y",
                    "WgvZXY9UXbGYOzh7N7FaGKNE9joVtjnIRhEmg",
                    "57IqBZ :k7aU:lrxNeWNwoawJLQQzlbam 1HM KUwQx8wYT06Nr76ZtHcg",
                    "AiknJCL:YVfiO1il35koCq1M0B2gFAkwHlVRvw4e2K7",
                    "zpvurjzV MgxDPBct",
                    "d5S9gXRSKbjiJ 2wmGYFLxRIjT",
                    "SUrJaC: gAqcvZ",
                    "N:toscTE41yUEKr87rKIyL2M1w78CrlUH14vv40LvQxIdiGgzuQOJF4wwP5o5lN",
                    "TfVT8pHa2ruQ5Yei qmSjwMeZ",
                    "G9ZyTPaM 8REKrAFFzXj BOAVb:h",
                    "CjH2rleJygS7VjlwbLVTQ4WT8jZzLt0Dp3hmGAKvG8hqX4 uleI5nqtw",
                    "VXN",
                    "YoiGqd",
                    "cSpodFIqjPLX",
                    "CmXgsUmhHQ3z:UEXGdxut8bVfVZ3 bRU2b:4qaG95RsrVA8KQy1vdpWFGyDJg",
                    "7uAcJ",
                    "b0DXG00zcpypg5sGGJU:Nua3r xrdXpZLPOqvlgNq1P HhjMFwgwTxNZYgit",
                    "c69NS4yLbpymH5kumGxt0Jx8I86GAr9vDQ4NemhBUxUOI",
                    "7gXBpLlz1MYtUdz4INjKAP6HO3tLPtL2do",
                    "W4hbQ",
                    "QCuHYAcCpzB:T:PRLYZnu1tNYGKYBkgc4i22i0xV:V ",
                    "HPtfeLG:0MrwMh6oBjNzRY6oLa9my9m2a9UUg",
                    "dAIoY7pFHbbLXDt5eJOTB I3m SxuFeY3aKQxmWH:vmxpUO",
                    "YnCfVTmELmBd5lg",
                    "lZpccS60G7OF2T :W8Npm:wHn3NmbgOT0UGo",
                    "6q7mK1kiCw",
                    "CNip quyjPB4i9JNDjvxHkIy0nJOMC6MowTMDgOgi2yEp7HN2k",
                    ]


LIGHT_ON_PERIOD = "02:27:13,02:27:17"
LIGHT_OFF_PERIOD = "02:27:18,02:27:20"

SERVER_SLEEP = 0.67
#######################################################################
#                              FUNCTIONS                              #
#######################################################################


def send_smartlight_inquiries(clnt_socket, fd):
    cmds = [KEEP_LIGHT_ON_CMD,  TURN_LIGHT_OFF_CMD, TURN_LIGHT_ON_CMD]
    invalid_cntr = 0
    for i in range(NUM_INQUIRIES):
        if (i % (len(cmds)+1)) == (len(cmds)):
            server_msg = INVALID_CMD_LIST[invalid_cntr]
            invalid_cntr += 1
        else:
            server_msg = cmds[i % (len(cmds) + 1)]
        print("-"*80)
        print("[%d] %s" %(i, server_msg))
        print("-"*80)
        print("[response:%d]" %i)
        time.sleep(SERVER_SLEEP)
        bench_send(clnt_socket, server_msg)
        response, response_size = bench_recv(clnt_socket, PACKET_SIZE, fd)
    return response, response_size


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
    print("-"*80)
    print("recv: %s" % msg)
    print("-"*80)
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


def send_smartlight_configs(clnt_socket, fd):
    # will send 3 cmds, one of them is an invalid cmd
    # cmds to send
    light_on_config_cmd = SET_LIGHT_ON_PERIOD_CMD + LIGHT_ON_PERIOD
    light_off_config_cmd = SET_LIGHT_OFF_PERIOD_CMD + LIGHT_OFF_PERIOD
    invalid_cmd = INVLAID_RANDOM_CMD
    cmds = [invalid_cmd, light_on_config_cmd, light_off_config_cmd]
    # send cmds
    for server_msg in cmds:
        # send cmd
        bench_send(clnt_socket, server_msg)
        # recv cmd
        msg, msg_size = bench_recv(clnt_socket, PACKET_SIZE, fd)
        #print("response = %s" % msg)
    return msg, msg_size


def bench_tcp_driver(host_ip, port, verification_file, benchmark_type):
    print("host_ip = %s, port= %s" %(host_ip, port))
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
            packet_cntr = 0
            cmd_period = 0
            if benchmark_type == 0:
                client.send("ack\0")
            response, response_size = send_smartlight_configs(client, fd)
            # counts packets to track whether to send/recv for the benchmark
            packet_cntr = 3
            response, response_size = send_smartlight_inquiries(client, fd)
            while response != IOT2_END_TCP_DRIVER_MSG:
                response, response_size = bench_recv(client, PACKET_SIZE, fd)
            # send the following only for bare-metal benchmark
            if benchmark_type == 0:
                client.send("ack\0")
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
    net_driver("192.168.0.10", 1337, args.iterations, "nothing.txt", "nothing", "nothing", benchmark_type)

