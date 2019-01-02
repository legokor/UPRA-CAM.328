uint32_t sicl_init(void)
{
  Serial.begin(SICL_BAUD);
  pinMode(BUSBUSY, INPUT);
  inputString.reserve(100);
  return 0;
}

bool busBusy_interrupt(void)
{
  if (digitalRead(BUSBUSY) != 1)
  {
    return true;
  }
  return false;
}

void setBusBusy(void)
{
  pinMode(BUSBUSY, OUTPUT);
  digitalWrite(BUSBUSY, LOW);
}

void clrBusBusy(void)
{
  digitalWrite(BUSBUSY, HIGH);
  pinMode(BUSBUSY, INPUT);
}

void clrCamCommand(void)
{
  uint8_t i;
  for(i = 0; i < 4; i++)
  {
    cam_command[i] = 0;
  }
}

int process_sicl_msg()
{
  int i=0;
  int msg_index=0;
  int msg_code = 0;
  
  for(i=0; i<100; i++)
  {
    if ((inputString[i] =='$') || (msg_index >= 80))
    {
      msg_index = 0;
    }
 
    if (inputString[i] != '\r')
    {
      msg[msg_index++] = inputString[i];
    }
 
    if (inputString[i] == '\n')
    {
      i = 200;
     // msg_index = 0;
    }    
  }
  if ((msg[1] == 'T') && (msg[2] == 'M') && (msg[3] == 'C') && (msg[4] == 'A') && (msg[5] == 'M'))
  {
    
  #ifdef DEBUG2
    Serial.begin(SICL_BAUD);
    Serial.println(F("CAM: Camera control message arrived")); //debug
  #endif
    //setBusBusy();
    processTMCAMcommand(msg_index);
    msg_index=0;
    msg_code = 1;
  }
  else if ((msg[1] == 'T') && (msg[2] == 'M') && (msg[3] == 'H') && (msg[4] == 'K') && (msg[5] == 'R'))
  {
    
  #ifdef DEBUG2
    Serial.begin(SICL_BAUD);
    Serial.println(F("CAM: HK request arrived")); //debug
  #endif
//    processTMHKRcommand(msg_index);
    msg_index=0;
    msg_code = 2;
  }  
  return msg_code;
  
}

void processTMCAMcommand(int msg_index)
{
  int i, j, k, IntegerPart;
  // $TMCAM,1,       CAP         *47
  //      cam_sel   command     chksm
  //        1         2 
 
  IntegerPart = 1;
  cam_attribute = 0;
 
  for (i=0, j=0, k=0; (i<msg_index) && (j<10); i++) // We start at 7 so we ignore the '$$TMCAM,'
  {
    if ((msg[i] == ',') || (msg[i] == '*'))
    {
      j++;    // Segment index
      k=0;    // Index into target variable
      IntegerPart = 1;
    }
    else
    {
      if (j == 1)
      {
        if ((msg[i] >= '0') && (msg[i] <= '9'))
        {        
          cam_sel = msg[i] - '0';
          k++;
        }
      }
      else if (j == 2)
      {
        if(IntegerPart == 1)
        {
          cam_command[k] = msg[i];
          k++;
        }
      }
      else if (j == 3)
      {
        if ((msg[i] >= '0') && (msg[i] <= '9'))
        {        
          cam_attribute = cam_attribute * 10;
          cam_attribute += (uint32_t)( msg[i] - '0');
          k++;
        }
      }
    }
  }
}

int processCAMcommand(void)
{
  int32_t error=0;
  if ((cam_command[0] == 'C') && (cam_command[1] == 'A') && (cam_command[2] == 'P'))
  {
 //   Serial.begin(SICL_BAUD);
    clrCamCommand();
    
    error = ics_send_cam_data_sicl(cam_sel);
//    pinMode(SICLTX, INPUT);
    if(error == -1)
    {
      Serial.println(F("CAM: Capture Handshake Timeout"));
      return 1;
    }
  }
  else if ((cam_command[0] == 'I') && (cam_command[1] == 'N') && (cam_command[2] == 'T'))
  {
    clrCamCommand();
    
    sd_store_intervalometer_timeout(cam_attribute);
    Serial.println(intervalometer_timeout, DEC);
  }
  return 0;
}
