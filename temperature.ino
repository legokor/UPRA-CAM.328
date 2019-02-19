double int_temp_read(void)
{
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - INTERNALCALIB ) * INTERNALGAIN; 
  //t = wADC; //CALIB
  // The returned temperature is in degrees Celsius.
  return (t);
}


int32_t get_pcb_temperature(void)
{
  double temp     = 0.0;
  double temp_avg = 0.0;
  double int_temp_tmp = 0.0;
  
  for (int i=0; i<AVRG; i++)
  {
    delay(10);
    int_temp_tmp = int_temp_read();
    int_temp_tmp = (int_temp_tmp + CALIBRATION_OFFSET) * CALIBRATION_GAIN;
    temp_avg += int_temp_tmp; 
  }
  
  temp = temp_avg/((double)AVRG); 

  pcb_temp = (int32_t)(temp * 10.0);
  return pcb_temp;

}




