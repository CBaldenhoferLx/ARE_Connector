#ifndef TOUCHCONTROLLER_H
#define TOUCHCONTROLLER_H

#include "AbstractIntervalTask.h"

class TouchController : public AbstractIntervalTask {
public:
  TouchController();

  void init();

  void update();

private:

};


#endif    //TOUCHCONTROLLER_H
