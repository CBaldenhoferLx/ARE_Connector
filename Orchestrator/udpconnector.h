#ifndef UDPCONNECTOR_H
#define UDPCONNECTOR_H

#include <QObject>
#include <QUdpSocket>

#include "datareceiver.h"

class UDPConnector : public DataReceiver
{
    Q_OBJECT
public:
    explicit UDPConnector(QObject *parent = nullptr);

    void sendData(Protocol::ProtocolAction action);

    void initSocket();

private:
    QUdpSocket* udpSocket;

    void processDatagram(QNetworkDatagram &datagram);

private slots:
    void readPendingDatagrams();

signals:

public slots:
};

#endif // UDPCONNECTOR_H
