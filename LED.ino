bool is_led_on;

int32_t led_init()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);  
  is_led_on = false;
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

void led_sd_error_indicator()
{
  if(card_present)
  {
    return;
  }
  led_toggle();
}

void led_status_indicator()
{
  if((card_present) && ((is_cam_ir_present) || (is_cam_vis_present)))
  {
    led_on();
  }
  else if(card_present)
  {
    led_off();
  }
}


