//=================================================================================================
//
// This file has the implementationfor the benchmark itself. The main file only calls the 
// benchmark main function (where the benchmark main function is named after each benchmark).
//
//=================================================================================================



//=========================================== INCLUDES ==========================================//

#include "mbed.h"
#include "smart_thermostat.h"

//======================================== DEFINES & GLOBALS ====================================//


Serial pc(USBTX, USBRX, NULL, 9600);       // serial to debug

// display variables
IoT2_DisplayInterface display;             // display object

/// global to track dataset entries
uint8_t dataset_cntr = 0;

/// locker interface struct
typedef struct{
    float hw_adc_val;                      // the actual value read from adc hardware   
    float dataset_adc_val;                 // vlaue used from the adc dataset
    float user_val;                        // current value set by the user
    bool hvac_state;                       // true: on, false: off
}ThermostatStruct;

// an array of lockers to emulate the smart locker
ThermostatStruct st_config ={
    .hw_adc_val = 0.0,
    .dataset_adc_val = 0,
    .user_val = 0.0,
    .hvac_state = false
};

// log file variabls to record actual values from ADC
IOT2FILE log_file;
IOT2FILE* log_file_ptr;
char log_filename[] = "st_log.txt";

// Use the ADC to read the temperature
AnalogIn   adc_in(ADC_PIN);
DigitalOut debug_led(IoT2_GENERAL_LED); // [iot2-debug]




//========================================= FUNCTIONS ===========================================//


void benchmarkMain(void){
    

    //-------------------------------------------------------------------------------------------//
    //                                    Initialization                                         //
    //-------------------------------------------------------------------------------------------//
    
    EthernetInterface eth;
    TCPSocket socket;
    TCPServer tcpserver;
    SocketAddress sockaddr;
    char tcp_buff[64];
    char temp_buff[64];
    float temp = 0.0;
    int int_bytes = 0;
    int disp_temp = 0;

    // initialize Ethernet/TCP
    iot2InitTCP(&eth, &socket, &tcpserver, &sockaddr);

    //file system
    iot2fs_init();
    iot2fs_fmount();

    // timer setup
    //st_ticker.attach(&readAdcHandler, temp_sense_wait);

    // open logging file
    log_file_ptr = &log_file;
    log_file_ptr = iot2fs_fopen(log_file_ptr, log_filename, "w+");
    //#sdio

    // clear up buffers
    memset(tcp_buff, 0, sizeof(tcp_buff));

    // initialize display with benchmark name
    display.config("SMART-THERMOSTAT BENCHMARK");

    // enable soft irq
    iot2EnableSWI(IoT2_USART1_IRQ);
    //-------------------------------------------------------------------------------------------//
    //                                     RECV setup                                            //
    //-------------------------------------------------------------------------------------------//
    

    //-------------------------------------------------------------------------------------------//
    //                          Handle input from server/device                                  //
    //-------------------------------------------------------------------------------------------//

   
    for (uint8_t i = 0; i < NUM_REMOTE_CMDS; i++){

        int_bytes = iot2recv(&socket, tcp_buff, sizeof(tcp_buff));
        
        if (int_bytes < 0){
            //pc.printf("err = %d\r\n", int_bytes);
            //pc.printf("tcp_buff= %s\r\n", tcp_buff);
            iot2SerialDebugMsg("[-] ERROR: recv returned < 0");
            iot2Error();
        }

        //pc.printf("--------------------------------\r\n");
        //pc.printf("[%d] recv: %s\r\n", tcp_buff);
        handleCmd(tcp_buff, sizeof(tcp_buff));
        //pc.printf("[%d] send: %s\r\n", tcp_buff);
        //pc.printf("--------------------------------\r\n");
        // send resoponse
        int_bytes = iot2send(&socket, tcp_buff, sizeof(tcp_buff));
        if( int_bytes < 0){
            iot2Error();
        }

        memset(tcp_buff, 0, sizeof(tcp_buff));

    }

    for (uint8_t i = 0; i < DATASET_SIZE; i++){
        
        // read the actual temperature
        readAdcHandler();
        //Set the new output.
        temp = (( (3.3 * st_config.hw_adc_val)- 0.76)/2.5) + 25;//24.695
        // log the adc value
        logHwAdc();

        st_config.dataset_adc_val = thermostat_dataset[i];
        // trigger interrupt to set the temperature package
        iot2TriggerBenchSoftIRQ(readUserTemp, IoT2_USART1_IRQ);
        
        pc.printf("User entered temperature = %f\r\n", thermostat_dataset[i]);
            
        disp_temp = (int) (st_config.user_val);
        itoa(disp_temp, temp_buff,10);
        
        int_bytes = snprintf(tcp_buff, sizeof(tcp_buff), 
            "Temperature: %s\n",temp_buff);
        
        if (int_bytes < 0){
            iot2SerialDebugMsg("[-] ERROR: Logging message not formatted");
        }
        displayMsg(display, (uint8_t*)tcp_buff);
        


    }

    iot2fs_fclose(log_file_ptr);
    pc.printf("[+] closed log file\r\n");



    //--------------------------------------------------------------------------
    // Benchmark end: Every benchmark should end with this code
    //--------------------------------------------------------------------------
    
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


void displayMsg(IoT2_DisplayInterface &display, uint8_t* msg){
    
    // clear the display before the new message
    display.clear(COLOR_WHITE);
    // display the message
    display.displayStrAt(DISPLAY_XPOS, DISPLAY_YPOS, msg, DISPLAY_MODE);

}

// timer interrupt to read adc value
void readAdcHandler(void){
    st_config.hw_adc_val = adc_in.read();

}


/// writes the current value read from the ADC peripheral to the log file
void logHwAdc(void){
    ssize_t bytes = 0;
    char float_str [16] = {0};
    int decimal_val = (int) st_config.hw_adc_val;
    int after_point_val = (int) (1000*(st_config.hw_adc_val - decimal_val));
    snprintf(float_str, sizeof(float_str), "HW_ADC:%2d.%3d\n" ,
        decimal_val, after_point_val);

    bytes = iot2fs_fwrite(&float_str, 1, strlen(float_str), log_file_ptr);
    if (bytes < 0){
        iot2SerialDebugMsg("[-] ERROR: Cannot write adc hw value to log file");
        iot2Error();
    }

}

void handleCmd(char* cmd, size_t cmd_size){

    size_t cmd_length = 0;

    // check if it is a cmd for setting on/off period
    for (uint8_t i = 0; i < cmd_size; i++){
        if (cmd[i] == ' '){
            cmd_length = i;
            break;
        }
    }
    // there are 2 cmds currently, so check against them only
    // any other cmd is invalid
    if (cmd_length != 0){
        // verify it is the same length as SET_TEMP
        if ((cmd_length+1) == strlen(SET_TEMP_CMD)){
            if (strncmp(cmd, SET_TEMP_CMD, 
                strlen(SET_TEMP_CMD)) == 0){

                // this is a valid cmd, get the input temperature, used <+1>
                // since cmd_length points to ' ', and adding one points to
                // the value of the temperature
                if(getInputTemp(cmd, cmd_length+1, strlen(cmd)) != TEMP_ERROR){
                    // return ack response for setting the desired temperature
                    setValidResponse(cmd, cmd_size, SET_TEMP_RESPONSE);
                }
                else {
                    // invalid temperature sent, return an error
                    setInvalidCmdResponse(cmd, cmd_size);
                }
            }
            // there is an error
            else{
                setInvalidCmdResponse(cmd, cmd_size);
            }
        }
        // there is an error
        else{
            setInvalidCmdResponse(cmd, cmd_size);
        }
    }

    // if cmd is not a SET_TEMP cmd
    else{
        if(strncmp(cmd, GET_TEMP_CMD, strlen(GET_TEMP_CMD)) == 0 &&
            strlen(cmd) == strlen(GET_TEMP_CMD)){
            setValidResponse(cmd, cmd_size, GET_TEMP_RESPONSE);
        }
        // invlid cmd
        else{
            setInvalidCmdResponse(cmd, cmd_size);
        }
    }
    
}

/// @brief function to comput the input temperature
/// @param cmd pointer to command string
/// @param temp_idx index of temperature start in cmd
/// @param cmd_len length of the cmd command
int getInputTemp(char* cmd, uint8_t temp_idx, size_t cmd_len){
    // check that the length of the temprature is correct
    int temp_length = (int) (cmd_len - temp_idx);
    int temp_val = 0;
    char temp_asci[3] = {0};
    // all acceptable temperature values are double digits
    if (temp_length != 2){
        temp_val = TEMP_ERROR; // error
    }

    // fill in the temp_asci buff
    temp_asci[0] = cmd[temp_idx];
    temp_asci[1] = cmd[temp_idx+1];

    // check the values of temp_asci
    if (temp_asci[0] < '1' || temp_asci[1] > '9' ||
        temp_asci[0] < '0' || temp_asci[1] > '9'){
        // error in the entered values, return an error
        return TEMP_ERROR;
    }

    // convert to integer
    temp_val = ((temp_asci[0] - '0') * 10) + (temp_asci[1] - '0');

    // finally check the enterd temperature is in the allowed range
    if (temp_val < MIN_TEMP || temp_val > MAX_TEMP){
        temp_val = TEMP_ERROR;
    }

    if (temp_val != TEMP_ERROR){
        st_config.user_val =  (float) temp_val;
    }

    return temp_val;

}


void setInvalidCmdResponse(char* buff, size_t buff_size){

    char invalid_req_response[] = INVALID_CMD_RESPONSE;
    // clear the buffer
    memset(buff, 0, buff_size);
    // set the buff to invalid request response
    for (uint8_t i = 0; i < strlen(INVALID_CMD_RESPONSE); i++){
        buff[i] = invalid_req_response[i];
    }
}

void setValidResponse(char* buff, size_t buff_size, const char* response_buff){

    size_t response_len = strlen(response_buff);
    char ascii_buff[3] = {0};
    
    // reset memory
    memset(buff, 0, buff_size);
    // check response buff
    switch(response_len){
        case SET_TEMP_RESP_LEN:
            if (strncmp(response_buff, SET_TEMP_RESPONSE,
                SET_TEMP_RESP_LEN) == 0){
                // copy the first part of the cmd
                cpBuff(buff, buff_size, response_buff, SET_TEMP_RESP_LEN);
                getAsciiTemp(ascii_buff);
                buff[SET_TEMP_RESP_LEN] = ascii_buff[0];
                buff[SET_TEMP_RESP_LEN+1] = ascii_buff[1];

            }else{
                setInvalidCmdResponse(buff, buff_size);
            }
            break;
        case GET_TEMP_RESP_LEN:
            if (strncmp(response_buff, GET_TEMP_RESPONSE,
                GET_TEMP_RESP_LEN) == 0){
                // copy the first part of the cmd
                cpBuff(buff, buff_size, response_buff, GET_TEMP_RESP_LEN);
                getAsciiTemp(ascii_buff);
                buff[GET_TEMP_RESP_LEN] = ascii_buff[0];
                buff[GET_TEMP_RESP_LEN+1] = ascii_buff[1];

            }else{
                setInvalidCmdResponse(buff, buff_size);
            }
            break;
        default:
            setInvalidCmdResponse(buff, buff_size);
    }
}

void cpBuff(char* cmd, size_t cmd_size, const char* buff, size_t buff_len){
    
    // check the length of the buffer
    if (buff_len > cmd_size){
        iot2Error();
        iot2SerialDebugMsg("[-] ERROR: buff_len > cmd_size in cpBuff");
    }

    for (uint8_t i = 0; i < buff_len; i++){
        cmd[i] = buff[i];
    }
}

// this fuction assumes ascii_buff length is 3 ( 2 chars + '\0')
void getAsciiTemp(char* ascii_buff){
    // the temperature is not supposed to be larger than a 2-digit number
    unsigned char first_digit = (int) (st_config.user_val/10);
    unsigned char second_digit = (int) (st_config.user_val - (first_digit* 10));
    unsigned char ascii_offset = 48;
    ascii_buff[0] = first_digit + ascii_offset;
    ascii_buff[1] = second_digit + ascii_offset;


}


void readUserTemp(void){
    st_config.user_val = st_config.dataset_adc_val;
}

