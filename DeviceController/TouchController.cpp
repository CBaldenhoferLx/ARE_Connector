#include "TouchController.h"

#include <LogHelper.h>
#include "TaskIDs.h"
#include "CommController.h"

TouchController::TouchController() : AbstractIntervalTask(1000) {
}

void TouchController::init() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
}

void TouchController::update() {
  int v[5] = {0};
  
  v[0] = analogRead(A0);
  v[1] = analogRead(A1);
  v[2] = analogRead(A2);
  v[3] = analogRead(A3);
  v[4] = analogRead(A4);

  for (uint8_t i=0;i<5;i++) {
    LOG_PRINT(i);
    LOG_PRINT(" ");
    LOG_PRINTLN(v[i]);
  }

  return;

  int m = 0;
  uint8_t maxIndex = -1;
  
  for (uint8_t i=0;i<4;i++) {
    if (v[i]>m) {
      m=v[i];
      maxIndex = i;
    }
  }

  LOG_PRINTLN(maxIndex);

  //taskManager->getTask<CommController*>(TOUCH_CONTROLLER)->sendPackage(CommController::ACTION_TOUCH_TRIGGERED, 0);

}
