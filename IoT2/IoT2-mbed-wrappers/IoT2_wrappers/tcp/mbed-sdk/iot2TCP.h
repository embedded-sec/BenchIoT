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
#include "BareMetalEthernet.h"
#include "BareMetalTCP.h"
#include "lwip/timeouts.h"



//======================================== DEFINES & GLOBALS ====================================//



//========================================= FUNCTIONS ===========================================//

/// Initializes ethernet and tcp
void iot2InitTCP(BareMetalEthernet *eth, BareMetalTCP *socket);


nsapi_size_or_error_t iot2send(BareMetalTCP *socket, const void *data, nsapi_size_t size);

nsapi_size_or_error_t iot2recv(BareMetalTCP *socket, void *data, nsapi_size_t size);

bool ethernetInitialized(void);

BareMetalEthernet* getEthernetInterface(void);

#endif  // IOT2_TCP_H //