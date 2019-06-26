//=================================================================================================
//
// This file is used to simplify initializig ethernet and resolve non-blocking send/recv
// calls in the main application.
//
//=================================================================================================


#ifndef IOT2_TCP_H
#define IOT2_TCP_H


//=========================================== INCLUDES ==========================================//

// IoT2 configuration and special interface files
#include "IoT2_Config.h"
#include "iot2Debug.h"

// mbed files
#include "mbed.h"

// board specific files
#include "benchmark_target.h"

// Network files
#include "TCPSocket.h"
#include "EthernetInterface.h"
#include "TCPSocket.h"
#include "SocketAddress.h"



//======================================== DEFINES & GLOBALS ====================================//

// This variable is used to indicate that the Ethernet peripheral has been
// initialized correctly.
//extern bool volatile iot2_ethernet_initialized;
// pointer to the etherenet interface used by the benchmark
//extern EthernetInterface *iot2_ethernet_interface_ptr;

//========================================= FUNCTIONS ===========================================//

/// Initializes ethernet and tcp
void iot2InitTCP(EthernetInterface *eth, TCPSocket *socket,\
                    TCPServer *tcpserver, SocketAddress *sockaddr);


nsapi_size_or_error_t iot2send(TCPSocket *socket, const void *data, nsapi_size_t size);

nsapi_size_or_error_t iot2recv(TCPSocket *socket, void *data, nsapi_size_t size);

bool ethernetInitialized(void);

EthernetInterface* getEthernetInterface(void);

#endif  // IOT2_TCP_H //