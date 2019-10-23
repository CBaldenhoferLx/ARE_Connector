#include <LogHelper.h>
#include <TaskManager.h>

#include "FanController.h"

TaskManager taskManager;

FanController fanController;

void setup() {
  LOG_INIT();

  taskManager.registerTask(&fanController);
  
  taskManager.init();
}

void loop() {
  taskManager.update();
  delay(10);
}
