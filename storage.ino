int sd_init(void)
{
  char tmp_sd_str[8];
  char input = 0;
  int i;
  int ret = 0;
  
  if (!sd.begin(SD_CS, SPI_HALF_SPEED)) 
  {
    #ifdef DEBUG  
    Serial.println(F("CAM: NO SD Card"));
    #else
    delay(10);
    #endif
 
    return 1;
  }
  if (outFile.open("index.dat", O_READ))
  {
    #ifdef DEBUG  
    Serial.println(F("CAM: Read index"));
    #else
    delay(10);
    #endif

    for(i = 0; (i < 8) && (input != '\r') && (input != '\n'); i++)
    {
      tmp_sd_str[i] = 0;
      input = outFile.read();        
      tmp_sd_str[i] = input;        
    }
    outFile.close();
    picture_index = sd_str_to_int(tmp_sd_str);
    
    #ifdef DEBUG  
    Serial.println(F("CAM: index set"));
    #else
    delay(10);
    #endif

    ret = 0;
  }
  else
  {
    picture_index = 0;
    if (!outFile.open("index.dat", O_RDWR | O_CREAT | O_AT_END))
    {
      ret = 2;
    }
    else
    {
      outFile.println(F("0"));
    }
    outFile.close();
  }

  if (outFile.open("intval.dat", O_READ))
  {
    #ifdef DEBUG  
    Serial.println(F("CAM: Read intervalometer"));
    #else
    delay(10);
    #endif

    input = 0;
    for(i = 0; (i < 3) && (input != '\r') && (input != '\n'); i++)
    {
      tmp_sd_str[i] = 0;
      input = outFile.read();        
      tmp_sd_str[i] = input;        
    }
    outFile.close();
    intervalometer_timeout = sd_str_to_int(tmp_sd_str);
    
    #ifdef DEBUG  
    Serial.print(F("CAM: intervalometer:"));
    Serial.println(intervalometer_timeout);
    #else
    delay(10);
    #endif

    return 0;
  }
  else
  {
    #ifdef DEBUG  
    Serial.println(F("CAM: create intervalometer config"));
    #else
    delay(10);
    #endif

    if(!sd_store_intervalometer_timeout(10))
    {
      ret = 3;
    }
    else
    {
      ret = 0;
    }
  }
  
  #ifdef DEBUG  
  Serial.println(F("CAM: success"));
  #else
  delay(10);
  #endif

  return ret;
}

int32_t sd_store_image(void)
{
  int32_t ret = 0;
  
  #ifdef CAM_VL_PRESENT
    //Flush the FIFO
    CAM_VL.flush_fifo();
    //Clear the capture done flag
    CAM_VL.clear_fifo_flag();
  #endif

  #ifdef CAM_IR_PRESENT
    //Flush the FIFO
    CAM_IR.flush_fifo();
    //Clear the capture done flag
    CAM_IR.clear_fifo_flag();
  #endif
    
  //Start capture
  #ifdef CAM_VL_PRESENT
    CAM_VL.start_capture();
  #endif
  #ifdef CAM_IR_PRESENT
    CAM_IR.start_capture();
  #endif
  
  Serial.println(F("start Capture"));
  #ifdef CAM_VL_PRESENT
    while(!CAM_VL.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  #endif
  #ifdef CAM_IR_PRESENT
    while(!CAM_IR.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  #endif
  Serial.println(F("Capture Done."));  

  #ifdef CAM_VL_PRESENT
    ret = sd_store_vl_image();
  #endif
  #ifdef CAM_IR_PRESENT
    ret += sd_store_ir_image();
  #endif
  sd_store_picture_index();
  intervalometer = intervalometer_timeout;
  return ret;
}

#ifdef CAM_VL_PRESENT
int32_t sd_store_vl_image(void)
{
  char sd_str[13];
  byte sd_buf[256];
  static int i = 0;
  static int k = 0;
  uint8_t sd_tmp = 0,sd_tmp_last=0;
  uint32_t sd_fifo_lenght = 0;
  bool sd_is_header = false;

  sd_fifo_lenght = CAM_VL.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(sd_fifo_lenght, DEC);
  
  if (sd_fifo_lenght >= MAX_FIFO_SIZE) //384K
  {
    Serial.println(F("Over size."));
    return -1;
  }
  if (sd_fifo_lenght == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
    return -2;
  }
  //Consd_struct a file name
  sprintf(sd_str, "%05d_vl.jpg", picture_index);
//  itoa(picture_index, sd_str, 10);
//  sd_strcat(sd_str, "_vl.jpg");
  
  //Open the new file
  //outFile = SD.open(sd_str, O_WRITE | O_CREAT | O_TRUNC);
  if(!outFile.open(sd_str, O_RDWR | O_CREAT | O_AT_END)){
    Serial.println(F("File open failed"));
    return -3;
  }
  CAM_VL.CS_LOW();
  CAM_VL.set_fifo_burst();
  while ( sd_fifo_lenght-- )
  {
    sd_tmp_last = sd_tmp;
    sd_tmp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (sd_tmp == 0xD9) && (sd_tmp_last == 0xFF) ) //If find the end ,break while,
    {
      sd_buf[i++] = sd_tmp;  //save the last  0XD9     
      //Write the remain bytes in the sd_buffer
      CAM_VL.CS_HIGH();
      outFile.write(sd_buf, i);    
      //Close the file
  //    outFile.close();
      Serial.println(F("Image save OK."));
      sd_is_header = false;
      i = 0;
    }  
    if (sd_is_header == true)
    { 
      //Write image data to sd_buffer if not full
      if (i < 256)
      sd_buf[i++] = sd_tmp;
      else
      {
        //Write 256 bytes image data to file
        CAM_VL.CS_HIGH();
        outFile.write(sd_buf, 256);
        i = 0;
        sd_buf[i++] = sd_tmp;
        CAM_VL.CS_LOW();
        CAM_VL.set_fifo_burst();
      }        
    }
    else if ((sd_tmp == 0xD8) & (sd_tmp_last == 0xFF))
    {
      sd_is_header = true;
      sd_buf[i++] = sd_tmp_last;
      sd_buf[i++] = sd_tmp;   
    } 
  }
  //Close the file
  outFile.close();

  return 0;
}
#endif

#ifdef CAM_IR_PRESENT
int32_t sd_store_ir_image(void)
{
  char sd_str[13];
  byte sd_buf[256];
  static int i = 0;
  static int k = 0;
  uint8_t sd_tmp = 0,sd_tmp_last=0;
  uint32_t sd_fifo_lenght = 0;
  bool sd_is_header = false;

  sd_fifo_lenght = CAM_IR.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(sd_fifo_lenght, DEC);
  
  if (sd_fifo_lenght >= MAX_FIFO_SIZE) //384K
  {
    Serial.println(F("Over size."));
    return -1;
  }
  if (sd_fifo_lenght == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
    return -2;
  }
  //Consd_struct a file name
  sprintf(sd_str, "%05_vl.jpg", picture_index);
//  itoa(picture_index, sd_str, 10);
//  sd_strcat(sd_str, "_vl.jpg");
  
  //Open the new file
  //outFile = SD.open(sd_str, O_WRITE | O_CREAT | O_TRUNC);
  if(!outFile.open(sd_str, O_RDWR | O_CREAT | O_AT_END)){
    Serial.println(F("File open failed"));
    return -3;
  }
  CAM_IR.CS_LOW();
  CAM_IR.set_fifo_burst();
  while ( sd_fifo_lenght-- )
  {
    sd_tmp_last = sd_tmp;
    sd_tmp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (sd_tmp == 0xD9) && (sd_tmp_last == 0xFF) ) //If find the end ,break while,
    {
      sd_buf[i++] = sd_tmp;  //save the last  0XD9     
      //Write the remain bytes in the sd_buffer
      CAM_IR.CS_HIGH();
      outFile.write(sd_buf, i);    
      //Close the file
  //    outFile.close();
      Serial.println(F("Image save OK."));
      sd_is_header = false;
      i = 0;
    }  
    if (sd_is_header == true)
    { 
      //Write image data to sd_buffer if not full
      if (i < 256)
      sd_buf[i++] = sd_tmp;
      else
      {
        //Write 256 bytes image data to file
        CAM_IR.CS_HIGH();
        outFile.write(sd_buf, 256);
        i = 0;
        sd_buf[i++] = sd_tmp;
        CAM_IR.CS_LOW();
        CAM_IR.set_fifo_burst();
      }        
    }
    else if ((sd_tmp == 0xD8) & (sd_tmp_last == 0xFF))
    {
      sd_is_header = true;
      sd_buf[i++] = sd_tmp_last;
      sd_buf[i++] = sd_tmp;   
    } 
  }
  //Close the file
  outFile.close();

  return 0;
}
#endif

void sd_store_picture_index(void)
{
  picture_index++;
  if(outFile.open("index.dat", O_RDWR | O_CREAT | O_TRUNC | O_AT_END))
  {
    outFile.println(picture_index, DEC);
  }
  outFile.close();  
}

uint32_t sd_store_intervalometer_timeout(uint32_t timeout)
{
    if(timeout > 120)
    {
      intervalometer_timeout = 120;
    }
    else
    {
      intervalometer_timeout = timeout;
    }
    
    if (!outFile.open("intval.dat", O_RDWR | O_CREAT | O_TRUNC ))
    {
      return 1;
    }
    outFile.println(intervalometer_timeout, DEC);
    outFile.close();  
    intervalometer = intervalometer_timeout;
    return 0;
}

uint32_t sd_str_to_int(char* index_sd_str)
{
  int i;
  uint32_t tmp = 0;
  
  for(i = 0; (i < 8) && (index_sd_str[i] != '\r') && (index_sd_str[i] != '\n'); i++)
  {
    if ((index_sd_str[i] >= '0') && (index_sd_str[i] <= '9'))
    {
      tmp = tmp * 10;
      tmp += (unsigned int)(index_sd_str[i] - '0');
    }
  }
  return tmp;
}


