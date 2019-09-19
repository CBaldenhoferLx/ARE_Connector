#ifndef DATARECEIVER_H
#define DATARECEIVER_H

#include <QObject>

#include "protocol.h"

class DataReceiver : public QObject
{
    Q_OBJECT
public:
    DataReceiver(QObject *parent) :QObject (parent) {}

    virtual void sendData(Protocol::ProtocolAction action) = 0;

signals:
    void dataReceived(Protocol::ProtocolAction action);

};

#endif // DATARECEIVER_H
