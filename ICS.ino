int32_t ics_init(void)
{
  #ifdef CAM_VIS_PRESENT
    ics_CAM_VIS_init();
  #endif

  #ifdef CAM_IR_PRESENT
    ics_CAM_IR_init();
  #endif

  intervalometer = intervalometer_timeout;
  
  return 0;
}

#ifdef CAM_VIS_PRESENT
int32_t ics_CAM_VIS_init(void)
{
  uint8_t vid, pid;
  uint8_t temp;
  


  delay(1000);
  //Check if the ArduCAM SPI bus is OK
  CAM_VIS.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = CAM_VIS.read_reg(ARDUCHIP_TEST1);
   
  if (temp != 0x55){
  #ifdef DEBUG 
    Serial.println("SPI1 interface Error!");
  #endif
    is_cam_vis_present = false;
    return -1;
    //while(1);
  }
   
  #if defined (OV2640_MINI_2MP)
    //Check if the camera module type is OV2640
    CAM_VIS.wrSensorReg8_8(0xff, 0x01);  
    CAM_VIS.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    CAM_VIS.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26) || (pid != 0x42))
    {
    #ifdef DEBUG 
      Serial.println("Can't find OV2640 module!");
    #endif
      is_cam_vis_present = false;
      return -2;
    }
    else
    {
    #ifdef DEBUG 
      Serial.println("OV2640 detected.");
    #endif
      is_cam_vis_present = true;
    }
  #else
   //Check if the camera module type is OV5642
    CAM_VIS.wrSensorReg16_8(0xff, 0x01);
    CAM_VIS.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    CAM_VIS.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42))
    {
    #ifdef DEBUG 
      Serial.println("Can't find OV2640 module!");
    #endif
      is_cam_vis_present = false;
      return -2;
    }
    else
    {
    #ifdef DEBUG 
      Serial.println("OV2640 detected.");
    #endif
      is_cam_vis_present = true;
    }
  #endif
 
  CAM_VIS.set_format(JPEG);
  CAM_VIS.InitCAM();
  
  #if defined (OV2640_MINI_2MP)
  //   CAM_VIS.OV2640_set_JPEG_size(OV2640_640x480);
    CAM_VIS.OV2640_set_JPEG_size(OV2640_1600x1200);
  #else
    CAM_VIS.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
    CAM_VIS.OV5642_set_JPEG_size(OV5642_320x240);
  #endif
  //  pinMode(SICLTX, INPUT);
  
  delay(1000);
  return 0;
}
#endif

#ifdef CAM_IR_PRESENT
int32_t ics_CAM_IR_init(void)
{
  uint8_t vid, pid;
  uint8_t temp;
  
  pinMode(CAM_IR_CS,OUTPUT);

  delay(1000);
  //Check if the ArduCAM SPI bus is OK
  CAM_IR.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = CAM_IR.read_reg(ARDUCHIP_TEST1);
   
  if (temp != 0x55){
    Serial.println("SPI1 interface Error!");
    is_cam_ir_present = false;
    return -1;
    //while(1);
  }
   
  #if defined (OV2640_MINI_2MP)
    //Check if the camera module type is OV2640
    CAM_IR.wrSensorReg8_8(0xff, 0x01);  
    CAM_IR.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    CAM_IR.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26) || (pid != 0x42))
    {
      Serial.println("Can't find OV2640 module!");
      is_cam_ir_present = false;
      return -2;
    }
    else
    {
      Serial.println("OV2640 detected.");
      is_cam_ir_present = true;
    }
  #else
   //Check if the camera module type is OV5642
    CAM_IR.wrSensorReg16_8(0xff, 0x01);
    CAM_IR.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    CAM_IR.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42))
    {
      Serial.println("Can't find OV5642 module!");
      is_cam_ir_present = false;
      return -2;
    }
    else
    {
      Serial.println("OV5642 detected.");
      is_cam_ir_present = true;
    }
  #endif
 
  CAM_IR.set_format(JPEG);
  CAM_IR.InitCAM();
  
  #if defined (OV2640_MINI_2MP)
  //   CAM_IR.OV2640_set_JPEG_size(OV2640_640x480);
    CAM_IR.OV2640_set_JPEG_size(OV2640_1600x1200);
  #else
    CAM_IR.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
    CAM_IR.OV5642_set_JPEG_size(OV5642_320x240);
  #endif
  //  pinMode(SICLTX, INPUT);
  
  delay(1000);
  return 0;
}
#endif

int32_t ics_send_cam_data_sicl(uint32_t cam)
{
  switch(cam)
  {
    case 1: 
    {
      #ifdef CAM_VIS_PRESENT      
        return ics_send_vis_cam_sicl();
      #endif
      return -5;
    }
    case 2:
    {
      #ifdef CAM_IR_PRESENT      
        return ics_send_ir_cam_sicl();
      #endif
      return -5;
    }
    default:
    {
      return -5;
    }
  }
  return 0;
}

#ifdef CAM_VIS_PRESENT
int32_t ics_send_vis_cam_sicl(void)
{
  char str[8];
  byte buf[256];
  static int i = 0;
  static int k = 0;
  char handshake=0;
  uint8_t temp = 0,temp_last=0;
  volatile int cam_wtchdg=0;
  int nowtime=0; 

  if( is_cam_vis_present == false)
  {
    return -2;
  }
//  cli();//disable interrupts
  intervalometer = 0;
  //Set image size
  CAM_VIS.OV2640_set_JPEG_size(OV2640_176x144);
  //Flush the FIFO
  CAM_VIS.flush_fifo();
  //Clear the capture done flag
  CAM_VIS.clear_fifo_flag();
  //Start capture
  CAM_VIS.start_capture();
//  Serial.println("star Capture");
 while(!CAM_VIS.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
 Serial.println("$CMDTA,S,*47");  

 uint32_t fifo_lenght = CAM_VIS.read_fifo_length();
  

 i = 0;
 CAM_VIS.CS_LOW();
 CAM_VIS.set_fifo_burst();

 temp=SPI.transfer(0x00);
 fifo_lenght--;
 //Serial.write(temp);
 //Read JPEG data from FIFO
 while ( (temp !=0xD9) | (temp_last !=0xFF))
 {
  temp_last = temp;
  temp = SPI.transfer(0x00);
  //Write image data to buffer if not full
  if( i < 256)
  {
   buf[i++] = temp;
  }
  else
  {
  //Write 256 bytes image data to file
  CAM_VIS.CS_HIGH();
  //file.write(buf ,256);
  for(k=0;k<256;k++)
  {
    Serial.write(buf[k]); 
    
    handshake=0;
    cam_wtchdg = millis();
    while(handshake != 'o')
    {
      nowtime=millis();
      if(nowtime - cam_wtchdg >3000)
      {        
        CAM_VIS.CS_HIGH();
        //Reset image size
        CAM_VIS.OV2640_set_JPEG_size(OV2640_1600x1200);
        return -1;
      }
      if(Serial.available() >0)
      {
        handshake=Serial.read();
      }
    }
  }
  
  i = 0;
  buf[i++] = temp;
  CAM_VIS.CS_LOW();
  CAM_VIS.set_fifo_burst();
  }
  delay(0); 
 }
 
 //Write the remain bytes in the buffer
 if(i > 0){
  CAM_VIS.CS_HIGH();
  for(k=0;k<i;k++)
  {
    Serial.write(buf[k]);    
    handshake=0;
    cam_wtchdg = millis();
    while(handshake != 'o')
    {
      nowtime=millis();
      if(nowtime - cam_wtchdg >3000)
      {        
        CAM_VIS.CS_HIGH();
        //Reset image size
        CAM_VIS.OV2640_set_JPEG_size(OV2640_1600x1200);        
        return -1;
      }
      if(Serial.available() >0)
      {
        handshake=Serial.read();
      }
    }
  } 
 }
  Serial.println("");  
  Serial.println("$CMDTA,E,*47"); 
  clrBusBusy();
  //Reset image size
  CAM_VIS.OV2640_set_JPEG_size(OV2640_1600x1200);
//  sei();//allow interrupts
  is_take_picture = true;  
  return 0;
}
#endif

#ifdef CAM_IR_PRESENT
int32_t ics_send_ir_cam_sicl(void)
{
  char str[8];
  byte buf[256];
  static int i = 0;
  static int k = 0;
  char handshake=0;
  uint8_t temp = 0,temp_last=0;
  volatile int cam_wtchdg=0;
  int nowtime=0; 

  if( is_cam_ir_present == false)
  {
    return -2;
  }

//  cli();//disable interrupts
  intervalometer = 0;
  //Set image size
  CAM_IR.OV2640_set_JPEG_size(OV2640_176x144);
  //Flush the FIFO
  CAM_IR.flush_fifo();
  //Clear the capture done flag
  CAM_IR.clear_fifo_flag();
  //Start capture
  CAM_IR.start_capture();
//  Serial.println("star Capture");
 while(!CAM_IR.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
 Serial.println("$CMDTA,S,*47");  

 uint32_t fifo_lenght = CAM_IR.read_fifo_length();
  

 i = 0;
 CAM_IR.CS_LOW();
 CAM_IR.set_fifo_burst();

 temp=SPI.transfer(0x00);
 fifo_lenght--;
 //Serial.write(temp);
 //Read JPEG data from FIFO
 while ( (temp !=0xD9) | (temp_last !=0xFF))
 {
  temp_last = temp;
  temp = SPI.transfer(0x00);
  //Write image data to buffer if not full
  if( i < 256)
   buf[i++] = temp;
   else{
    //Write 256 bytes image data to file
    CAM_IR.CS_HIGH();
    //file.write(buf ,256);
    for(k=0;k<256;k++)
    {
      Serial.write(buf[k]); 
      
      handshake=0;
      cam_wtchdg = millis();
      while(handshake != 'o')
      {
        nowtime=millis();
        if(nowtime - cam_wtchdg >3000)
        {        
          CAM_IR.CS_HIGH();
          //Reset image size
          CAM_IR.OV2640_set_JPEG_size(OV2640_1600x1200);
          return -1;
        }
        if(Serial.available() >0)
        {
          handshake=Serial.read();
        }
      }
    }

    i = 0;
    buf[i++] = temp;
    CAM_IR.CS_LOW();
    CAM_IR.set_fifo_burst();
   }
   delay(0); 
 }
 
 //Write the remain bytes in the buffer
 if(i > 0){
  CAM_IR.CS_HIGH();
  for(k=0;k<i;k++)
  {
    Serial.write(buf[k]);    
    handshake=0;
    cam_wtchdg = millis();
    while(handshake != 'o')
    {
      nowtime=millis();
      if(nowtime - cam_wtchdg >3000)
      {        
        CAM_IR.CS_HIGH();
        //Reset image size
        CAM_IR.OV2640_set_JPEG_size(OV2640_1600x1200);        
        return -1;
      }
      if(Serial.available() >0)
      {
        handshake=Serial.read();
      }
    }
  } 
 }
  Serial.println("");  
  Serial.println("$CMDTA,E,*47"); 
  clrBusBusy();
  //Reset image size
  CAM_IR.OV2640_set_JPEG_size(OV2640_1600x1200);
//  sei();//allow interrupts
  is_take_picture = true;  
  return 0;
}
#endif
