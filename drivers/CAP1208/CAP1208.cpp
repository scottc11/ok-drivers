#include "CAP1208.h"


void CAP1208::init() {
  
  i2cWrite(AVR_SMPL_CONF_REG, 0x00);        // speed up sampling time
  i2cWrite(SENSITIVITY, 0xF);               // delta sense @ 128
  i2cWrite(MULT_TOUCH_CONF_REG, 0x00);      // allow multiple touches
  i2cWrite(INT_ENABLE_REG, 0xFF);           // enable interupts
  i2cWrite(REPEAT_RATE_ENABLE_REG, 0x00);   // disable repeat rate for all channels
  i2cWrite(CONF_2_REG, 0x60);               // disable BLK_PWR_CTRL power saving feature

  clearInterrupt();
}

void CAP1208::disableInterupts() {
  i2cWrite(INT_ENABLE_REG, 0x00);
}

/**
 * @brief read product id register to test connection
 *
 * @return true
 * @return false
 */
bool CAP1208::isConnected()
{
  if (i2cRead(MANUFACTURER_ID_REG) == CAP12x8_MAN_ID)
  {
    return true;
  }
  else
  {
    return false;
  }
}

uint8_t CAP1208::getControlStatus() {
  return i2cRead(MAIN_CTRL_REG);
}

uint8_t CAP1208::getGeneralStatus() {
  return i2cRead(GENERAL_STATUS_REG);
}

void CAP1208::calibrate() {
  i2cWrite(CALIBRATE_REG, 0xFF);
}

/**
 * The ALERT# pin is an active low output that is driven when an interrupt event is detected.
 * Interrupts are indicated by the setting of the INT bit in the Main Control Registe
 * 
 * for some reason we have to "clear" the INT bit everytime we read the sensors...
*/
void CAP1208::clearInterrupt() {
  i2cWrite(MAIN_CTRL_REG, MAIN_CTRL_REG & ~0x01);
#ifdef osCMSIS_FreeRTOS
  vTaskDelay(1);
#endif
}

// read input status of CAP1208
uint8_t CAP1208::touched() {
  this->clearInterrupt();
  uint8_t data = i2cRead(INPUT_STATUS_REG);
  return data;
}

// if a pad *is* touched
bool CAP1208::padIsTouched(int pad, int currTouched) {
  return (bool)bitwise_read_bit(currTouched, pad);
}

// if a pad it *wasn't* touched and now *is*, alert!
bool CAP1208::padWasTouched(int pad, int currTouched, int prevTouched) {
  return ((bool)bitwise_read_bit(currTouched, pad) && !(bool)bitwise_read_bit(prevTouched, pad));
}