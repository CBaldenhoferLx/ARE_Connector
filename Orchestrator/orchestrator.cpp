#include "orchestrator.h"

#include <QDebug>

Orchestrator::Orchestrator(QObject *parent) : QObject(parent)
{

}

void Orchestrator::handleKeyPressed(unsigned long key) {
    qDebug() << Q_FUNC_INFO << key;

    Protocol::ProtocolAction a;
    a.isValid = true;

    switch(key) {
    case 'f':
    case 'F':
        qDebug() << "Emitting Set Fan";

        a.param = 0;
        a.action = Protocol::ACTION_SET_FAN_LEFT;
        a.sender = Protocol::SENDER_ML_UDP;

        onDataReceived(a);
        break;
    case 't':
    case 'T':
        qDebug() << "Emitting Touch triggered";

        a.param = 0;
        a.action = Protocol::ACTION_TOUCH_TRIGGERED;
        a.sender = Protocol::SENDER_ARDUINO_SERIAL;

        onDataReceived(a);
        break;
    }
}

void Orchestrator::addReceiver(Protocol::Senders sender, DataReceiver *receiver) {
    m_Receivers.insert(sender, receiver);

    connect(receiver, &DataReceiver::dataReceived, this, &Orchestrator::onDataReceived);
}

void Orchestrator::onDataReceived(Protocol::ProtocolAction action) {
    qDebug() << Q_FUNC_INFO << action.action << action.param;

    switch(action.action) {
    case Protocol::ACTION_SET_FAN_LEFT:
    case Protocol::ACTION_SET_FAN_MID:
    case Protocol::ACTION_SET_FAN_RIGHT:
        redirectMessage(Protocol::SENDER_ARDUINO_SERIAL, action);
        break;
    case Protocol::ACTION_TOUCH_TRIGGERED:
        redirectMessage(Protocol::SENDER_UL_LIB, action);
        break;
    default:
        qWarning() << "Action not handled yet";
    }
}

void Orchestrator::redirectMessage(Protocol::Senders receiver, Protocol::ProtocolAction action) {
    if (m_Receivers.contains(receiver)) {
        m_Receivers.value(receiver)->sendData(action);
    } else {
        qWarning() << "Receiver not registered" << receiver;
    }
}
