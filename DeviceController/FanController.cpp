#include "FanController.h"

#include <LogHelper.h>

#include "TaskIDs.h"

FanController::FanController() : AbstractIdleTask() {
}

void FanController::init() {
  //LOG_PRINTLN("INIT");

  for (uint8_t i=0;i<FAN_MOTOR_COUNT;i++) {
    fans[i] = new AF_DCMotor(1+i);
    fans[i]->setSpeed(0);
    fans[i]->run(RELEASE);
  }
}

void FanController::update() {
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
