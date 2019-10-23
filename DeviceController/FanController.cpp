#include "FanController.h"

#include <LogHelper.h>

#include "TaskIDs.h"

FanController::FanController() : AbstractIntervalTask(20) {
}

void FanController::init() {
  motorShield = new Adafruit_MotorShield();
  
  motorShield->begin();
  
  fan1 = motorShield->getMotor(1);
  fan1->run(RELEASE);
  
  /*
  fan1->setSpeed(0);
  fan1->run(FORWARD);
  */
}

void FanController::update() {
}

void FanController::setSpeed(uint8_t motorIndex, uint8_t speed) {
  speed = constrain(speed, 0, 9);
  speed = map(speed, 0, 9, 0, 255);
  
  if (speed==0) {
    fan1->run(RELEASE);
  } else {
    fan1->run(FORWARD);
    fan1->setSpeed(speed);
  }
}
