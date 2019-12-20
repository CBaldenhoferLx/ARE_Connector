#include "orchestrator.h"

#include <QDebug>

Orchestrator::Orchestrator(QObject *parent) : QObject(parent)
{

}

void Orchestrator::handleKeyPressed(unsigned long key) {
    qDebug() << Q_FUNC_INFO << key;

    SerialProtocol::SerialProtocolAction a;
    a.isValid = true;

    switch(key) {
    case 'f':
    case 'F':
        qDebug() << "Emitting Set Fan";

        a.param = 0;
        a.action = SerialProtocol::ACTION_SET_FAN_LEFT;
        a.sender = SerialProtocol::SENDER_ML_UDP;

        onDataReceived(a);
        break;
    case 't':
    case 'T':
        qDebug() << "Emitting Touch triggered";

        a.param = 0;
        a.action = SerialProtocol::ACTION_TOUCH_TRIGGERED;
        a.sender = SerialProtocol::SENDER_ARDUINO_SERIAL;

        onDataReceived(a);
        break;
    }
}

void Orchestrator::addReceiver(SerialProtocol::Senders sender, DataReceiver *receiver) {
    m_Receivers.insert(sender, receiver);

    connect(receiver, &DataReceiver::dataReceived, this, &Orchestrator::onDataReceived);
}

void Orchestrator::onDataReceived(SerialProtocol::SerialProtocolAction action) {
    qDebug() << Q_FUNC_INFO << action.action << action.param;

    switch(action.action) {
    case SerialProtocol::ACTION_SET_FAN_LEFT:
    case SerialProtocol::ACTION_SET_FAN_MID:
    case SerialProtocol::ACTION_SET_FAN_RIGHT:
        redirectMessage(SerialProtocol::SENDER_ARDUINO_SERIAL, action);
        break;
    case SerialProtocol::ACTION_TOUCH_TRIGGERED:
        redirectMessage(SerialProtocol::SENDER_UL_LIB, action);
        break;
    case SerialProtocol::ACTION_SCAN_ENABLED:
        redirectMessage(SerialProtocol::SENDER_UL_LIB, action);
        break;
    default:
        qWarning() << "Action not handled yet";
    }
}

void Orchestrator::redirectMessage(SerialProtocol::Senders receiver, SerialProtocol::SerialProtocolAction action) {
    if (m_Receivers.contains(receiver)) {
        m_Receivers.value(receiver)->sendData(action);
    } else {
        qWarning() << "Receiver not registered" << receiver;
    }
}
