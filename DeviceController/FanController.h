#ifndef FANCONTROLLER_H
#define FANCONTROLLER_H

#include <Adafruit_MotorShield.h>

#include "AbstractIntervalTask.h"

class FanController : public AbstractIntervalTask {
public:
  FanController();

  void init();

  void update();

  void setSpeed(uint8_t motorIndex, uint8_t speed);

private:
  Adafruit_MotorShield *motorShield;

  Adafruit_DCMotor *fan1;
  
};


#endif    //FANCONTROLLER_H
