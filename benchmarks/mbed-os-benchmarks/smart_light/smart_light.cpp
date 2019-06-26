//=================================================================================================
//
// This file has the implementationfor the benchmark itself. The main file only calls the 
// benchmark main function (where the benchmark main function is named after each benchmark).
//
//=================================================================================================



//=========================================== INCLUDES ==========================================//

#include "mbed.h"
#include "smart_light.h"

//======================================== DEFINES & GLOBALS ====================================//


Serial pc(USBTX, USBRX, NULL, 9600);

// Ticker object to be used for periodic interrupt. The low power ticker is used
// here to allow entring deep sleep state if possible
LowPowerTicker lp_ticker;

// LED as a smart light
DigitalOut smart_light(IoT2_GENERAL_LED, LED_OFF);

const float led_max_wait = 0.333;
const float server_msg_wait = 0.67;

/// This is to track light state
enum SMART_LIGHT_MODE{
    NORMAL_LIGHT = 0,
    KEEP_LIGHT_ON = 1,
};


/// This struct holds the configuration of the smart light
/// state holds the configured state
/// update_state is the value used by the motion detector and timer interrupt
/// to turn on/off the light for power saving. The timer interrupt checks if
/// the value is true/false to resove whether a motion has occured since the 
/// last check. Note that the motion detector will switch the value to true to
/// signal that a motion has happened.
typedef struct{
    enum SMART_LIGHT_MODE mode;
    bool update_state;
    bool has_turned_off;
    char turn_on_period_start[LIGHT_CONFIG_LENGTH];
    char turn_on_period_end[LIGHT_CONFIG_LENGTH];
    int32_t turn_on_start_time;
    int32_t turn_on_end_time;
    char turn_off_period_start[LIGHT_CONFIG_LENGTH];
    char turn_off_period_end[LIGHT_CONFIG_LENGTH];
    int32_t turn_off_start_time;
    int32_t turn_off_end_time;
}LightConfigStrct;


LightConfigStrct smart_light_config = {
    .mode = NORMAL_LIGHT,
    .update_state = true,
    .has_turned_off = false,
    .turn_on_period_start = {0},
    .turn_on_period_end = {0},
    .turn_on_start_time = 0,
    .turn_on_end_time = 0,
    .turn_off_period_start = {0},
    .turn_off_period_end = {0},
    .turn_off_start_time = 0,
    .turn_off_end_time = 0
};


//========================================= FUNCTIONS ===========================================//


void benchmarkMain(void){
    

    //-------------------------------------------------------------------------------------------//
    //                                    Initialization                                         //
    //-------------------------------------------------------------------------------------------//

    // networking variables
    EthernetInterface eth;
    TCPSocket socket;
    TCPServer tcpserver;
    SocketAddress sockaddr;
    // other variables 
    char tcp_buff[TCP_BUFF_SIZE];
    char rtc_buff[34];
    int int_bytes = 0;
    
    // clear up buffers
    memset(tcp_buff, 0, sizeof(tcp_buff));
    memset(rtc_buff, 0, sizeof(rtc_buff));
   
    // initialize Ethernet/TCP
    iot2InitTCP(&eth, &socket, &tcpserver, &sockaddr);
    
    // set RTC
    set_time(IoT2_RTC_SECONDS);  // Set RTC time
    time_t seconds;
    seconds = time(NULL);        // get current time from mbed RTC
    
    // attach the timer interrupt handler
    lp_ticker.attach(&checkLightHandler, led_max_wait);

    // turn on the light
    smart_light.write(LED_ON);

    //-------------------------------------------------------------------------------------------//
    //                                    RECV light setup                                       //
    //-------------------------------------------------------------------------------------------//
    
    // receive 3 config cmds, one is an invalid cmd
    for (uint8_t i = 0; i < NUM_RECV_CONFIG_CMDS; i++){
        // recv cmd
        int_bytes = iot2recv(&socket, tcp_buff, sizeof(tcp_buff));
        
        if (int_bytes < 0){
            iot2SerialDebugMsg("[-] ERROR: recv returned < 0");
            iot2Error();
        }

        // handle the sent cmd
        handleSmartLightCmd(tcp_buff, sizeof(tcp_buff));

        // send resoponse
        int_bytes = iot2send(&socket, tcp_buff, sizeof(tcp_buff));
        if( int_bytes < 0){
            iot2Error();
        }

        // clear buff
        memset(tcp_buff, 0, sizeof(tcp_buff));
    }

    //-------------------------------------------------------------------------------------------//
    //                             Handle inquiries from server                                  //
    //-------------------------------------------------------------------------------------------//

    for (uint8_t i = 0; i < DATASET_SIZE; i++){

        seconds = time(NULL);
        strftime(rtc_buff,sizeof(rtc_buff), "TIME=%H:%M:%S", localtime(&seconds));
        // check if we need to apply configs
        applyConfigs(&(rtc_buff[5]));

        // add artificial wait
        wait(server_msg_wait);

        // recv cmd
        int_bytes = iot2recv(&socket, tcp_buff, sizeof(tcp_buff));
        
        if (int_bytes < 0){
            //pc.printf("err = %d\r\n", int_bytes);
            //pc.printf("tcp_buff= %s\r\n", tcp_buff);
            iot2SerialDebugMsg("[-] ERROR: recv returned < 0");
            iot2Error();
        }

        // handle the sent cmd
        handleSmartLightCmd(tcp_buff, sizeof(tcp_buff));

        // send resoponse
        int_bytes = iot2send(&socket, tcp_buff, sizeof(tcp_buff));
        if( int_bytes < 0){
            iot2Error();
        }

        // the motion detector is fired up once every 4 iterations.
        if ((i % 4) == 3){
            iot2TriggerBenchSoftIRQ(motionDetectionHandler, IOT2_MOTION_DETECTOR_IRQ);
            iot2SerialDebugMsg("---STIR triggered!---");
        }
        
        // clear buffs
        memset(rtc_buff, 0, sizeof(rtc_buff));
        memset(tcp_buff, 0, sizeof(tcp_buff));

    }

    // turn led on at the end
    smart_light = LED_ON;


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


void checkLightHandler(void){

    // Only check timer if the led is ON
    if (smart_light.read() == LED_ON && smart_light_config.mode != KEEP_LIGHT_ON){
        // check if timer reached a limit without a change in status/motion detected
        if (smart_light_config.update_state == false){
            smart_light = LED_OFF;
        }
    }

    // reset update state for the next call
    smart_light_config.update_state = false;

}

/// This handler is executed if a motion is detected.
void motionDetectionHandler(void){
    
    smart_light.write(LED_ON);
    smart_light_config.update_state = true;
}


void handleSmartLightCmd(char* cmd, size_t cmd_size){

    size_t cmd_length = 0, delimiter_idx = 0;
    char comma_delimiter = ',';

    // check if it is a cmd for setting on/off period
    for (uint8_t i = 0; i < cmd_size; i++){
        if (cmd[i] == ' '){
            cmd_length = i;
            break;
        }
    }

    //--------------------------------------------------------------------------
    // the cmd is a period config command if cmd_length != 0
    //--------------------------------------------------------------------------
    if (cmd_length != 0){
        // there are 2 possibilities for config cmds, both include
        // a comma as delimiter. If this is not found then there is
        // an error
        for (uint8_t i = cmd_length; i < cmd_size; i++){
            if(cmd[i] == comma_delimiter){
                delimiter_idx = i;
                break;
            }
        }

        // verify the format of the cmd is correct
        if (( (delimiter_idx - cmd_length) != LIGHT_CONFIG_LENGTH) ||
            ( (strlen(cmd) - delimiter_idx) != LIGHT_CONFIG_LENGTH))
        {
            setInvalidReqResponse(cmd, cmd_size);
            return;
        }

        // verify that correct time formats have been sent
        if (!verifyConfigTimeFormat(cmd, cmd_length+1)){
            setInvalidReqResponse(cmd, cmd_size);
            return;
        }

        // if it is SET_LIGHT_ON_PERIOD_CMD
        if (strlen(SET_LIGHT_ON_PERIOD_CMD) == (cmd_length+1)){
            if (strncmp(cmd, SET_LIGHT_ON_PERIOD_CMD, strlen(SET_LIGHT_ON_PERIOD_CMD)) == 0){
                // cmd is accurate, set the required configuration
                setPeriodConfig(cmd, smart_light_config.turn_on_period_start,
                                smart_light_config.turn_on_period_end, cmd_length+1);

                 // convert to time_t values in configs
                smart_light_config.turn_on_start_time = convTimeStrToSeconds(
                                                        smart_light_config.turn_on_period_start
                                                                             );
                smart_light_config.turn_on_end_time = convTimeStrToSeconds(
                                                        smart_light_config.turn_on_period_end
                                                                            );

                char exec_cmd_buff[] = SET_LIGHT_ON_PERIOD_CMD;
                // set response header
                setCmdReponseHeader(cmd, cmd_size);
                // add the executed cmd
                cpyCmdBuff(cmd, exec_cmd_buff, strlen(cmd), strlen(exec_cmd_buff));

            }
            // error, unknown cmds
            else{
                setInvalidReqResponse(cmd, cmd_size);
                return;
            }
        }

        // if it configure light off period
        else if(strlen(SET_LIGHT_OFF_PERIOD_CMD) == (cmd_length+1)) {
            if (strncmp(cmd, SET_LIGHT_OFF_PERIOD_CMD, strlen(SET_LIGHT_OFF_PERIOD_CMD)) == 0){
                // cmd is accurate, set the required configuration
                setPeriodConfig(cmd, smart_light_config.turn_off_period_start,
                                smart_light_config.turn_off_period_end, cmd_length+1);


                // convert to time_t values in configs
                smart_light_config.turn_off_start_time = convTimeStrToSeconds(
                                                        smart_light_config.turn_off_period_start
                                                                             );
                smart_light_config.turn_off_end_time = convTimeStrToSeconds(
                                                        smart_light_config.turn_off_period_end
                                                                            );

                char exec_cmd_buff[] = SET_LIGHT_OFF_PERIOD_CMD;
                // set response header
                setCmdReponseHeader(cmd, cmd_size);
                // add the executed cmd
                cpyCmdBuff(cmd, exec_cmd_buff, strlen(cmd), strlen(exec_cmd_buff));


            }
            // error, unknown cmds
            else{
                setInvalidReqResponse(cmd, cmd_size);
                return;
            }
        }
    }

    //--------------------------------------------------------------------------
    // the cmd is NOT period config command
    //--------------------------------------------------------------------------

    else{
        handleNonConfigCmd(cmd, cmd_size);
    }

}


bool verifyConfigTimeFormat(char *cmd, uint8_t periods_config_idx){

    // this variable is used to indicate which condition to verify
    // in the period configuration. By shifting the value of the loop
    // iterator and getting the remainder, we can check if the corrent
    // value should be number of <:> easily
    uint8_t idx_remainder = 0, shifted_idx = 0;
    for(uint8_t i = 0; i < (2*(LIGHT_CONFIG_LENGTH-1))+1; i++){

        shifted_idx = i + periods_config_idx;
        idx_remainder = i % 3;

        // if we are at the delimiter idx
        if (i == 8){
             if(cmd[shifted_idx] != ','){
                return false;
             }
        }
        else{
            if (idx_remainder == 2){
                if (cmd[shifted_idx] != ':'){
                    return false;
                }
            }
            else{
                if (cmd[shifted_idx] < '0' || cmd[shifted_idx] > '9'){
                    return false;
                }
            }
        }
    }

    return true;
}


// uinfied function to set periods configuration (whether on/off). Note that
// it is assumed the input is validated apriori.
void setPeriodConfig(char *cmd, char *start, char *end, uint8_t periods_config_idx){
    // set the start period
    for (uint8_t i = 0; i < LIGHT_CONFIG_LENGTH-1; i++){
        start[i] = cmd[i+periods_config_idx];
    }

    // set the end
    for (uint8_t i = 0; i < LIGHT_CONFIG_LENGTH-1; i++){
        end[i] = cmd[i+periods_config_idx+LIGHT_CONFIG_LENGTH];
    }

}


void setInvalidReqResponse(char *buff, size_t buff_size){

    char invalid_req_response[] = INVALID_REQ_RESPONSE;
    // clear the buffer
    memset(buff, 0, buff_size);
    // set the buff to invalid request response
    for (uint8_t i = 0; i < strlen(INVALID_REQ_RESPONSE); i++){
        buff[i] = invalid_req_response[i];
    }
}


void setCmdReponseHeader(char *buff, size_t buff_size){
    char response_header[] = CMD_RESPONSE_HEADER;
    // clear the buffer
    memset(buff, 0, buff_size);
    // set the header
    for (uint8_t i = 0; i < strlen(CMD_RESPONSE_HEADER); i++){
        buff[i] = response_header[i];
    }
}


void handleNonConfigCmd(char *buff, size_t buff_size){

    // the length of buff should be equal to either TURN_LIGHT_ON_CMD, 
    // TURN_LIGHT_OFF_CMD, or KEEP_LIGHT_ON_CMD. Two of these are the same

    if (strlen(buff) == strlen(TURN_LIGHT_ON_CMD)){
        // the cmd should be either TURN_LIGHT_ON_CMD or KEEP_LIGHT_ON_CMD
        if (strncmp(buff, TURN_LIGHT_ON_CMD, strlen(TURN_LIGHT_ON_CMD)) == 0){
            
            smart_light.write(LED_ON);
            smart_light_config.mode = NORMAL_LIGHT;
            smart_light_config.update_state = true;
            // set reponse header
            setCmdReponseHeader(buff, buff_size);
            // add the cmd executed
            char turn_light_on_buff[] = TURN_LIGHT_ON_CMD;
            cpyCmdBuff(buff, turn_light_on_buff, strlen(buff), strlen(turn_light_on_buff));
        }

        else if(strncmp(buff, KEEP_LIGHT_ON_CMD, strlen(KEEP_LIGHT_ON_CMD)) == 0){
            smart_light_config.mode = KEEP_LIGHT_ON;
            smart_light.write(LED_ON);
            smart_light_config.update_state = true;
            // set response header
            setCmdReponseHeader(buff, buff_size);
            // add the executed cmd
            char keep_light_on_buff[] = KEEP_LIGHT_ON_CMD;
            cpyCmdBuff(buff, keep_light_on_buff, strlen(buff), strlen(keep_light_on_buff));
        }

        // error, invalid cmd
        else{
            setInvalidReqResponse(buff, buff_size);
        }

    }

    else if (strlen(buff) == strlen(TURN_LIGHT_OFF_CMD)){
        if (strncmp(buff, TURN_LIGHT_OFF_CMD, strlen(TURN_LIGHT_OFF_CMD)) == 0){
            
            smart_light.write(LED_OFF);
            smart_light_config.update_state = true;
            // set reponse header
            setCmdReponseHeader(buff, buff_size);
            // add the cmd executed
            char turn_light_off_buff[] = TURN_LIGHT_OFF_CMD;
            cpyCmdBuff(buff, turn_light_off_buff, strlen(buff), strlen(turn_light_off_buff));
        }

        // error, invalid cmd
        else{
            setInvalidReqResponse(buff, buff_size);
        }

    }

    // error, invlaid cmd
    else{
        setInvalidReqResponse(buff, buff_size);
    }

}

// [iot2-debug]: modify again to check for the size of the buffers
void cpyCmdBuff(char *buff, char *cmd_cpy, uint8_t buff_idx, uint8_t cmd_cpy_size){

    for(uint8_t i = 0; i < cmd_cpy_size; i++){
        buff[i+buff_idx] = cmd_cpy[i];
    }
}


void applyConfigs(char *time_buff){

    // if light is on and stat == KEEP_LIGHT_ON then there is no need to check
    // LIGHT_ON_PERIOD for this call
    if (smart_light_config.mode != KEEP_LIGHT_ON || smart_light.read() != LED_ON){
        if (isLightOnPeriod(time_buff)){
            //pc.printf("IT IS LIGHTS ON TIME\r\n");
            smart_light_config.mode = KEEP_LIGHT_ON;
            smart_light.write(LED_ON);
        }
    }

    // if it is time to turn lights off
    if (isLightOffTime(time_buff)){
        //pc.printf("IT IS LIGHTS OFF TIME\r\n");
        smart_light_config.mode = NORMAL_LIGHT;
        smart_light_config.has_turned_off = true;
        smart_light.write(LED_OFF);
    }
}


bool isLightOnPeriod(char *time_buff){
    // get the time in seconds
    int32_t seconds = convTimeStrToSeconds(time_buff);
    bool after_start = (seconds - smart_light_config.turn_on_start_time) >= 0 ? true:false;
    // set the start and end buff accordingly
    bool before_end = (seconds - smart_light_config.turn_on_end_time) <= 0 ? true:false;
    return (after_start && before_end);

}


bool isLightOffTime(char *time_buff){
    // get the time in seconds
    int32_t seconds = convTimeStrToSeconds(time_buff);
    bool after_start = (seconds - smart_light_config.turn_off_start_time) >= 0 ? true:false;
    bool after_end = (seconds - smart_light_config.turn_off_end_time) > 0 ? true: false;
    smart_light_config.has_turned_off = after_end;
    bool res = after_start && (!smart_light_config.has_turned_off);
    return res;
}


int32_t convTimeStrToSeconds(char *time_buff){
    // note that we assume the input has been checked here already
    // time string is formatted as ##:##:##
    char h_buff[3] = {0}, m_buff[3] = {0}, s_buff[3] = {0};
    int16_t h = 0, m = 0, s = 0;
    int32_t res = 0;
    h_buff[0] = time_buff[0];
    h_buff[1] = time_buff[1];
    m_buff[0] = time_buff[3];
    m_buff[1] = time_buff[4];
    s_buff[0] = time_buff[6];
    s_buff[1] = time_buff[7];
    // convert all to seconds and return the result
    h = atoi(h_buff) * 60 * 60;
    m = atoi(m_buff) * 60;
    s = atoi(s_buff);
    res = h + m + s;
    return res;
}