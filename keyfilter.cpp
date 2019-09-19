#include "keyfilter.h"

#include <QDebug>
#include <QEvent>

KeyFilter::KeyFilter(QObject *parent) : QThread(parent)
{

}

void KeyFilter::run() {
    DWORD control_key = 0;
    WORD keypress = 0;

    while(keypress != VK_ESCAPE)
    {
      keypress = get_keypress(control_key);
      Q_EMIT(keyPressed(keypress));
    }
}

WORD KeyFilter::get_keypress(DWORD& control_key)
{
  HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
  DWORD events = 0;      // how many events took place
  INPUT_RECORD input_record;  // a record of input events
  DWORD input_size = 1;    // how many characters to read
  bool not_a_keypress = true;
  BOOL read = 0;

  do
  {
    // Stop from echoing.
    FlushConsoleInputBuffer(input_handle);
    read = ReadConsoleInput(input_handle, &input_record, input_size, &events);
    if(!read)
    { // ReadConsoleInput failed.
      control_key = 0;
      return 0;
    }

    control_key = input_record.Event.KeyEvent.dwControlKeyState;

    if(input_record.EventType == KEY_EVENT && input_record.Event.KeyEvent.bKeyDown)
    { // A key was pressed, so return it.
      return input_record.Event.KeyEvent.wVirtualKeyCode;
    }
  } while(not_a_keypress);

  control_key = 0;
  return 0;
}
