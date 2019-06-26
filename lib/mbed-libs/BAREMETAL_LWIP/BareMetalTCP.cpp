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


//===============================================================================================//
//
// This is a TCP wrapper class for baremetal systems.
//
//===============================================================================================//



//========================================== INCLUDES ===========================================//


// Class header file
#include "BareMetalTCP.h"


//====================================== DEFINES & GLOBALS ======================================//

/// @brief global variable used to allow callbacks to class functions
BareMetalTCP* globalTrampolinePtr = NULL;


//-----------------------------------------------------------------------------------------------//
//                                      Callback Trampolines                                     //
//-----------------------------------------------------------------------------------------------//

// @brief Trampoline function to callback_accept
/// @param arg Pointer to baremetalTCPstruct
/// @param newpcb Pointer to new tcp_pcb connection
/// @param err Error value, not used in this function
/// @retval Error value: ERR_OK if it succeeds.
static err_t trampoline_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

/// @brief Trampoline function to callback_recv
/// @param arg Pointer to baremetalTCPstruct
/// @param tpcb Pointer to tcp_pcb connection
/// @param p Pointer to the received pbuf data struct
/// @param Error value associated with the pbuf struct
/// @retval Error value: ERR_OK if it succeeds.
static err_t trampoline_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

/// @brief Trampoline function to callback_sent
/// @param arg Pointer to baremetalTCPstruct
/// @param tpcb Pointer to tcp_pcb connection
/// @param len data length
/// @retval error state
static err_t trampoline_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

/// @brief Trampoline function to callback_poll
/// @param None
/// @retval None
static err_t trampoline_poll(void *arg, struct tcp_pcb *tpcb);

/// @brief Trampoline function to callback_error
/// @param None
/// @retval None
static void trampoline_error(void *arg, err_t err);

/// @brief Trampoline function for callback_connect (maps to callback_accept_connect)
static err_t trampoline_connect(void *arg, struct tcp_pcb *tpcb, err_t err);

//========================================== FUNCTIONS ==========================================//


// only include in case TCP is used
#if LWIP_TCP

/// Constructor
BareMetalTCP::BareMetalTCP(): redLed(LED_RED), bmTCPserial(USBTX, USBRX){
    
    
    globalTrampolinePtr = this;                                 // Store global trampoline pointer
    this->redLed = 1;                                           // Set red LED to be off
    this->callback_recv_finished = false;
    this->recvBuffSize = 0;
    this->recvedBytes = 0;
    this->recvOffset = 0;
    this->recvPBUF = NULL;
    this->tempPBUF = NULL;
    this->tempPBUFSize = 0;
    this->eth_ptr = NULL;
}

/// Destructor
BareMetalTCP::~BareMetalTCP(){

    // Check recvPBUF has been freed, if not then free it.
    if(this->recvPBUF != NULL){
       this->recvPBUF =  this->freePBUF(this->recvPBUF);
    }

    // Check tempPBUF has been freed, if not then free it.
    if(this->tempPBUF != NULL){
        this->tempPBUF = this->freePBUF(this->tempPBUF);
    }

}


/// @brief Initializes the tcp socket
/// @param None
/// @retval None
bool BareMetalTCP::init(){

    this->baremetalTCPpcb = tcp_new();

    // Verify and an error did not occur during the allocation within tcp_new
    if(this->baremetalTCPpcb == NULL){

        // an error occured, deallocate
        memp_free(MEMP_TCP_PCB,this->baremetalTCPpcb);
        return false;
    }

    return true;
}


/// @brief wrapper function for tcp_bind
/// @param port number
/// @retval 0 on success, -1 on failure
int BareMetalTCP::bind(uint16_t port){

    this->err = tcp_bind(this->baremetalTCPpcb, IP_ADDR_ANY, port);
    
    if (this->err != ERR_OK){

        // an error occured, deallocate
        memp_free(MEMP_TCP_PCB, this->baremetalTCPpcb);
        return -1;
    }

    return 0;
}


/// @brief Wrapper function for tcp_listen
/// @param None
/// @retval None
void BareMetalTCP::listen(){

    this->baremetalTCPpcb = tcp_listen(this->baremetalTCPpcb);

    // Use this class accept as the callback function of LwIP tcp_accept
    tcp_accept(this->baremetalTCPpcb, trampoline_accept);
}


void BareMetalTCP::set_eth_ptr(BareMetalEthernet *eth){
    this->eth_ptr = eth;
}

/// @brief Accept a connection
/// @param Pointet to baremetalTCPstruct, not used in this function
/// @param Pointer to new tcp_pcb connection
/// @param Error value, not used in this function
/// @retval Error value: ERR_OK if it succeeds.
err_t BareMetalTCP::callback_accept_connect(void *arg, struct tcp_pcb *newpcb, err_t err){

    // A structure to maintain conneciton information
    struct baremetalTCPstruct *bTCPstrct;

    // Avoid not used arguments warning
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);

    // Set priority for the new TCP connection
    tcp_setprio(newpcb, TCP_PRIO_MIN);

    // allocate memory for bTCPstrct
    bTCPstrct = (struct baremetalTCPstruct *)mem_malloc(sizeof(struct baremetalTCPstruct));

    if (bTCPstrct != NULL){

        // Populate the bTCPstrct to reflect that a connectio has been accepeted
        bTCPstrct->state = BMTCP_ACCEPTED;      // Connection state flag
        bTCPstrct->pcb = newpcb;                // Pointer to new tcp_pcb connection
        bTCPstrct->retries = 0;                 // Number of retires
        bTCPstrct->p = NULL;                    // Pointer to pbuf for recv/send

        // Pass bTCPstrct as an arg with newpcb
        tcp_arg(newpcb,bTCPstrct);

        // Initialize callbacks to LwIP's tcp_* functions
        tcp_recv(newpcb, trampoline_recv);          // Map callback of tcp_recv
        tcp_err(newpcb, trampoline_error);          // Map callback of tcp_err
        tcp_poll(newpcb, trampoline_poll, 0);       // Map callback of tcp_poll

        this->err = ERR_OK;
    }
    // If an error occured
    else{

        this->close(newpcb, bTCPstrct);         // Close connection and free the struct
        this->err = ERR_MEM;
    }

    // update the current pcb to newpcb, this is to return a reference to the
    // main application.
    if (this->err == ERR_OK){
        this->clntPcb = newpcb;
    }

    return this->err;
}


/// @brief wrapper function for tcp_connect
/// @param ip address
/// @param port port number
/// @retval ERR_OK on success, other on failure
err_t BareMetalTCP::connect(const char *ip, uint16_t port){
    ip4_addr_t ip_addr;
    inet_aton(ip, &ip_addr);
    //IP4_ADDR(&ip_addr, 192, 168,0,11);
    if (this->baremetalTCPpcb == NULL){
        while(1){;}
    }
    err_t err = tcp_connect(this->baremetalTCPpcb, &ip_addr, port, trampoline_connect);
    return err;
}


/// @brief Receive data from a TCP connection
/// @param arg Pointer to baremetalTCPstruct
/// @param tpcb Pointer to tcp_pcb connection
/// @param pbuf Pointer to the received data struct
/// @param err Error value associated with the pbuf struct
/// @retval Error value: ERR_OK if it succeeds.
err_t BareMetalTCP::callback_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err){

  
    struct baremetalTCPstruct *bTCPstrct;   // A structure to maintain conneciton information
    u16_t prevTotLen = 0;                   // Previous tot_len before chaining

    LWIP_ASSERT("arg != NULL", arg != NULL);

    bTCPstrct = (struct baremetalTCPstruct *)arg;

    // There are 6 cases when we receive a pbuf:
    //      1) Empty -> Set the state to closing and close the connection
    //      2) Non-empty with err != ERR_OK -> Free pbuf and return the given error
    //      3) Accepted -> Set the state to received and store the received data
    //      4) Received -> Update pbuf_chain accordingly
    //      5) Closing -> An odd case where remote side closes twice, so free pbuf.
    //      6) Uknown state ->  Trash data, so just free pbuf.
    // Note that 5 & 6 are handled the same way. It might be helpful to combine them in
    // on case.[iot2-debug]

    //-------------------------------------------------------------------------------------------//
    //                                  Case 1: Empty pbuf                                       //
    //-------------------------------------------------------------------------------------------//
    
    // pbuf is empty, so remote closed the connection.
    if (p == NULL){

        bTCPstrct->state = BMTCP_CLOSING;
        
        // if the buffer is empty close directly, otherwise ack then close
        if (bTCPstrct->p == NULL){

            this->close(tpcb, bTCPstrct);
            this->callback_recv_finished = true;
        }
        else{
            
            // Free the  recv pbuf 
            tcp_recved(tpcb,bTCPstrct->p->tot_len);
            bTCPstrct->p = this->freePBUF(bTCPstrct->p);
            this->callback_recv_finished = true;
            
        }
        this->err = ERR_OK;
    }

    //------------------------------------------------------------------------------------------//
    //                                  Case 2: err != ERR_OK                                   //
    //------------------------------------------------------------------------------------------// 

    else if(err != ERR_OK){
        // Free pbuf
        if (p != NULL){

            bTCPstrct->p = NULL;
            pbuf_free(p);
        }
        this->err = err;
    }

    //-------------------------------------------------------------------------------------------//
    //                                  Case 3: BMTCP_ACCEPTED                                   //
    //-------------------------------------------------------------------------------------------//

    else if (bTCPstrct->state == BMTCP_ACCEPTED){

        bTCPstrct->state = BMTCP_RECEIVED;           // Update the state after first received chunk
        bTCPstrct->p = p;                            // Reference to pbuf chain
        
        this->recvedBytes += bTCPstrct->p->tot_len;  // update the received bytes size

        // check if we received the specified recv buffer size by the user
        this->callback_recv_finished = this->isCallbackRecvFinished();

        if (this->callback_recv_finished){
            
            this->cpyRecvPBUF(bTCPstrct->p);         // Copy the recieved pbuf
            
            // Free the  recv pbuf [iot2-debug]
            tcp_recved(tpcb,bTCPstrct->p->tot_len);
            bTCPstrct->p = this->freePBUF(bTCPstrct->p);

        }

        this->err = ERR_OK;                          // Set err to ERR_OK

    }

    //-------------------------------------------------------------------------------------------//
    //                                  Case 4: BMTCP_RECEIVED                                   //
    //-------------------------------------------------------------------------------------------//

    else if(bTCPstrct->state == BMTCP_RECEIVED){

        // If new data is received
        if(bTCPstrct->p == NULL){
            
            // Store the reference to begining of the data
            bTCPstrct->p = p;

        }
        // Otherwise, the received data is a another chunk of the received data
        else{

            struct pbuf *ptr;
            ptr = bTCPstrct->p;            
            
            prevTotLen = bTCPstrct->p->tot_len;           // Previous tot_len before chaining
            pbuf_chain(ptr, p);                           // Chain the received data to old one
        }

        // update the received bytes size
        this->recvedBytes += bTCPstrct->p->tot_len - prevTotLen;
        prevTotLen = 0;                                   // Reset prevTotLen

        // check if we received the specified recv buffer size by the user
        this->callback_recv_finished = this->isCallbackRecvFinished();
        
        if (this->callback_recv_finished){

            this->cpyRecvPBUF(bTCPstrct->p);              // Copy the recieved pbuf
            // Free recv pbuf [iot2-debug]
            tcp_recved(tpcb,bTCPstrct->p->tot_len);
            bTCPstrct->p = this->freePBUF(bTCPstrct->p); // Free pbuf if callback_recv is done
            
        }

        this->err = ERR_OK;
    }

    //------------------------------------------------------------------------------------------//
    //                                  Case 5: BMTCP_CLOSING                                   //
    //------------------------------------------------------------------------------------------// 

    else if(bTCPstrct->state == BMTCP_CLOSING){
        
        // Remote closed, so stop receiving. If recvbytes != recvBuffSize then an error will be
        // catched at recv
        this->recvedBytes += bTCPstrct->p->tot_len;     // update the received bytes size
        this->cpyRecvPBUF(bTCPstrct->p);                // Copy the recieved pbuf
        this->callback_recv_finished = true;            // Remote closed so stop callback_recv

        // Remote closed twice here, so the received data is irrelavent
        tcp_recved(tpcb, p->tot_len);
        bTCPstrct->p = NULL;
        pbuf_free(p);

        this->err = ERR_OK;
    }

    //------------------------------------------------------------------------------------------//
    //                                  Case 6: Unkown state                                    //
    //------------------------------------------------------------------------------------------// 
    else{

        // This is unknown state. The data here is irrelavent so no copying is needed.[iot2-debug]
        // The data here is also considered trash data
        tcp_recved(tpcb, p->tot_len);
        bTCPstrct->p = NULL;
        pbuf_free(p);
        this->err = ERR_OK;

    }

    return this->err;

}

// [iot2-debug] fix the function below to check if recvedBytes > buffSize
/// @brief Receive data from a TCP connection
/// @param Pointet to baremetalTCPstruct
/// @param Pointer to tcp_pcb connection
/// @param Pointer to the received data struct
/// @param Error value associated with the pbuf struct
/// @retval Number of charecters recieved on success, negative on failure
int BareMetalTCP::recv(void *buff, uint16_t buff_size){

    // Set the current receive buffer size
    this->recvBuffSize = buff_size;
    
    
    // There are 3 cases when calling recv:
    //      1) recvPBUF has trailing data from preivous callback_recv, which is
    //         less than buff_size. Then save this data to tempPBUF and receive
    //         the extra data by polling.
    //      2) recvPBUF is NULL, so start polling directly.
    //      3) recvPBUF has data > buff_size, so return these data immediatly
    //         and do not poll.


    //-------------------------------------------------------------------------------------------//
    //                                      store tempPBUF                                       //
    //-------------------------------------------------------------------------------------------//

    if(this->recvedBytes != 0 && (this->recvedBytes - this->recvOffset) < this->recvBuffSize){
        // Store trailing data to tempPBUF
        if (this->storeTempPBUF() != ERR_OK){
            return -1;
        }
        // Free recvPBUF
        this->recvPBUF = this->freePBUF(this->recvPBUF);
    }

    //-------------------------------------------------------------------------------------------//
    //                                       recvPBUF==NULL                                      //
    //-------------------------------------------------------------------------------------------//

    // Check that we do not have previously recieved data. if not that start
    // lwip callback loop
    if(this->recvPBUF == NULL){

        this->recvedBytes = 0;
        this->recvOffset = 0;
        // [iot2-debug]: check if restart_timeouts is actually needed
        // The goal of using it here is if we do not use recv fo a long time.
        sys_restart_timeouts();
        while(!this->callback_recv_finished){
        
            this->eth_ptr->get_input();
            sys_check_timeouts();
        }

        // Reset callback_recv_finished to false for next call of recv
        this->callback_recv_finished = false;
    }

    // Check that recvedBytes >= recvBuffSize
    if (this->recvedBytes < this->recvBuffSize){
        // An error occured, return
        return -1;
    }

    //-------------------------------------------------------------------------------------------//
    //                                  Copy data to user buff                                   //
    //-------------------------------------------------------------------------------------------//

    // Copy the data to buff, we might only need to copy a part of the data
    // as recvPBUF might receive multiple packets at once.
    uint16_t pbufBytes = pbuf_copy_partial(this->recvPBUF, buff, buff_size, this->recvOffset);
    this->recvOffset += pbufBytes;      // Update recvOffset

    // Check if recvPBUF has been totally copied. If yes then free recvPBUF
    if (this->recvOffset >= this->recvedBytes){
        
        if (this->recvOffset > this->recvedBytes){
            // An error occured
            return -1;

        }

        // Free recvPBUF and reset recvedBytes 
        this->recvPBUF = this->freePBUF(this->recvPBUF);
        this->recvedBytes = 0;
        this->recvOffset = 0; 
    }

    return recvBuffSize;

}


/// @brief Send datat to remote TCP connection
/// @param Pointer to TCP program control block
/// @param Pointer to baremetal TCP structure
/// @retval Number of charecters send on success, negative on failure
int BareMetalTCP::send(struct tcp_pcb *tpcb, const void* buff , uint16_t len){
    
    tcp_sent(tpcb, trampoline_sent);
    struct pbuf *sndPBUF;
    err_t sndErr = ERR_OK;
    sndPBUF = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
    memcpy(sndPBUF->payload, buff, len);

    //((char*)(sndPBUF->payload))[len] = '\0';
    
    while( (sndErr = tcp_write(tpcb, sndPBUF->payload, sndPBUF->len, 1) != ERR_OK));
    sndErr = tcp_output(tpcb);
    if (sndErr != ERR_OK){
        return -1;
    }
    tcp_recved(tpcb,sndPBUF->len);
    sndPBUF = this->freePBUF(sndPBUF);
    return len;

}


/// @brief Send  remaining data or acknowledgment if all data has been delivered
/// @param Pointet to baremetalTCPstruct
/// @param Pointer to tcp_pcb connection
/// @param data length
/// @retval error state
err_t BareMetalTCP::callback_sent(void *arg, struct tcp_pcb *tpcb, u16_t len){

    // A structure to maintain conneciton information
    struct baremetalTCPstruct *bTCPstrct;

    LWIP_UNUSED_ARG(len);
    bTCPstrct = (struct baremetalTCPstruct *)arg;
    bTCPstrct->retries = 0;

    // If pbuf != NULL, then there is still data that need to be sent.
    if(bTCPstrct->p != NULL){
        bmTCPserial.printf("[+] @callback_sent, calling send next!!\r\n");
        //tcp_sent(tpcb, trampoline_sent);
        //this->send(tpcb, bTCPstrct, bTCPstrct->p->len);
    }
    else{

        // No more data to send, close the connection
        if (bTCPstrct->state == BMTCP_CLOSING){
            this->close(tpcb, bTCPstrct);
        }
    }

    this->err = ERR_OK;
    return this->err;
}


/// @brief Function to catch errors. Implemented as a simple infinite loop
/// @param None
/// @retval None
void BareMetalTCP::close(struct tcp_pcb *tpcb, struct baremetalTCPstruct *bTCPstrct){
     /* remove all callbacks */
      tcp_arg(tpcb, NULL);
      tcp_sent(tpcb, NULL);
      tcp_recv(tpcb, NULL);
      tcp_err(tpcb, NULL);
      tcp_poll(tpcb, NULL, 0);
      
      /* delete es structure */
      if (bTCPstrct != NULL)
      {
        mem_free(bTCPstrct);
      }  
      
      /* close tcp connection */
      tcp_close(tpcb);
}


/// @brief Function to catch errors. Implemented as a simple infinite loop
/// @param None
/// @retval None
err_t BareMetalTCP::callback_poll(void *arg, struct tcp_pcb *tpcb){
    
    err_t ret_err;
    
    // A structure to maintain conneciton information
    struct baremetalTCPstruct *bTCPstrct;

    bTCPstrct = (struct baremetalTCPstruct *)arg;
    if (bTCPstrct != NULL)
    {
        if (bTCPstrct->p != NULL)
        {
            //tcp_sent(tpcb, trampoline_sent);
            /* there is a remaining pbuf (chain) , try to send data */
            //this->send(tpcb, bTCPstrct, bTCPstrct->p->len);
        }
        else
        {
            /* no remaining pbuf (chain)  */
            if(bTCPstrct->state == BMTCP_CLOSING)
            {
                /*  close tcp connection */
                this->close(tpcb, bTCPstrct);
            }
        }   
        ret_err = ERR_OK;
    }
    else
    {
        /* nothing to be done */
        tcp_abort(tpcb);
        ret_err = ERR_ABRT;
    }
    return ret_err;
}


/// @brief Function to catch errors. Implemented as a simple infinite loop
/// @param None
/// @retval None
void BareMetalTCP::callback_error(void *arg, err_t err){

    // A structure to maintain conneciton information
    struct baremetalTCPstruct *bTCPstrct;

    LWIP_UNUSED_ARG(err);

    bTCPstrct = (struct baremetalTCPstruct *)arg;
    if(bTCPstrct != NULL){
        mem_free(bTCPstrct);
    }

    // An error occured! Loop inifinitely
    
    redLed = 0; // turn red led on
    
}


/// @brief Frees the received pbuf
/// @retval NULL
struct pbuf* BareMetalTCP::freePBUF(struct pbuf *p){

    struct pbuf *curr_pbuf;
    u8_t freed;
    //this->bmTCPserial.printf("[-] iot2-debug: @freePBUF\r\n");
    // Free pbuf (a while loop in case of pbuf chain)
    while(p != NULL){
        curr_pbuf = p;
        p = curr_pbuf->next;

        if (p != NULL){
            pbuf_ref(p);
        }
        do{
                // Free pbuf 
                freed = pbuf_free(curr_pbuf);
            }while(freed == 0);

    }
    //this->bmTCPserial.printf("[-] iot2-debug: @freePBUF--->END\r\n");
    return NULL;

}


/// @brief Checks if callback_recv has finished
/// @param None
/// @retval True if has finished, False otherwise
bool BareMetalTCP::isCallbackRecvFinished(void){

    // check if we received the specified recv buffer size by the user
    if (this->recvedBytes >= this->recvBuffSize){
        
        return true; // Set callback_recv_finished = true to return to recv
    }
    
    return false;
}


/// @brief Copy the received packet in callback_recv to data member recvPBUF
/// @pararm pointer to bTCPstrct pbuf
/// @retval ERR_OK if copied correctly, error otherwise
err_t BareMetalTCP::cpyRecvPBUF(struct pbuf *pkt_buf){

    //this->bmTCPserial.printf("[-] iot2-debug: @cpyRecvPBUF\r\n");
    struct pbuf *curr_pbuf;
    uint16_t offset = 0;
    
    // [iot2-debug]: Check tot_len == recvBuffSize
    if (pkt_buf->tot_len != this->recvedBytes){
        this->bmTCPserial.printf("[-] iot2-debug: WARNING, tot_len(%u)!= recvedBytes(%u)\r\n", pkt_buf->tot_len, this->recvedBytes);
    }

    //-------------------------------------------------------------------------------------------//
    //                                     Allocate recvPBUF                                     //
    //-------------------------------------------------------------------------------------------//
    
    // Check if we need to copy tempPBUF before. If yes, then copy
    // it and put it at the beginning of recvPBUF
    if (this->tempPBUF != NULL){
        
        // Allocate memory only for the recvPBUF while considering tempPBUF 
        // without headers in RAM.
        this->recvPBUF = pbuf_alloc(PBUF_RAW, this->recvedBytes+
            this->tempPBUFSize+1, PBUF_RAM); 

        // Copy tempPBUF to the begining of recvPBUF and set offset
        memcpy((uint8_t*)this->recvPBUF->payload , this->tempPBUF->payload,
            this->tempPBUFSize);
        offset = this->tempPBUFSize;

        // Free tempPBUF
        this->tempPBUF = this->freePBUF(this->tempPBUF);

        this->recvedBytes += this->tempPBUFSize;    // Update recvedBytes
        this->recvOffset = 0;                       // Update recvOffset
    }

    else{
        // Allocate memory only for the receiver buffer without headers in RAM
        this->recvPBUF = pbuf_alloc(PBUF_RAW, this->recvedBytes+1, PBUF_RAM);
    }

    // Check recvPBUF has been allocated correctly
    if(this->recvPBUF == NULL){
        return ERR_MEM;
    }

    //-------------------------------------------------------------------------------------------//
    //                                    Copy to recvPBUF                                       //
    //-------------------------------------------------------------------------------------------//

    // Copy pbuf chain
    curr_pbuf = pkt_buf;
    while ( (curr_pbuf != NULL) && ((this->recvedBytes - offset) >= curr_pbuf->len) ){
        memcpy((uint8_t*)this->recvPBUF->payload + offset, curr_pbuf->payload, curr_pbuf->len);
        offset += curr_pbuf->len;
        curr_pbuf = curr_pbuf->next;
    }

    // NULL terminate recvPBUF
    ((char*)this->recvPBUF->payload)[this->recvedBytes] = '\0';
    //this->bmTCPserial.printf("[iot2-debug]: recvPBUF->payload: %s\r\n", this->recvPBUF->payload);
    return ERR_OK;
}


/// @brief This function is used to store an incomplete trainling part of a
///        of packet in a temporary pbuf. The temporary pbuf is copied to 
///        begining of recvPBUF at the next call of cpyRecvPBUF.
/// @param None
/// @retval ERR_OK if copied correctly, ERR_MEM otherwise.
err_t BareMetalTCP::storeTempPBUF(void){

    // Before storing into tempPBUF, check for the case of a trailing
    // NULL (i.e., only a NULL is remaining in recvPBUF). This results
    // when the server adds an additional terminating NULL. If such a case
    // occures, then we do not need to store any thing tempPBUF and we can
    // return immediatly.
    uint16_t traillingNullSizeCheck = this->recvedBytes - this->recvOffset;
    if (traillingNullSizeCheck == 1 && 
        ((char*)(this->recvPBUF->payload))[this->recvOffset+1]=='\0'){
        
        this->bmTCPserial.printf("[iot2-debug]: NULL-ERROR detected!\r\n");
        return ERR_OK;
    }
    this->tempPBUFSize = this->recvedBytes - this->recvOffset;

    // Allocate tempPBUF
    this->tempPBUF = pbuf_alloc(PBUF_RAW, this->tempPBUFSize+1, PBUF_RAM);
    if (this->tempPBUF == NULL){
        return ERR_MEM;
    }

    // Copy temporary buffer
    uint16_t pbufBytes = pbuf_copy_partial(this->recvPBUF, this->tempPBUF->payload, 
        this->tempPBUFSize, this->recvOffset);
    if (pbufBytes != this->tempPBUFSize){
        return ERR_MEM;
    }

    ((char*)this->tempPBUF->payload)[this->tempPBUFSize] = '\0';
    
    // setup tempPBUF values
    this->tempPBUF->tot_len = this->tempPBUFSize;
    this->tempPBUF->len = this->tempPBUFSize;

    return ERR_OK;
}



//-----------------------------------------------------------------------------------------------//
//                                      Callback Trampolines                                     //
//-----------------------------------------------------------------------------------------------//


// @brief Accept a connection
/// @param Pointet to baremetalTCPstruct, not used in this function
/// @param Pointer to new tcp_pcb connection
/// @param Error value, not used in this function
/// @retval Error value: ERR_OK if it succeeds.
static err_t trampoline_accept(void *arg, struct tcp_pcb *newpcb, err_t err){

    // Check before calling
    if(globalTrampolinePtr != NULL){
        return globalTrampolinePtr->callback_accept_connect(arg, newpcb, err);
    }
    return ERR_MEM;
}


/// @brief Receive data from a TCP connection
/// @param Pointet to baremetalTCPstruct
/// @param Pointer to tcp_pcb connection
/// @param Pointer to the received data struct
/// @param Error value associated with the pbuf struct
/// @retval Error value: ERR_OK if it succeeds.
static err_t trampoline_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err){

    // Check before calling
    if(globalTrampolinePtr != NULL){
        return globalTrampolinePtr->callback_recv(arg, tpcb, p, err);
    }

    return ERR_MEM;
}


/// @brief Send  remaining data or acknowledgment if all data has been delivered
/// @param Pointet to baremetalTCPstruct
/// @param Pointer to tcp_pcb connection
/// @param data length
/// @retval error state
static err_t trampoline_sent(void *arg, struct tcp_pcb *tpcb, u16_t len){

    // Check before calling
    if(globalTrampolinePtr != NULL){
        return globalTrampolinePtr->callback_sent(arg, tpcb, len);
    }

    return ERR_MEM;
}


/// @brief Function to catch errors. Implemented as a simple infinite loop
/// @param None
/// @retval None
static err_t trampoline_poll(void *arg, struct tcp_pcb *tpcb){

    // Check before calling
    if(globalTrampolinePtr != NULL){
        return globalTrampolinePtr->callback_poll(arg, tpcb);
    }

    return ERR_MEM;
}


/// @brief Function to catch errors. Implemented as a simple infinite loop
/// @param None
/// @retval None
static void trampoline_error(void *arg, err_t err){

    // Check before calling
    if(globalTrampolinePtr != NULL){
        globalTrampolinePtr->callback_error(arg, err);
    }
}

/// @brief This function is called in lwip_stack.c to indicate callback_connect 
///        can be called after the board has initialized.
static err_t trampoline_connect(void *arg, struct tcp_pcb *tpcb, err_t err){
    // Check before calling
    if(globalTrampolinePtr != NULL){
        // the callback functions of accept and connect map to the same function
        return globalTrampolinePtr->callback_accept_connect(arg,tpcb,err);
    }
    return ERR_MEM;
}


#endif // LWIP_TCP
