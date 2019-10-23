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

    void addReceiver(Protocol::Senders sender, DataReceiver *receiver);

    void handleKeyPressed(unsigned long key);

signals:

public slots:

private slots:
    void onDataReceived(Protocol::ProtocolAction action);

private:
    void redirectMessage(Protocol::Senders receiver, Protocol::ProtocolAction action);

    QMap<Protocol::Senders, DataReceiver*> m_Receivers;

};

#endif // ORCHESTRATOR_H
