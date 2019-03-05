#ifndef ICS_H
#define ICS_H

//UNCOMMENT USED CAMERAS
#define CAM_VIS_PRESENT
#define CAM_IR_PRESENT


//This demo can only work on OV2640_MINI_2MP or OV5642_MINI_5MP or OV5642_MINI_5MP_BIT_ROTATION_FIXED
//or OV5640_MINI_5MP_PLUS or ARDUCAM_SHIELD_V2 platform.
#if !(defined OV5642_MINI_5MP || defined OV5642_MINI_5MP_BIT_ROTATION_FIXED || defined OV2640_MINI_2MP)
  #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

const int CAM_VIS_CS = 10;
const int CAM_IR_CS = 9;

#if defined (OV2640_MINI_2MP)
ArduCAM CAM_VIS( OV2640, CAM_VIS_CS );
ArduCAM CAM_IR( OV2640, CAM_IR_CS );
#else
ArduCAM CAM_VIS( OV5642, CAM_VIS_CS );
#endif

uint32_t picture_index;
uint8_t intervalometer_timeout = 10; // [s]
uint8_t intervalometer;

bool is_take_picture = false;
bool is_cam_vis_present;
bool is_cam_ir_present;

char cam_sel = 0;
char cam_command[4];
uint32_t cam_attribute;

#endif
