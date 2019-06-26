//=================================================================================================
//
// This file has the implementationfor the benchmark itself. The main file only calls the 
// benchmark main function (where the benchmark main function is named after each benchmark).
//
//=================================================================================================



//=========================================== INCLUDES ==========================================//

#include "mbed.h"
#include "smart_locker.h"
#include "FlashIAP.h"
//======================================== DEFINES & GLOBALS ====================================//


Serial pc(USBTX, USBRX, NULL, 9600);       // serial to debug

// display variables
IoT2_DisplayInterface display;                                            // display object
char bench_welcom_msg[] = "1) Drop package 2) Pickup package";            // display messages
char drop_or_pick_msg[] = "Enter apartment number and name of recipient";
char pickup_package_msg[] = "Enter pin";
char wrong_pin_msg[] = "Incorrect pin";
char correct_pin_msg[] = "Please pickup your package from the opened locker";


/// global to track dataset entries
uint8_t dataset_cntr = 0;

/// locker interface struct
typedef struct{
    char name[11];                           // name is formatted as "Person_###"   
    uint8_t apart_num;                       // recipient apartment number
    unsigned char pin_hash[SHA256_LENGTH+1] ;// hash of the locker's pin, +1 is for \0
    bool occupied;                           // flase indicates the locker is empty
}LockerStrct;

// an array of lockers to emulate the smart locker
LockerStrct smart_lockers[DATASET_SIZE];

// gpio represents the state of the locker (open=0/locked=1), only one
// is used for all lockers.
DigitalOut locker_gpio(LOCKER_LED);

// flag to alternate between correct and wrong pin entry
bool enter_correct_pin = true;

// pointer to the user pin (either bench_dataset or wrong_pins_dataset)
unsigned char *input_pin;

// holds the benchmark result in format <hash, hash_res, result, occupied result>
char bench_result[NUM_BENCH_RESULTS][BENCH_RESULT_SIZE];


//#sdio-------------------------------------------------------------------------
__attribute__((section(".SECURE_DATA"))) IOT2FILE log_file;
__attribute__((section(".SECURE_DATA"))) IOT2FILE* log_file_ptr;
char log_filename[] = "sl_log.txt";

//#sdio-------------------------------------------------------------------------



//========================================= FUNCTIONS ===========================================//


void benchmarkMain(void){
    

    //-------------------------------------------------------------------------------------------//
    //                                    Initialization                                         //
    //-------------------------------------------------------------------------------------------//
    
    EthernetInterface eth;
    TCPSocket socket;
    TCPServer tcpserver;
    SocketAddress sockaddr;
    char tcp_buff[128];
    char rtc_buff[34];
    ssize_t bytes = 0, bytes_written = 0;
    int int_bytes = 0;
    set_time(IoT2_RTC_SECONDS);  // Set RTC time
    time_t seconds;
    seconds = time(NULL);        // get current time from mbed RTC
    char *rtc_ptr;

    // initialize Ethernet/TCP
    iot2InitTCP(&eth, &socket, &tcpserver, &sockaddr);

    //#sdio
    iot2fs_init();
    iot2fs_fmount();

    // open logging file
    iot2CallSVC(IOT2_RESERVED_SVC,ENABLE_SECURE_DATA_SVC_NUM);// enable secure region
    log_file_ptr = &log_file;
    log_file_ptr = iot2fs_fopen(log_file_ptr, log_filename, "w+");
    iot2CallSVC(IOT2_RESERVED_SVC,DISABLE_SECURE_DATA_SVC_NUM);// disable secure region
    //#sdio

    // clear up buffers
    memset(tcp_buff, 0, sizeof(tcp_buff));
    memset(rtc_buff, 0, sizeof(rtc_buff));

    // initialize display with benchmark name
    display.config("SMART-LOCKER BENCHMARK");

    // enable soft irq
    iot2EnableSWI(IoT2_USART1_IRQ);
    //-------------------------------------------------------------------------------------------//
    //                                     Drop packages                                         //
    //-------------------------------------------------------------------------------------------//

    for (uint8_t i = 0; i < DATASET_SIZE; i++){
        // print welcom message on display
        displayMsg(display, (uint8_t*)bench_welcom_msg);

        // update dataset cntr
        dataset_cntr = i;

        // show message at display to choose either to drop or pickup packages
        displayMsg(display, (uint8_t*)drop_or_pick_msg);

        // emulate choice by putc()
        pc.putc('1');
        pc.putc('\n');
        // trigger interrupt to drop package
        iot2TriggerBenchSoftIRQ(dropPackageHandler, IoT2_USART1_IRQ);

        // change the locker's gpio to locked
        locker_gpio = 1;

        
        // fomart the rtc string, only at hour precision to simplify verification process
        seconds = time(NULL);
        strftime(rtc_buff,sizeof(rtc_buff), "DATE=%a/%d/%m/%Y,TIME=%H", localtime(&seconds));
        rtc_ptr = rtc_buff;

        // format logging msg, input pin has already been set up from dropPackageHandler
        int_bytes = snprintf(tcp_buff, sizeof(tcp_buff), 
            "REQ=DROP,%s,NAME=%s,APT=%d,LOCKER:%d,PIN=%s\n",rtc_ptr, 
            smart_lockers[dataset_cntr].name,smart_lockers[dataset_cntr].apart_num, 
            smart_lockers[dataset_cntr].occupied,input_pin);
        

        if (int_bytes < 0){
            iot2SerialDebugMsg("[-] ERROR: Logging message not formatted");
        }
        bytes = int_bytes;
        // write to log file
        //#sdio
        iot2CallSVC(IOT2_RESERVED_SVC,ENABLE_SECURE_DATA_SVC_NUM);// enable secure region
        bytes_written = iot2fs_fwrite(tcp_buff, 1, bytes,log_file_ptr);
        iot2CallSVC(IOT2_RESERVED_SVC,DISABLE_SECURE_DATA_SVC_NUM);// disable secure region


        // send the information to the server
        int_bytes = iot2send(&socket, tcp_buff, sizeof(tcp_buff));
        if( int_bytes < 0){
            iot2SerialDebugMsg("[SEND ERROR]: int_bytes < 0");
            iot2Error();
        }

        // clear up tcp_buff and rtc_buff
        memset(tcp_buff, 0, sizeof(tcp_buff));
        memset(rtc_buff, 0, sizeof(rtc_buff));

    }

    //-------------------------------------------------------------------------------------------//
    //                             Handle inquiries from server                                  //
    //-------------------------------------------------------------------------------------------//

    // here the server sends requests inquiring if packages for some resident are in this
    // locker or not. The smart-locker repondes with locker number if it exists.

    for (uint8_t i = 0; i < NUM_SERVER_INQUIRIES; i++){
        int_bytes = iot2recv(&socket, tcp_buff, sizeof(tcp_buff));
        
        if (int_bytes < 0){
            iot2SerialDebugMsg("[-] ERROR: recv returned < 0");
            iot2Error();
        }

        // handle sent cmd
        handleSmartLockerReq(tcp_buff, sizeof(tcp_buff));
        // send the response
        int_bytes = iot2send(&socket, tcp_buff, sizeof(tcp_buff));
        if( int_bytes < 0){
            iot2Error();
        }
        // clear the tcp_buff
        memset(tcp_buff, 0, sizeof(tcp_buff));
    }


    //-------------------------------------------------------------------------------------------//
    //                                    Pickup packages                                        //
    //-------------------------------------------------------------------------------------------//

    // reset dataset cntr 
    dataset_cntr = 0;

    for (uint8_t i = 0; i < NUM_BENCH_RESULTS; i++){
        bool pin_result = false;
        // print welcom message on display
        displayMsg(display, (uint8_t*)bench_welcom_msg);

        // setup pin entry flag
        enter_correct_pin = !enter_correct_pin;

        // show message at display to choose either to drop or pickup packages
        displayMsg(display, (uint8_t*)drop_or_pick_msg);
        // emulate choice by putc()
        pc.putc('2');
        pc.putc('\n'); 

        // display msg to enter pin
        displayMsg(display, (uint8_t*)pickup_package_msg);

        // trigger interrupt to pickup package
        iot2TriggerBenchSoftIRQ(pickupPackageHandler, IoT2_USART1_IRQ);

        // check the entered user pin
        pin_result = checkPickupPin(input_pin, i);


        // change the locker's gpio to locked
        locker_gpio = 1;

        // fomart the rtc string, only at hour precision to simplify verification process
        seconds = time(NULL);
        strftime(rtc_buff,sizeof(rtc_buff), "DATE=%a/%d/%m/%Y,TIME=%H", localtime(&seconds));
        rtc_ptr = rtc_buff;

        // format logging msg, input pin has already been set up from dropPackageHandler
        bytes = snprintf(tcp_buff, sizeof(tcp_buff), 
            "REQ=PICKUP,%s,NAME=%s,APT=%d,LOCKER:%d,PIN=%s\r\n",rtc_ptr, 
            smart_lockers[dataset_cntr].name,smart_lockers[dataset_cntr].apart_num, 
            smart_lockers[dataset_cntr].occupied,input_pin);
        
        int_bytes = bytes;
        if (int_bytes < 0){
            iot2SerialDebugMsg("[-] ERROR: Logging message not formatted");
        }

        // write to log file
        //#sdio
        iot2CallSVC(IOT2_RESERVED_SVC,ENABLE_SECURE_DATA_SVC_NUM);// enable secure region
        bytes_written = iot2fs_fwrite(tcp_buff, 1, bytes,log_file_ptr);
        iot2CallSVC(IOT2_RESERVED_SVC,DISABLE_SECURE_DATA_SVC_NUM);// disable secure region

        // send the information to the server
        int_bytes = iot2send(&socket, tcp_buff, sizeof(tcp_buff));
        if( int_bytes < 0){
            iot2SerialDebugMsg("[SEND ERROR]: int_bytes < 0");
            iot2Error();
        }

        // display message according to result (i.e., correct/wrong pin)
        if(pin_result){
            displayMsg(display, (uint8_t*)correct_pin_msg);
            dataset_cntr++;     // increment dataset counter
        }
        else{
            displayMsg(display, (uint8_t*)wrong_pin_msg);
        }

        // clear up tcp_buff and rtc_buff
        memset(tcp_buff, 0, sizeof(tcp_buff));
        memset(rtc_buff, 0, sizeof(rtc_buff));

    }


    //#sdio
    iot2CallSVC(IOT2_RESERVED_SVC,ENABLE_SECURE_DATA_SVC_NUM);// enable secure region
    iot2fs_fclose(log_file_ptr);
    iot2CallSVC(IOT2_RESERVED_SVC,DISABLE_SECURE_DATA_SVC_NUM);// disable secure region
    iot2fs_deinit();



    // send end msg to remote
    snprintf(tcp_buff, sizeof(tcp_buff), IoT2_TCP_DRIVER_END_MSG);
    // send the information to the server
    int_bytes = iot2send(&socket, tcp_buff, sizeof(tcp_buff));
    if( int_bytes < 0){
        iot2SerialDebugMsg("[SEND ERROR]: int_bytes < 0");
        iot2Error();
    }

    socket.close();
    eth.disconnect();
}


void dropPackageHandler(void){

    char num_buff[3];
    itoa(dataset_cntr, num_buff,10);


    // update name, apartment number, and state
    smart_lockers[dataset_cntr].name[0] = 'P';
    smart_lockers[dataset_cntr].name[1] = 'e';
    smart_lockers[dataset_cntr].name[2] = 'r';
    smart_lockers[dataset_cntr].name[3] = 's';
    smart_lockers[dataset_cntr].name[4] = 'o';
    smart_lockers[dataset_cntr].name[5] = 'n';
    smart_lockers[dataset_cntr].name[6] = '_';
    smart_lockers[dataset_cntr].name[7] = '0';
    
    if (strlen(num_buff) > 1){
        smart_lockers[dataset_cntr].name[8] = num_buff[0];
        smart_lockers[dataset_cntr].name[9] = num_buff[1];
    }
    else{
        smart_lockers[dataset_cntr].name[8] = '0';
        smart_lockers[dataset_cntr].name[9] = num_buff[0];
    }
    smart_lockers[dataset_cntr].name[10] = '\0';

    smart_lockers[dataset_cntr].apart_num = dataset_cntr;
    smart_lockers[dataset_cntr].occupied = true;
 
    // execute putc to emulate actual execution, the output message is just a 
    // place holder. Emulating name is enough here as the apartment
    // number is appended within the name at the end
    for (uint8_t i = 0; i < strlen(smart_lockers[dataset_cntr].name); i++){
        //pc.putc(smart_lockers[dataset_cntr].name[i]);
    }
    // print a new line
    //pc.putc('\n');

    // change the state of locker's gpio to open
    locker_gpio = 0;

    // get the input_pin from the dataset
    input_pin = (unsigned char *) bench_dataset[dataset_cntr];
    // hash the pin
    mbedtls_sha256(input_pin, PIN_LENGTH, smart_lockers[dataset_cntr].pin_hash, 0);

}


void pickupPackageHandler(void){

    // emulate pin entry. this will be correct or not depending on the flag set
    // before the interrupt
    if(enter_correct_pin){
        // get the input_pin from the dataset
        input_pin = (unsigned char *) bench_dataset[dataset_cntr];

    }
    // get the wrong_pin
    else{
        input_pin = (unsigned char *)worng_pins_dataset[dataset_cntr];
 
    }


}


bool checkPickupPin(unsigned char* user_pin, uint8_t res_cntr){

    // variable to storre the hash result of the pin
    unsigned char hash_res[SHA256_LENGTH+1];
    // stores the result of the attempt.
    bool dataset_result = false;

    // hash the user input
    mbedtls_sha256(user_pin, PIN_LENGTH, hash_res, 0);
    // null terminate hash_res
    hash_res[SHA256_LENGTH] = '\0';

    // lookup the result
    for (uint8_t i = 0; i < DATASET_SIZE; i++){
        // check if hash matches the locker
        if (isPinHash(hash_res, i) ){
            // change state of the locker
            smart_lockers[i].occupied = false;
            // open locker
            locker_gpio = 0;
            dataset_result = true;
            break;
        }
    }

    // store the result of current iteration
    storeBenchResult(dataset_result,res_cntr);

    return dataset_result;
}

bool isPinHash(unsigned char *entered_pin, uint8_t locker_idx){
    
    for (uint8_t i = 0; i < SHA256_LENGTH; i++){
        if (entered_pin[i] != smart_lockers[locker_idx].pin_hash[i]){
            return false;
        }
    }

    return true;
}


void storeBenchResult(bool dataset_result, uint8_t res_cntr){

    // store the result according to the format
    for (uint8_t i = 0; i < BENCH_RESULT_SIZE-2; i++){
        bench_result[res_cntr][i] = result_prefix[i];
    }
    // add the value of dataset_result and a newline
    bench_result[res_cntr][BENCH_RESULT_SIZE-2] = dataset_result ? '1' : '0';
    bench_result[res_cntr][BENCH_RESULT_SIZE-1] = '\n';
}


void displayMsg(IoT2_DisplayInterface &display, uint8_t* msg){
    
    // clear the display before the new message
    display.clear(COLOR_WHITE);
    // display the message
    display.displayStrAt(DISPLAY_XPOS, DISPLAY_YPOS, msg, DISPLAY_MODE);

}


void handleSmartLockerReq(char* req, size_t req_size){

    char cmd[] = CONTAIN_PACKAGE_CMD;
    // check the first arg of the req
    for(uint8_t i = 0; i < strlen(cmd); i++){
        if (cmd[i] != req[i]){
            // request is invalid
            memset(req, 0, req_size);
            strncpy(req, INVALID_REQ_RESPONSE, strlen(INVALID_REQ_RESPONSE));
            return;
        }
    }
    const char *resident_name = &(req[strlen(cmd)]);

    // lookup resident and send response
    for (uint8_t i = 0; i < DATASET_SIZE; i++){
        // check if the name length is the same before proceeding
        if (strlen(smart_lockers[i].name) == strlen(resident_name)){
            // compare smart locker name and request name
            if(strncmp(smart_lockers[i].name, resident_name, strlen(resident_name))==0){
                // request is valid, return locker number
                char num_buff [3] = {0};
                itoa(i, num_buff, 10);
                memset(req, 0, req_size);
                req[0] = 'L';
                req[1] = 'o';
                req[2] = 'c';
                req[3] = 'k';
                req[4] = 'e';
                req[5] = 'r';
                req[6] = '_';
                req[7] = '0';

                if (strlen(num_buff) > 1){
                    req[8] = num_buff[0];
                    req[9] = num_buff[1];
                }
                else{
                    req[8] = '0';
                    req[9] = num_buff[0];
                }
                req[10] = '\n';
                return;
            }
        }
    }

    // no package for the given resident is available
    memset(req, 0, req_size);
    strncpy(req, NO_PACKAGE_AVAILABLE, strlen(NO_PACKAGE_AVAILABLE));
    return;
}