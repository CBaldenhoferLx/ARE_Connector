#include <LogHelper.h>
#include <TaskManager.h>

#include "FanController.h"
#include "CommController.h"
#include "TouchController.h"

TaskManager taskManager;

FanController fanController;
CommController commController;
TouchController touchController;

void setup() {
  //LOG_INIT();

  taskManager.registerTask(&commController);
  taskManager.registerTask(&fanController);
  taskManager.registerTask(&touchController);
  
  taskManager.init();
}

void loop() {
  taskManager.update();
  delay(10);
}
