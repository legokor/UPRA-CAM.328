#include <stdio.h>
#include <ArduCAM.h>
#include <Wire.h>
#include <SPI.h>
#include <SdFat.h>
#include "ICS.h"
#include "SICL.h"
#include "storage.h"
#include "memorysaver.h"
#include "temperature.h"
#include "LED.h"



void setup() {
  // put your setup code here, to run once:

  // INIT DATA BUSES
  Wire.begin();
  SPI.begin();
  sicl_init();
  pinMode(SD_PRESENT, INPUT_PULLUP);
  pinMode(CAM_VIS_CS, OUTPUT);
  pinMode(CAM_IR_CS, OUTPUT);
  digitalWrite(CAM_VIS_CS, HIGH);
  digitalWrite(CAM_IR_CS, HIGH);

  led_init();
  
#ifdef DEBUG   
  Serial.println(F("ArduCAM Start!"));
#endif

  led_on();

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
  led_status_indicator();
  if (stringComplete) 
  {
   // Serial.println(inputString);
 //   cli();//stop interrupts
    if(inputString[0]=='d')
    {
     // ics_send_vis_cam_sicl();
     sd_store_image();
    }
    msg_idx = process_sicl_msg();
    switch(msg_idx)
    {
      case 1:
      {
        setBusBusy();
        processCAMcommand();
        clrBusBusy();
        break;        
      }
      case 2:
      {
        setBusBusy();
        send_hk_packet();
        clrBusBusy();
        break;
      }
      default: break;
    }
     // clear the string:
    inputString = "";
    stringComplete = false;  
 //   sei();//allow interrupts 
  }
  
  if(is_take_picture)
  {
    if((card_present) && ((is_cam_ir_present) || (is_cam_vis_present)))
    {
      sd_store_image();
    }
//    else
//    {
//      if(sd_init() == 0)
//      {
//        card_present = true;
//      }
//    }
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
