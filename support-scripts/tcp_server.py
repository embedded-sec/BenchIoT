import socket
import sys
import time
HOST = '192.168.0.11'
PORT = 8888
number_of_packets = 1000


def tcp_server():
    global number_of_packets
    while True:
        try:
            server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server.bind((HOST, PORT))
            server.listen(11)
            print ("connected!")
            msg = "TCP_ECHO_DEMO_"
            i = 0
            clnt, addr = server.accept()
            print("ACCEPTED CLIENT @addr=%s", str(addr))
            while i != number_of_packets:
                print("-" * 80)
                response = clnt.recv(8192)
                print("Iteration: %d" %i)
                print(response)
                print(len(response))
                print(response.split())
                print("-" * 80)
                send_msg = "".join("%04d" %i)
                bytes_sent = clnt.send(send_msg)#msg+str(i+10))
                print("SEND = %d" %bytes_sent)
                i += 1
                if i == 1000:
                    i = 0
        except KeyboardInterrupt:
            server.close()
            print("[-] CTRL-C pressed, closing....")
            return
        except Exception as e:
            print("[ERROR] Exception: %s" %(str(e)))
            server.close()
            pass

if __name__ == '__main__':
    tcp_server()
