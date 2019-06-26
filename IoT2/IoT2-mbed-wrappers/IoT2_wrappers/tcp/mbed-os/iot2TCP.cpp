//=================================================================================================
//
// This file is used to print error messages when debugging is enabled
//
//=================================================================================================



//=========================================== INCLUDES ==========================================//

#include "mbed.h"
#include "iot2TCP.h"
//======================================== DEFINES & GLOBALS ====================================//

// This variable is used to indicate that the Ethernet peripheral has been
// initialized correctly.
bool volatile iot2_ethernet_initialized = false;
// pointer to the etherenet interface used by the benchmark
EthernetInterface *iot2_ethernet_interface_ptr = NULL;

/// used to start the energy measurements, this happens after enabling measuring
/// all metrics
extern DigitalOut energyGPIO;

//========================================= FUNCTIONS ===========================================//

/// Initializes ethernet and tcp
void iot2InitTCP(EthernetInterface *eth, TCPSocket *socket,\
                    TCPServer *tcpserver, SocketAddress *sockaddr){

    
    // Initialize Ethernet and TCP
    if(eth->set_network(BOARD_IP, NETMASK, GATEWAY)){
        iot2SerialDebugMsg("[-] ERROR: Unable to setup network");
        iot2Error();
    }

    // connect ethernet
    if(eth->connect()){
        iot2SerialDebugMsg("[-] ERROR: Unable to connect Ethernet");
        iot2Error();
    }

    if(tcpserver->open(eth)){
        iot2SerialDebugMsg("[-] ERROR: TCPServer.open()");
        iot2Error();

    }

   iot2SerialDebugMsg("[+] Server opened");
    
    if(tcpserver->bind(BOARD_IP,IoT2_PORT) < 0){
        iot2SerialDebugMsg("[-] ERROR: Unable to bind IP and port");
        iot2Error();
    }

    if (tcpserver->listen() < 0) {
        iot2SerialDebugMsg("[-] ERROR: TCPServer.listen()");
        iot2Error();
    }

    iot2SerialDebugMsg("[+] Listening");

    // The while loop is to solve the bug of returning the NSAPI_ERROR_WOULD_BLOCK
    // error for the first call to accept.
    while (tcpserver->accept(socket, sockaddr) < 0) {
        iot2SerialDebugMsg("[-] ERROR: TCP accept");
    }

    iot2SerialDebugMsg("[+] Socket Accepted");

    // ethernet has been initialized correctly. Setup global pointer and
    // and state so that other sockets can use the the ethernet object
    iot2_ethernet_interface_ptr = eth;
    iot2_ethernet_initialized = true;

    // reset counters and enable all metrics
    iot2CallSVC(IOT2_RESERVED_SVC,IOT2_DWT_REST_SETUP_SVC_NUM);
    // enable measuremets
    COLLECT_IOT2_METRICS = 1;

    // initialize priv thread counter
    iot2RecordPrivThread();

    // set the energy gpio to 1 for energy measurement
    energyGPIO = 1;

}

/// send function, ensures sending even when non-blocking. Should ensure sending
/// all the data as one bulk without fragments to simplify collcting the results
/// on desktop network driver.
nsapi_size_or_error_t iot2send(TCPSocket *socket, const void *data, nsapi_size_t size){

    nsapi_size_or_error_t send_bytes = 0, s_size = size;

    bool send_incomplete = true;

    while(send_incomplete){
        
        send_bytes = socket->send(data, size);
        if (send_bytes == NSAPI_ERROR_WOULD_BLOCK){
            iot2SerialDebugMsg("[-] ERROR: socket->send returned [NASPI_ERROR_WOULD_BLOCK]");
            //wait(0.1);
                
        }

        else if(send_bytes > 0 && send_bytes < s_size){
            iot2SerialDebugMsg("[-] ERROR: socket->send sent bytes less than specified size");
            send_incomplete = false;
        }
        
        else{
            send_incomplete = false;
        }

    }

    return send_bytes;
}

/// wrapper function for recv. Used to enable unified api for bare-metal/os apps
nsapi_size_or_error_t iot2recv(TCPSocket *socket, void *data, nsapi_size_t size){
    // changed to while loop to avoid single error
    nsapi_size_or_error_t bytes = -1;
    while(bytes < 0){
        bytes = socket->recv(data, size);
        if (bytes < size && bytes > 0){
            //__ASM("bkpt\n\t");
            iot2SerialDebugMsg("[-] ERROR: socket->recv sent bytes less than specified size");
        }
        else if (bytes == NSAPI_ERROR_WOULD_BLOCK){
            iot2SerialDebugMsg("[-] ERROR: socket->recv NSAPI_ERROR_WOULD_BLOCK");
        }

    }
    //nsapi_size_or_error_t bytes = socket->recv(data, size);
    return bytes;
}

bool ethernetInitialized(void){
    return iot2_ethernet_initialized;
}

EthernetInterface* getEthernetInterface(void){
    return iot2_ethernet_interface_ptr;
}