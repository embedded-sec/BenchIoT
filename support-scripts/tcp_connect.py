import socket
import sys
import time

number_of_packets = 1000


#######################################################################
#                              GLOBALS                                #
#######################################################################

# network setup
host_ip = '192.168.0.10'
port = 1337



#######################################################################
#                              FUNCTIONS                              #
#######################################################################

def tcp_client():
    while True:
        try:
            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client.settimeout(10)
            client.connect((host_ip, port))
            client.settimeout(None)
            print("connected!")
            msg = "A"*255
	    bytes_sent = client.send(msg)
	    i= 0
            # check the content of the first message to start recording the results
            while i != number_of_packets:
                print("-" * 80)
                response = client.recv(8192)
                print("Iteration: %d" % i)
                print(response)
                print(len(response))
                print(response.split())
                print("-" * 80)
                send_msg = "".join("%04d" % i)
                bytes_sent = client.send(msg)  # msg+str(i+10))
                print("SEND = %d" % bytes_sent)
                i += 1
                if i == 1000:
                    i = 0
        except KeyboardInterrupt:
            client.close()
            print("[-] CTRL-C pressed, closing....")
            return
        except Exception as e:
            print("[ERROR] Exception: %s" % (str(e)))
            client.close()
            pass

if __name__ == '__main__':
    tcp_client()
