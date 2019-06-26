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
BareMetalEthernet *iot2_ethernet_interface_ptr = NULL;

/// used to start the energy measurements, this happens after enabling measuring
/// all metrics
extern DigitalOut energyGPIO;

//========================================= FUNCTIONS ===========================================//

/// Initializes ethernet and tcp
void iot2InitTCP(BareMetalEthernet *eth, BareMetalTCP *socket){

    char accept_ack[4] = {0};
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

    // initialize a server
    socket->init();

   iot2SerialDebugMsg("[+] Server opened");
    
    // Bind port
    if(socket->bind(IoT2_PORT) < 0){
       iot2SerialDebugMsg("[-] ERROR: Failed to bind");
       iot2Error();
    }

    iot2SerialDebugMsg("[+] Listening");

    // for the baremetal verison, accept is called inside listen
    socket->listen();

    iot2recv(socket, accept_ack, sizeof(accept_ack));
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

/// send function
nsapi_size_or_error_t iot2send(BareMetalTCP *socket, const void *data, nsapi_size_t size){

    nsapi_size_or_error_t send_bytes = 0;
    // [iot2-debug]: potential issues with type confusion here. Lower level API require
    // uint16 and benchmarks appear to use unsigned int. Fix later on.
    uint16_t data_size = (uint16_t) size;

    // error should be handled at bench side
    send_bytes = socket->send(socket->clntPcb, data, data_size);


    return send_bytes;
}

/// wrapper function for recv. Used to enable unified api for bare-metal/os apps
nsapi_size_or_error_t iot2recv(BareMetalTCP *socket, void *data, nsapi_size_t size){
    
    nsapi_size_or_error_t bytes = 0;
    
    // [iot2-debug]: potential issues with type confusion here. Lower level API require
    // uint16 and benchmarks appear to use unsigned int. Fix later on.
    uint16_t data_size = (uint16_t) size;

    bytes = socket->recv(data, data_size);

    return bytes;
}

bool ethernetInitialized(void){
    return iot2_ethernet_initialized;
}

BareMetalEthernet* getEthernetInterface(void){
    return iot2_ethernet_interface_ptr;
}