import socket
import sys
import time
import random
import string
from argparse import ArgumentParser

import string
from random import *
import random
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
NUM_INQUIRIES = 120

# smart light cmds
SET_TEMP_CMD = "SET_TEMPERATURE "
GET_TEMP_CMD = "GET_TEMPERATURE"
INVLAID_RANDOM_CMD = "knock knock how is there? a bad cmd"

INVALID_CMD_LIST = [
    'kwzzqkfyisk',
     'avfzvnvviqhywnrwswdmjucjhrdnqrueludmeh',
     'akvpbewscsluzpfafaxmhjcfotyxfmgyugt',
     'irhtrfbtlrqyokszfkdouooulgacaqlkfepyzadblkvaxozgvlsxta',
     'vlcvvikbrsgtticzsnvpubzryzdegsrkmmvovqtfbvqvmdgvruzrjrmnynd',
     'xjlhonfnxoxcrdtfbpxjqr',
     'vkietneetyyae',
     'dnuhuocykxflpnbbcbhxvbtlnhpfkkpezzgg',
     'khsxfrbcqxgurswdczybcosstjftueuxcviupcwebcrhhgzzzwfgcozlwclt',
     'kna',
     'ediziwmjvir',
     'xcrvjebvuuluqlcune',
     'jujnsuuwqhywnsfestzwpsgc',
     'fdkrkjyaly',
     'm',
     'agewqdlkpmcxgvedwakurxhedsoaynfcyqlvdxluwvlsbjsbgmdzfdkcvu',
     'wnvztytrkarwpwmuvnovcnsaasepvwfjklyygtrcqdtxsnhjzioom',
     'oyyqffcwygeulibhapoeafpqtviuryvixqqdwyhdulvuzobm',
     'ttpqthgxbsdvhwqiiwhgimoqwnuywllfqbk',
     'wxkufggjlrkwdadpjfpvqglbwfltlufwsarchhknxvgtkzpktmaguqmaqrwim',
     'vlezm',
     'nufjyphofwoage',
     'xruldcqqqgwbmarqnqsyggstrbtjkjjhaksezzwcoymbmvmxmlmgvxsztfzwzgw',
     'pbydwzxrtjzctkoiehyzekewrjtamegqhwyg',
     'jcbihtmykhifrubzqrurdvcwc',
     'fob',
     'irtzmabwkzvrayjcomhzvxtxhbplpgksvgtfyx',
     'jhrbzjavfkdptvtwdeowvxyxziwkkysotrklewlvvvqsyngbfl',
     'xophwtrzrvownuqgbvrgctnfwz',
     'irmzcshtfgmknwbwblbxvgifqrimxaafhnxofhsfgzeqq',
    ']Qr[Iumc96;C_3d}q6\'xOBxvl*5DIaB{3Of6JZgS9h|Z&Bk&Id#BxumK(.(ck',
    'i]Q7#s=Uwiv3?NKCD(O\';D@gKxPs|8-[*!r5',
    'xbBco5ZV<O[X4FG@V,n)ho6,&sc4f]+hYH<NF.L+R[b"hwK,42*C1',
    'v2AVds5p2%mKn8LEwIY|',
    'H?scVmJf28I78WNO#o4&f1n:.%fy,>@',
    'h}3@/O/L+7Ueyj_ONa!$v',
    'q5JN<+~+\'P^q*kqUM,$076[{F9l^(4vm*u,(O6zp^?FUpoU',
    '%}$6Bt6x6;js(i/>`0S~ifnQ7}#TYeXF^ti?\WvR7hG*y^1I#q&',
    '_PWZ<fBSK)6:Q"g_]Wer/-yOUkDg\jU:/G#!w`K_+',
    'fq`?-(>"kquwC"Kk@`@9{',
    'tP?r&kLFpIKQEjaSBzH}q@-eH3=DG&Z`fQ',
    'G3;4{<+Mum8s5OTH&04&Dq8h7ImF+F[A(~DXU]y$t_Q~WT\.q8e',
    'm#aaiivDsCnxP,|V$0/CuEifJ<,h(w3uaq%.b.bJ$@&U(Ri*Dc,yVkSC1iGx:',
    '"5/o,n]COp)T>r7}@7Qd8|Xab*"w:',
    'l6O-Q|2-6KuwQ0JIvN}v^gJ*!',
    'YUGZ=!$"_2lAPs^;wzT\WvN0^cvea".QSi:H4hxl+fC@,tU)\]@',
    'B/t(lsSutx<_&oo6mPq',
    'a4[t<2~^cteQYs?21u.W1(nKwm(!j]S?X',
    'aOh_<\'o)b;F}qATPNv*AVIG',
    '6?@e_INTy@|&L88cvqro;E:HiUCLT7.6xm*?kwv%__\p]kcv!CiaL!,a9Q',
    'AagfsWr(44jtaUT_<Yp2jNkFu"a$;n"^PRK9o9A<',
    '6&Xd=+OGbD|z1DT9s9{Hi0kxDx2mBs(M$kE_:A-dL5^vLb_S',
    'On.gs1DDt!ZFTvRecetaC;T%u*L*N"3F/\'{z\'U/:e+Rz3+<AVW:+d-~x8aL%=',
    '{P[D``Un0z.JGrS\R<R~gJ%Y;JY{',
    'J,3$bRFaE+?U%_Ewx#,AHQ%QFdRo1t4<vT$i$)"S:L3^',
    '%3Bv`U-6R08tBO}8<$Q9?*%9==B-k.*8(Zfx*OF#Dz*9:G{Zv>>V@tnf~+#Hp^n',
    '1XNA{t@j7m.5$oYvT[PEE\J\'/VqCqhd9$%&.rA?R/[U0M4VP1&:iB4q@IYcx8',
    '4^@|Gr8*+J0:&F1flq:i{&7wVPwv`lRtB~]#^5PM%t?#h-.',
    'BtZ^TL3U?RysxQt(4S',
    ':{CJr&)Ah:X}+`NWL+{vwZil7Pp+N%e7+wt./#k5D;(Pp\s_:',
                    ]


INPUT_TEMP_LIST = [38, 18, 18, 29, 28, 18, 27, 24, 33, 16, 34, 14, 30, 33, 47, 28, 23, 40, 41, 12, 44, 41, 34, 28, 31,
                   30, 45, 38, 25, 40, 22, 10, 32, 45, 11, 15, 13, 13, 29, 49, 18, 24, 19, 36, 28, 28, 32, 35, 15, 46,
                   23, 50, 10, 43, 34, 44, 10, 45, 13, 34]

#######################################################################
#                              FUNCTIONS                              #
#######################################################################


def send_smartthermostat_cmds(clnt_socket, fd):
    cmds = [SET_TEMP_CMD,  GET_TEMP_CMD]
    invalid_cntr = 0
    set_temp_input_cntr = 0
    for i in range(NUM_INQUIRIES):
        cmd_flag = i % (len(cmds)+1)
        if cmd_flag == (len(cmds)):
            server_msg = INVALID_CMD_LIST[invalid_cntr]
            invalid_cntr += 1
        elif i % len(cmds) == 0:
            server_msg = cmds[0] + str(INPUT_TEMP_LIST[set_temp_input_cntr])
            set_temp_input_cntr += 1
        else:
            # get temperature cmd
            server_msg = cmds[1]

        print("-"*80)
        print("[%d] %s" %(i, server_msg))
        bench_send(clnt_socket, server_msg)
        response, response_size = bench_recv(clnt_socket, PACKET_SIZE, fd)
        print("[response:%d] %s" % (i, response))
        print("-"*80)
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
            response, response_size = send_smartthermostat_cmds(client, fd)
            if benchmark_type == 0:
                client.send("ack\0")
            while response != IOT2_END_TCP_DRIVER_MSG:
                response, response_size = bench_recv(client, PACKET_SIZE, fd)
            # send the following only for bare-metal benchmark

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
    args = arg_parser.parse_args()
    benchmark_type = 1  # OS benchmark
    if args.run_baremetal:
        benchmark_type = 0
    net_driver("192.168.0.10", 1337, 1, "nothing.txt", "nothing", "nothing", benchmark_type)

