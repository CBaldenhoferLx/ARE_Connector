#ifndef FANCONTROLLER_H
#define FANCONTROLLER_H

//#include <Adafruit_MotorShield.h>

#include <AFMotor.h>

#include "AbstractIdleTask.h"

#define FAN_MOTOR_COUNT 3

class FanController : public AbstractIdleTask {
public:
  FanController();

  void init();

  void update();

  void setSpeed(uint8_t motorIndex, uint8_t speed);

private:

  void _setSpeed(uint8_t motorIndex, uint8_t speed);  
  

};


#endif    //FANCONTROLLER_H
