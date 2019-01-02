#include <stdio.h>
#include <ArduCAM.h>
#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include "ICS.h"
#include "SICL.h"
#include "storage.h"
#include "memorysaver.h"


void setup() {
  // put your setup code here, to run once:

  // INIT DATA BUSES
  Wire.begin();
  SPI.begin();
  sicl_init();
  pinMode(CAM_VL_CS,OUTPUT);
  pinMode(CAM_IR_CS,OUTPUT);
  digitalWrite(CAM_VL_CS, HIGH);
  digitalWrite(CAM_IR_CS, HIGH);
   
  Serial.println(F("ArduCAM Start!"));

  //Initialize SD Card
#ifdef DEBUG
  Serial.println(F("Init SD Card"));
#else
  delay(10);
#endif

  if(sd_init() == 0)
  {
    card_present = true;
  }

  ics_init();
  time_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  
  if (stringComplete) 
  {
   // Serial.println(inputString);
 //   cli();//stop interrupts
    if(inputString[0]=='d')
    {
     // ics_send_vl_cam_sicl();
     sd_store_image();
    }
    if (process_sicl_msg() == 1)
    {
      //send_LSPDinitOK();
      setBusBusy();
      processCAMcommand();
      clrBusBusy();
    }
     // clear the string:
    inputString = "";
    stringComplete = false;  
 //   sei();//allow interrupts 
  }
  
  if(is_take_picture)
  {
    if(card_present)
    {
      sd_store_image();
    }
    else
    {
      if(sd_init() == 0)
      {
        card_present = true;
      }
    }
    is_take_picture = false;
  }
}

void serialEvent() 
{
 
  while (Serial.available()) 
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') 
    {
      stringComplete = true;
    }
  }
}
