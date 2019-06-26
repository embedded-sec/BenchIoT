//=================================================================================================
//
// This file has the implementationfor the benchmark itself. The main file only calls the 
// benchmark main function (benchmarkMain).
//
//=================================================================================================



//=========================================== INCLUDES ==========================================//

#include "mbed.h"
#include "connected_display.h"

//======================================== DEFINES & GLOBALS ====================================//


DigitalOut led1(LED1);
Serial pc(USBTX, USBRX, NULL, 9600); 
IOT2FILE jpeg_file, res_file;

char ack[] ="ack";
//========================================= FUNCTIONS ===========================================//


void benchmarkMain(void){

    char welcome_msg[] = "Connected-display benchmark";
    char orig_img_name[] = "image";
    char compressed_img[] = "compr";
    char decompressed_img[] = "decom";
    char tmp_ext[] = ".tmp";
    char jpeg_ext[] = ".jpg";
    char curr_img[64] = {0};
    uint32_t bytesbmp = 0;
    size_t recv_buff_size = 256;
    char *recv_buff= new char[256];
    int32_t bytes = 0;

    
    // networking variables
    BareMetalEthernet eth;
    BareMetalTCP socket;
    
    IoT2_DisplayInterface display;
    IOT2FILE *jpeg_file_ptr, *res_file_ptr;
    jpeg_file_ptr = &jpeg_file;
    res_file_ptr = &res_file;

    //-------------------------------------------------------------------------------------------//
    //                                    Initialization                                         //
    //-------------------------------------------------------------------------------------------//

    // initialize Ethernet/TCP
    iot2InitTCP(&eth, &socket);
    
    // display benchmark msg
    display.config(welcome_msg);
    
    // initialize file system
    iot2fs_init();
    iot2fs_fmount();
    
    //-------------------------------------------------------------------------------------------//
    //                                      Handle login                                         //
    //-------------------------------------------------------------------------------------------//




    //-------------------------------------------------------------------------------------------//
    //                                       Recv images                                         //
    //-------------------------------------------------------------------------------------------//



    for (uint8_t i = 0; i < TOTAL_IMGS; i++){

        // recv cmd
        bytes = iot2recv(&socket, recv_buff, recv_buff_size);
        if (bytes < 0){
            iot2SerialDebugMsg("[-] ERROR: at recv");
        }

        // handle cmd
        handleConnDispCmd(recv_buff, recv_buff_size, &socket);
        memset(recv_buff, 0, recv_buff_size);
    }



    //-------------------------------------------------------------------------------------------//
    //                        Decompress, compress, and display Image                            //
    //-------------------------------------------------------------------------------------------//



    for (uint8_t i = 0; i < TOTAL_IMGS; i++){


        // Re-open jpeg file with read permissions only
        setupImageName(curr_img, orig_img_name, jpeg_ext, sizeof(curr_img) , 
            sizeof(orig_img_name), sizeof(jpeg_ext), i);
        //pc.printf("[+] Opening image = %s\r\n", curr_img);
        jpeg_file_ptr = iot2fs_fopen(jpeg_file_ptr, curr_img, "r");

        // Open a temporary file for the decompressed results of the JPG image
        setupImageName(curr_img, decompressed_img, tmp_ext, sizeof(curr_img) , 
            sizeof(decompressed_img), sizeof(tmp_ext), i);
        //pc.printf("[+] Opening file = %s\r\n", curr_img);
        res_file_ptr = iot2fs_fopen(res_file_ptr, curr_img, "w");

        
        // Decode the jpeg image and write the result to the new temporary file.
        decodeJpegToBMP(jpeg_file_ptr,res_file_ptr, IMAGE_QUALITY, IMAGE_WIDTH, IMAGE_HEIGHT,
            IMAGE_COMPONENTS, &display);
        
        // Show the name of the image below the image itself
        display.displayStrAt(0, DISPLAY_CENTER_YPOS, (uint8_t*)curr_img, CENTER_MODE);
        
        //wait(3);
        // clear the display
        display.clear(LCD_COLOR_WHITE);
        
        // Close both files
        iot2fs_fclose(jpeg_file_ptr);
        iot2fs_fclose(res_file_ptr);
/*
        // Open a file the re-compressed JPG image
        setupImageName(curr_img, compressed_img, jpeg_ext, sizeof(curr_img) , 
            sizeof(compressed_img), sizeof(jpeg_ext), i);
        pc.printf("[+] Opening Image = %s\r\n", curr_img);
        jpeg_file_ptr =  iot2fs_fopen(jpeg_file_ptr,curr_img, "w");

        // Open temporary file that has the stored results of decompressing
        // the original JPG image earlier
        setupImageName(curr_img, decompressed_img, tmp_ext, sizeof(curr_img) , 
                        sizeof(decompressed_img), sizeof(tmp_ext), i);

        pc.printf("[+] Opening file = %s\r\n", curr_img);
        res_file_ptr = iot2fs_fopen(res_file_ptr, curr_img, "r");
        
        // Encode BMP image to jpeg
        encodeBmpToJpeg(res_file_ptr, jpeg_file_ptr, IMAGE_QUALITY, IMAGE_WIDTH, IMAGE_HEIGHT, 
            IMAGE_COMPONENTS);

        // Close the files
        iot2fs_fclose(res_file_ptr);
        iot2fs_fclose(jpeg_file_ptr);
*/
        if(iot2fs_remove(curr_img) != 0){
            iot2SerialDebugMsg("Unable to delete file");
            iot2Error();
        }
        
    }



    //-------------------------------------------------------------------------------------------//
    //                                     Send result images                                    //
    //-------------------------------------------------------------------------------------------//
/*
    iot2send(&socket, ack, sizeof(ack));
    for (uint8_t i = 0; i < TOTAL_IMGS; i++){

        // recv cmd
        bytes = iot2recv(&socket, recv_buff, sizeof(recv_buff));
        if (bytes < 0){
            iot2SerialDebugMsg("[-] ERROR: at recv");
        }

        // handle cmd
        handleConnDispCmd(recv_buff, sizeof(recv_buff), &socket);
        memset(recv_buff, 0, sizeof(recv_buff));
    }
*/
/*   
    while(1) {
       // pc.printf("[+] Welcome to mbed SDK template!\n");
        led1 = 1;
        wait(0.2);
        //uint32_t tick_val = us_ticker_read();
          //pc.printf("[+] tick = ?\n");
        led1 = 0;
        wait(0.2);
        //iot2Error();
    }
*/
    // free recv buff
    delete[] recv_buff;    
}



void encodeBmpToJpeg(JFILE *res_fd, JFILE *jpeg_fd, uint16_t img_quality, uint16_t img_w, uint16_t img_h, uint8_t img_comp){
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;
    uint32_t bytes = 0;

    // BMP image buffer
    uint8_t img_buff[1024];

    // Next line to read in JPEG
    JDIMENSION next_line;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // setup the destination file for encoded jpeg image
    jpeg_stdio_dest(&cinfo, jpeg_fd);

    cinfo.image_width = img_w;  
    cinfo.image_height = img_h;
    cinfo.input_components = img_comp;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);

    cinfo.dct_method  = JDCT_FLOAT;
    jpeg_set_quality(&cinfo, img_quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = img_w * img_comp;


    while (cinfo.next_scanline < cinfo.image_height) {
        next_line = cinfo.next_scanline* row_stride;
        
        // SEEK_SET is chosen in here to enable running for both
        // SDIO/SPI. It will reset res_fd to the desired position of each 
        // iteration is case of SPI. Note that we are handling the offset manually
        // in this application via next line.
        if(iot2fs_fseek(res_fd, next_line, SEEK_SET) == 0){
            bytes = iot2fs_fread(img_buff, 1, row_stride, res_fd);
            if (bytes > 0){
                row_pointer[0] = (JSAMPROW)img_buff;
                (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
            }
        }
    }

    jpeg_finish_compress(&cinfo);

    jpeg_destroy_compress(&cinfo);
}


void decodeJpegToBMP(JFILE *jpeg_fd, JFILE *res_fd, uint16_t img_quality, uint16_t img_w, 
    uint16_t img_h, uint8_t img_comp, IoT2_DisplayInterface *disp){

    // reset display column counter
    disp->colCntr = 0;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW out_buff[2]= {0};
    uint8_t img_buff[1024]= {0};
    out_buff[0] = img_buff;
    int row_stride = 0;
    ssize_t bytes = 0;

    // Next line to read in JPEG
    JDIMENSION next_line;

    //pc.printf("[+] decode  BEFORE step(1)\r\n");
    // Step1 (1): JPEG decompression object allocation and initalization
    cinfo.err = jpeg_std_error(&jerr);      // Set error routine
    jpeg_create_decompress(&cinfo);         // Init. decompress struct

    // Step (2): set source (i.e., jpeg file fd)
    jpeg_stdio_src(&cinfo, jpeg_fd);

    // Step (3): read file params
    (void) jpeg_read_header(&cinfo, TRUE);
    
    // Step (4): set decompression params
    cinfo.dct_method = JDCT_FLOAT;

    // Step (5): start decompression
    jpeg_start_decompress(&cinfo);
    row_stride = img_w * img_comp;

    // Step (6): Loop thorugh decompression output
    //  (i)  Show image on display
    //  (ii) Write output to BMP image
    while(cinfo.output_scanline < cinfo.output_height){
        
        (void) jpeg_read_scanlines(&cinfo, out_buff, 1);

        bytes = iot2fs_fwrite(img_buff, 1, row_stride, res_fd);
        if(bytes < 0){  
            // Error
            iot2Error();
        }

        // Show the resulting image on display
        disp->showBMPImg(out_buff[0], img_w);

    }

    // Step (7): End decompression
    jpeg_finish_decompress(&cinfo);

    // Step (8): Free jpeg decompression object
    jpeg_destroy_decompress(&cinfo);


}


void setupImageName(char* imgFullPath, const char* imgName, const char* imgExt, 
    size_t imgFullPathSize , size_t imgNameSize, size_t imgExtSize, uint8_t index){

    // check that the size of the target (i.e., ImgFullpath) is greater or equal to dir/file.ext 
    // sizes before operating. If the sizes do not satisfy this return immediatly without setting
    // the name. Note that 3 below is the size of the index (0 becomes 000, 91 becomes 091..etc)
    // Size of the dir is setup below. The name of the dir is hardcoded in the begining of sprintf.
    // [iot2-debug]: Modify this to put an error instead of looping, modify explaination above
    size_t dirNameSize = 9;
    if ( (dirNameSize+imgNameSize+imgExtSize+3) > imgFullPathSize){
        iot2Error();
    }

    sprintf(imgFullPath, "ConnDisp/%s%03u%s", imgName, index, imgExt);
}

void handleConnDispCmd(char* buff, size_t buff_size, BareMetalTCP *socket){

    // There are 4 cmds, thus we have 5 cases:
    // 1) login ---> handled earlier
    // 2) pass  ---> handled earlier
    // 3) put (3 tokens)
    // 4) get (2 tokens)
    // 5) invalid
    char cmd_buff[4] = {0};
    char filename_buff[13] = {0};   // we are limited by name length as 8chars+[.ext]
    int32_t size_val = 0;
    uint32_t cmd_length = 0;

    // get the length of the command
    for (uint8_t i = 0; i < buff_size; i++){
        if  (buff[i] == ' '){
            cmd_length = i;
            break;
        }
    }

    // if it is not 3, then it is invalid already
    if (cmd_length != 3){
        handleInvalidCmd(buff, buff_size, socket);
        return;
    }

    // the cmd is either put or get
    // for put
    if(strncmp(buff, PUT_FILE_CMD, strlen(PUT_FILE_CMD)) == 0){
        // get the filename
        memcpy(filename_buff, &buff[cmd_length+1], sizeof(filename_buff)-1);
        // get the size
        size_val = parseFileSize(buff, strlen(buff), cmd_length+sizeof(filename_buff)+1);
        recvImgFile(socket, filename_buff, strlen(filename_buff), 
            size_val, buff, buff_size );

    }
    // for get
    else if(strncmp(buff, GET_FILE_CMD, strlen(GET_FILE_CMD)) == 0){
        // get the filename
        memcpy(filename_buff, &buff[cmd_length+1], sizeof(filename_buff)-1);
        // send the file
        sendImgFile(socket, filename_buff, strlen(filename_buff),
                    buff, buff_size);
    }

    // an invalid cmd
    else{
        handleInvalidCmd(buff, buff_size, socket);
        return;
    }


}


void handleInvalidCmd(char *buff, size_t buff_size, BareMetalTCP *socket){

    char invalid_cmd[] = INVALID_CMD;
    // clear the buffer
    memset(buff, 0, buff_size);
    // set the buff to invalid request response
    for (uint8_t i = 0; i < strlen(INVALID_CMD); i++){
        buff[i] = invalid_cmd[i];
    }

    if(iot2send(socket, buff, buff_size) < 0 ){
        iot2SerialDebugMsg("[-] ERROR: send INVALID_CMD failed");
        iot2Error();
    }

}

// assumes that the first token is checked as put. Now it extracts
// the name 
void handlePutCmd(char *buff, size_t buff_size, BareMetalTCP *socket);


//int32_t parseFileSize(char *buff, size_t buff_size, uint8_t start_idx);


int32_t parseFileSize(char *buff, size_t buff_size, uint8_t start_idx){
    char *size_buff = new char[buff_size - start_idx+1];
    uint8_t max_size_buff_size = 9;
    int32_t size_val = 0;

    // copy chars and verify they are digits
    for(uint8_t i = 0; i < buff_size-start_idx; i++){
        // avoid using library functions as much as possible
        if (buff[i+start_idx] < '0' || buff[i+start_idx] > '9'){
            iot2SerialDebugMsg("[-] ERROR: non digit value for size");
            // non digit, return -1 for error
            delete[] size_buff;
            return -1;
        }
        size_buff[i] = buff[i+start_idx];
    }

    size_buff[buff_size-start_idx] = '\0';

    //pc.printf("strlen(size_buff = %s, len=%d\r\n", size_buff, strlen(size_buff));
    // before calling atoi, make sure the size is permissible
    if (strlen(size_buff) > max_size_buff_size){
        iot2SerialDebugMsg("[-] ERROR: file size is too large");
        delete[] size_buff;
        return -1;
    }

    // get size value
    size_val = atoi(size_buff);

    // free size buff
    delete[] size_buff;
    return size_val;
}



void recvImgFile(BareMetalTCP *socket, char *filename_buff, size_t filename_buff_size,
               int32_t file_size, char* buff, size_t buff_size){

    IOT2FILE requested_file;
    IOT2FILE *file_ptr;
    file_ptr = &requested_file;
    int32_t recv_size = 0, buff_size_int = buff_size;
    ssize_t bytes;
    char full_path[22] ={0};    // the full path size if limited as fatfs configs
    uint8_t fullpath_prefix_len = 9;
    full_path[0] = 'C';
    full_path[1] = 'o';
    full_path[2] = 'n';
    full_path[3] = 'n';
    full_path[4] = 'D';
    full_path[5] = 'i';
    full_path[6] = 's';
    full_path[7] = 'p';
    full_path[8] = '/';

    // validate size
    if (filename_buff_size > (sizeof(full_path) - fullpath_prefix_len - 1)){
        // if larger, change it to the fixed size
        filename_buff_size = sizeof(full_path) - fullpath_prefix_len - 1;
    }

    for (uint8_t i = 0; i < filename_buff_size; i++){
        full_path[i+9] = filename_buff[i];
    }

    // open a file using the full file path
    file_ptr = iot2fs_fopen(file_ptr,full_path, "w");
    
    pc.printf("recveived = %d, total_file_size = %d\r\n", recv_size, file_size);
    while(recv_size < file_size){
        //pc.printf("recveived = %d, total_file_size = %d\r\n", recv_size, file_size);
        if ((file_size-recv_size) >= buff_size_int){
            bytes = iot2recv(socket, buff, buff_size);
            if( bytes < 0){
                iot2SerialDebugMsg("[-] ERROR: recv returned < 0");
                iot2Error();
            }
            
            // write to file
            bytes = iot2fs_fwrite(buff, 1, bytes,file_ptr);

        }else{
            bytes = iot2recv(socket, buff, file_size - recv_size);
            if(bytes < 0){
                iot2SerialDebugMsg("[-] ERROR: recv returned < 0");
                iot2Error();
            }
            // write the remaining bytes to the file
            bytes = iot2fs_fwrite(buff, 1, bytes, file_ptr);

        }
        // update recv size
        recv_size += bytes;
        //memset(buff, 0, buff_size);
        // send ack to server
        iot2send(socket, ack, sizeof(ack));

    }
    pc.printf("recveived = %d, total_file_size = %d\r\n", recv_size, file_size);
    // close the file
    if (iot2fs_fclose(file_ptr) != 0){
        iot2SerialDebugMsg("[-] ERROR: close file return != 0");
        iot2Error();
    }

}


void sendImgFile(BareMetalTCP *socket, char *filename_buff, size_t filename_buff_size,
                char* buff, size_t buff_size){

    IOT2FILE requested_file;
    IOT2FILE *file_ptr;
    file_ptr = &requested_file;
    int32_t send_size = 0, file_size = 0, buff_size_int = buff_size;
    ssize_t bytes;
    char full_path[22] ={0};    // the full path size if limited as fatfs configs
    uint8_t fullpath_prefix_len = 9;
    full_path[0] = 'C';
    full_path[1] = 'o';
    full_path[2] = 'n';
    full_path[3] = 'n';
    full_path[4] = 'D';
    full_path[5] = 'i';
    full_path[6] = 's';
    full_path[7] = 'p';
    full_path[8] = '/';

    // validate size
    if (filename_buff_size > (sizeof(full_path) - fullpath_prefix_len - 1)){
        // if larger, change it to the fixed size
        filename_buff_size = sizeof(full_path) - fullpath_prefix_len - 1;
    }

    for (uint8_t i = 0; i < filename_buff_size; i++){
        full_path[i+9] = filename_buff[i];
    }

    // open a file using the full file path
    file_ptr = iot2fs_fopen(file_ptr,full_path, "r");

    // get the file size
    file_size = iot2fs_fsize(file_ptr);

    // send the size information
    memset(buff, 0, buff_size);
    snprintf(buff, buff_size, "get size:%ld", file_size);
    bytes = iot2send(socket, buff, buff_size);

    // send the contents of the file
    pc.printf("send_size = %d, file_size = %d\r\n", send_size, file_size);
    while(send_size < file_size){
        //pc.printf("send_size = %d, file_size = %d\r\n", send_size, file_size);
        if ((file_size-send_size) >= buff_size_int){
            
            // read from the file
            bytes = iot2fs_fread(buff, 1, buff_size,file_ptr);

            bytes = iot2send(socket, buff, buff_size);
            if( bytes < 0){
                iot2SerialDebugMsg("[-] ERROR: send returned < 0");
                iot2Error();
            }
            
        }else{

            // write the remaining bytes to the file
            bytes = iot2fs_fread(buff, 1, file_size - send_size, file_ptr);
            bytes = iot2send(socket, buff, bytes);
            if(bytes < 0){
                iot2SerialDebugMsg("[-] ERROR: send returned < 0");
                iot2Error();
            }

        }
        // update send size
        send_size += bytes;
        memset(buff, 0, buff_size);
        // send ack to server
        iot2send(socket, ack, sizeof(ack));
        
    }
    pc.printf("send_size = %d, file_size = %d\r\n", send_size, file_size);
    // close the file
    if (iot2fs_fclose(file_ptr) != 0){
        iot2SerialDebugMsg("[-] ERROR: close file return != 0");
        iot2Error();
    }
}