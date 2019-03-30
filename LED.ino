bool is_led_on;
uint8_t status_cntr;

int32_t led_init()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);  
  is_led_on = false;
  status_cntr = STATUS_TIMEOUT;
}

void led_on()
{
  digitalWrite(LED, HIGH);  
  is_led_on = true;
}

void led_off()
{
  digitalWrite(LED, LOW);  
  is_led_on = false;
}

void led_toggle()
{
  if(is_led_on)
  {
    led_off();  
  }
  else
  {
    led_on();
  }
}

bool led_is_on()
{
  return is_led_on;
}

void led_reset_status_cntr()
{
  status_cntr = STATUS_TIMEOUT;
}

void led_status_indicator()
{
  if(is_take_picture)
  {
    return;
  }
  if((card_present) && ((is_cam_ir_present) && (is_cam_vis_present)))
  {
    led_on();
    return;
  }
  if((card_present == false) && ((is_cam_ir_present) || (is_cam_vis_present)))
  {
    led_toggle();
    return;
  }
  if((is_cam_ir_present == false) || (is_cam_vis_present == false))
  {
    if(status_cntr != 0)
    {
      status_cntr--;
      if(status_cntr == 0)
      {
        led_toggle();
        led_reset_status_cntr();
      }
    }
    return;
  }
}


