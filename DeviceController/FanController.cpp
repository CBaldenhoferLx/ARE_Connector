#include "FanController.h"

#include <LogHelper.h>

#include "TaskIDs.h"

FanController::FanController() : AbstractTask() {
}

void FanController::init() {
  //motorShield = new Adafruit_MotorShield();
  
  //motorShield->begin();
  
  LOG_PRINTLN("INIT");

  //fan1 = motorShield->getMotor(1);
  //fan1->run(RELEASE);

  for (uint8_t i=0;i<FAN_MOTOR_COUNT;i++) {
    fans[i] = new AF_DCMotor(1+i);
    fans[i]->setSpeed(0);
    fans[i]->run(RELEASE);
  }
}

void FanController::update() {
  /*
  for (uint8_t i=0;i<FAN_MOTOR_COUNT;i++) {
    fans[i]->run(FORWARD);
    fans[i]->setSpeed(fan_speeds[i]);
  }*/
}

void FanController::setSpeed(uint8_t motorIndex, uint8_t speed) {
  if (motorIndex>=FAN_MOTOR_COUNT) {
    LOG_PRINTLN(F("INVALID MOTOR INDEX"));
    return;
  }

  uint8_t s = constrain(speed, 0, 9);
  s = map(s, 0, 9, 0, 255);

  if (s==0) {
    fans[motorIndex]->setSpeed(0);
    fans[motorIndex]->run(RELEASE);
  } else {
    fans[motorIndex]->run(FORWARD);
    fans[motorIndex]->setSpeed(s);
  }
}
