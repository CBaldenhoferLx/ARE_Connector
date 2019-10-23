#include <LogHelper.h>
#include <TaskManager.h>

#include "FanController.h"
#include "CommController.h"

TaskManager taskManager;

FanController fanController;
CommController commController;

void setup() {
  //LOG_INIT();

  taskManager.registerTask(&commController);
  taskManager.registerTask(&fanController);
  
  taskManager.init();
}

void loop() {
  taskManager.update();
  delay(10);
}
