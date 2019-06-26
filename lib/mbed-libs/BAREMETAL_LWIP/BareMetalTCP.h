/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of and a contribution to the lwIP TCP/IP stack.
 *
 * Credits go to Adam Dunkels (and the current maintainers) of this software.
 *
 * Christiaan Simons rewrote this file to get a more stable echo application.
 *
 **/

 /* This file was modified by ST */


#ifndef BAREMETALTCP_H
#define BAREMETALTCP_H

//===============================================================================================//
//
// This is a TCP wrapper class for baremetal systems.
//
//==============================================================================================//


//========================================== INCLUDES ==========================================//

// mbed files
#include "mbed.h"

// LwIP files
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"
#include "lwip/inet.h"
#include "lwip/ip4_addr.h"
// board specific files
#include "eth_arch.h"
#include "BareMetalEthernet.h"
//======================================== DEFINES & GLOBALS ====================================//

/* ECHO protocol states */
enum baremetalTCPstates
{
  BMTCP_NONE = 0,
  BMTCP_ACCEPTED = 1,
  BMTCP_CONNECTED = 1,      // CONNECTED == ACCEPTED to use the same recv functionality
  BMTCP_RECEIVED = 2,
  BMTCP_CLOSING = 3
};


/* structure for maintaing connection infos to be passed as argument 
   to LwIP callbacks*/
struct baremetalTCPstruct
{
  u8_t state;             /* current connection state */
  u8_t retries;
  struct tcp_pcb *pcb;    /* pointer on the current tcp_pcb */
  struct pbuf *p;         /* pointer on the received/to be transmitted pbuf */
};


class BareMetalTCP
{
public:
    /// Constructor
    BareMetalTCP();

    /// Destructor
    ~BareMetalTCP();

    /// @brief Initializes TCP socket
    /// @param None
    /// @retval None
    bool init(void);

    /// @brief wrapper function for tcp_bind
    /// @param port number
    /// @retval @retval 0 on success, -1 on failure
    int bind(uint16_t port);

    /// @brief Wrapper function for tcp_listen
    /// @param None
    /// @retval None
    void listen(void);


    void set_eth_ptr(BareMetalEthernet *eth);

    /// @brief Callback for accept (for server) and connect (for client)
    /// @param Pointet to baremetalTCPstruct, not used in this function
    /// @param Pointer to new tcp_pcb connection
    /// @param Error value, not used in this function
    /// @retval Error value: ERR_OK if it succeeds.
    err_t callback_accept_connect(void *arg, struct tcp_pcb *newpcb, err_t err);

    /// @brief wrapper function for tcp_connect
    /// @param ip address
    /// @param port port number
    /// @retval ERR_OK on success, other on failure
    err_t connect(const char *ip, uint16_t port);

    /// @brief Callback function to receive data from a TCP connection
    /// @param Pointet to baremetalTCPstruct
    /// @param Pointer to tcp_pcb connection
    /// @param Pointer to the received data struct
    /// @param Error value associated with the pbuf struct
    /// @retval Error value: ERR_OK if it succeeds.
    err_t callback_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

    /// @brief Receive data from a TCP connection
    /// @param Pointet to baremetalTCPstruct
    /// @param Pointer to tcp_pcb connection
    /// @param Pointer to the received data struct
    /// @param Error value associated with the pbuf struct
    /// @retval Error value: ERR_OK if it succeeds.
    int recv(void *buff, uint16_t buff_size);

    /// @brief Send datat to remote TCP connection
    /// @param Pointer to TCP program control block
    /// @param Pointer to baremetal TCP structure
    /// @retval None
    int send(struct tcp_pcb *tpcb, const void* buff , uint16_t len);

    /// @brief Send  remaining data or acknowledgment if all data has been delivered
    /// @param Pointet to baremetalTCPstruct
    /// @param Pointer to tcp_pcb connection
    /// @param data length
    /// @retval error state
    err_t callback_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

    /// @brief Function to catch errors. Implemented as a simple infinite loop
    /// @param None
    /// @retval None
    void close(struct tcp_pcb *tpcb, struct baremetalTCPstruct *bTCPstrct);

    /// @brief Function to catch errors. Implemented as a simple infinite loop
    /// @param None
    /// @retval None
    err_t callback_poll(void *arg, struct tcp_pcb *tpcb);

    /// @brief Function to catch errors. Implemented as a simple infinite loop
    /// @param None
    /// @retval None
    void callback_error(void *arg, err_t err);

    /// @brief Frees the received pbuf
    /// @retval NULL
    struct pbuf* freePBUF(struct pbuf *p);

    // @brief Copy the received packet in callback_recv to data member recvPBUF
    // @pararm pointer to bTCPstrct pbuf
    // @retval ERR_OK if copied correctly, error otherwise
    err_t cpyRecvPBUF(struct pbuf *pkt_buf);

    /// @brief Checks if callback_recv has finished
    /// @param None
    /// @retval True if has finished, False otherwise
    bool isCallbackRecvFinished(void);

    /// @brief This function is used to store an incomplete trainling part of a
    ///        of packet in a temporary pbuf. The temporary pbuf is copied to 
    ///        begining of recvPBUF at the next call of cpyRecvPBUF.
    /// @param None
    /// @retval ERR_OK if copied correctly, ERR_MEM otherwise.
    err_t storeTempPBUF(void);


    DigitalOut redLed;
    Serial bmTCPserial;
    struct tcp_pcb *baremetalTCPpcb;
    err_t err;
    struct tcp_pcb *clntPcb;             // Points to new_pcb after accept
    bool volatile callback_recv_finished;// Indicates if callback_recv has finished
    uint16_t recvBuffSize;               // The recv size set by the user
    uint16_t recvedBytes;                // Counts the received bytes until it reaches recvBuffSize
    struct pbuf *recvPBUF;               // The received pbuf
    uint16_t recvOffset;                 // Offset in current recvPBUF
    struct pbuf *tempPBUF;               // Temporary trailing part of recvPBUF
    uint16_t tempPBUFSize;
    BareMetalEthernet *eth_ptr;
};

//========================================= FUNCTIONS ===========================================//




#endif  // BAREMETALTCP_H //