#include "CommController.h"

#include <LogHelper.h>
#include "Debug.h"
#include "TaskIDs.h"

#include "FanController.h"

CommController::CommController() : AbstractTask() {
  
}

void CommController::init() {
  COMM_SERIAL.begin(SERIAL_SPEED);
}

void CommController::update() {
  char c = COMM_SERIAL.peek();
  
  if (c==DATAGRAM_START) {
    COMM_SERIAL.read();
    currentData = "";
    //LOG_PRINTLN("START");
  } else if (c==DATAGRAM_END) {
    COMM_SERIAL.read();
    handlePackage();
    currentData = "";
  } else if (c=='\n' || c=='\r') {
    COMM_SERIAL.read();
    currentData = "";
    //LOG_PRINTLN("RESET");
  } else if (c>0) {
    char b = COMM_SERIAL.read();
    currentData+=b;
    //LOG_PRINTLN("DATA");
  }
}

void CommController::handlePackage() {
  if (currentData.length()<DATA_PACKAGE_SIZE_MIN) return;
  int i = currentData.indexOf(DATAGRAM_SEP);
  
  if (i>0) {    // 0 is invalid
    int cmd = currentData.substring(0, i).toInt();
    int param = currentData.substring(i+1).toInt();

    switch(cmd) {
      case ACTION_SET_FAN_LEFT:
        taskManager->getTask<FanController*>(FAN_CONTROLLER)->setSpeed(0, param);
        break;
      case ACTION_SET_FAN_MID:
        taskManager->getTask<FanController*>(FAN_CONTROLLER)->setSpeed(1, param);
        break;
      case ACTION_SET_FAN_RIGHT:
        taskManager->getTask<FanController*>(FAN_CONTROLLER)->setSpeed(2, param);
        break;
    }
  } else {
    // invalid package
    LOG_PRINTLN(F("INVALID PACKAGE"));
  }
}

void CommController::sendPackage(PAction cmd, int param) {
  COMM_SERIAL.print(DATAGRAM_START);
  COMM_SERIAL.print(cmd);
  COMM_SERIAL.print(DATAGRAM_SEP);
  COMM_SERIAL.print(param);
  COMM_SERIAL.println(DATAGRAM_END);
}
