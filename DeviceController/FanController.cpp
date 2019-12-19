#include "FanController.h"

#include <LogHelper.h>

#include "TaskIDs.h"

AF_DCMotor m1(1);
AF_DCMotor m2(2);
AF_DCMotor m3(3);

FanController::FanController() : AbstractIdleTask() {
}

void FanController::init() {
  //LOG_PRINTLN("INIT");

  m1.setSpeed(0);
  m2.setSpeed(0);
  m3.setSpeed(0);

  m1.run(RELEASE);
  m2.run(RELEASE);
  m3.run(RELEASE);

  for (uint8_t i=0;i<FAN_MOTOR_COUNT;i++) {
    //fans[i]->setSpeed(0);
    //fans[i]->run(RELEASE);
  }
}

void FanController::update() {
}

void FanController::_setSpeed(uint8_t motorIndex, uint8_t speed) {
  LOG_PRINT(motorIndex);
  LOG_PRINT(" ");
  LOG_PRINTLN(speed);

  switch(motorIndex) {
    case 0:
      m1.setSpeed(speed);
      m1.run(speed==0 ? RELEASE : FORWARD);
      break;
    case 1:
      m2.setSpeed(speed);
      m2.run(speed==0 ? RELEASE : FORWARD);
      break;
    case 2:
      m3.setSpeed(speed);
      m3.run(speed==0 ? RELEASE : FORWARD);
      break;
  }
}

void FanController::setSpeed(uint8_t motorIndex, uint8_t speed) {
  LOG_PRINT(motorIndex);
  LOG_PRINT(" ");
  LOG_PRINTLN(speed);
  
  if (motorIndex>=FAN_MOTOR_COUNT) {
    LOG_PRINTLN(F("INVALID MOTOR INDEX"));
    return;
  }

  uint8_t s = constrain(speed, 0, 9);
  s = map(s, 0, 9, 0, 255);

  _setSpeed(motorIndex, s);

/*
  if (s==0) {
    fans[motorIndex]->setSpeed(0);
    fans[motorIndex]->run(RELEASE);
  } else {
    fans[motorIndex]->run(FORWARD);
    fans[motorIndex]->setSpeed(s);
  }

  */
}
