#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include <QObject>
#include <QMap>

#include "protocol.h"
#include "datareceiver.h"

class Orchestrator : public QObject
{
    Q_OBJECT
public:
    explicit Orchestrator(QObject *parent = nullptr);

    void addReceiver(SerialProtocol::Senders sender, DataReceiver *receiver);

    void handleKeyPressed(unsigned long key);

signals:

public slots:

private slots:
    void onDataReceived(SerialProtocol::SerialProtocolAction action);

private:
    void redirectMessage(SerialProtocol::Senders receiver, SerialProtocol::SerialProtocolAction action);

    QMap<SerialProtocol::Senders, DataReceiver*> m_Receivers;

};

#endif // ORCHESTRATOR_H
