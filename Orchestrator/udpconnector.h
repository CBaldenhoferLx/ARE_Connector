#ifndef UDPCONNECTOR_H
#define UDPCONNECTOR_H

#include <QObject>
#include <QUdpSocket>
#include "appconfig.h"

#include "datareceiver.h"

class UDPConnector : public DataReceiver
{
    Q_OBJECT
public:
    explicit UDPConnector(AppConfig *appConfig, QObject *parent = nullptr);

    void sendData(Protocol::ProtocolAction action);

    void initSocket();

private:
    QUdpSocket* udpSocket;

    quint16 m_receive_port = 7755;

    QString m_send_host;
    quint16 m_send_port = 7766;

    void processDatagram(QNetworkDatagram &datagram);

private slots:
    void readPendingDatagrams();

signals:

public slots:
};

#endif // UDPCONNECTOR_H
