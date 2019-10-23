#ifndef FANCONTROLLER_H
#define FANCONTROLLER_H

//#include <Adafruit_MotorShield.h>

#include <AFMotor.h>

#include "AbstractTask.h"

#define FAN_MOTOR_COUNT 3

class FanController : public AbstractTask {
public:
  FanController();

  void init();

  void update();

  void setSpeed(uint8_t motorIndex, uint8_t speed);

private:
  //Adafruit_MotorShield *motorShield;

  AF_DCMotor *fans[FAN_MOTOR_COUNT];

};


#endif    //FANCONTROLLER_H
