#include "udpconnector.h"

#include <QNetworkDatagram>
#include <QDebug>

#include "protocol.h"

UDPConnector::UDPConnector(AppConfig *appConfig, QObject *parent) : DataReceiver (parent)
{
    m_receive_port = appConfig->getValue("UDP_RECEIVE_PORT", 7755).toUInt();

    m_send_host = appConfig->getValue("UDP_SEND_HOST", "172.30.90.117").toString();
    m_send_port = appConfig->getValue("UDP_SEND_PORT", 7766).toUInt();

    initSocket();
}

void UDPConnector::sendData(Protocol::ProtocolAction action) {
    qDebug() << Q_FUNC_INFO;
    udpSocket->writeDatagram(Protocol::serializeAction(action).toLatin1(), QHostAddress(m_send_host), m_send_port);
}

void UDPConnector::initSocket()
{
    qDebug() << Q_FUNC_INFO;

    udpSocket = new QUdpSocket(this);
    if (udpSocket->bind(QHostAddress::Any, m_receive_port)) {
        qDebug() << "Listening on UDP" << m_receive_port;
    } else {
        qWarning() << "Failed to bind to UDP" << m_receive_port;
    }


    connect(udpSocket, &QUdpSocket::readyRead,
            this, &UDPConnector::readPendingDatagrams);
}

void UDPConnector::readPendingDatagrams()
{
    qDebug() << Q_FUNC_INFO;

    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        processDatagram(datagram);
    }
}

void UDPConnector::processDatagram(QNetworkDatagram &datagram) {
    qDebug() << Q_FUNC_INFO << datagram.data();

    Protocol::ProtocolAction action = Protocol::parseMessage(Protocol::SENDER_ML_UDP, datagram.data());
    if (action.isValid) {
        Q_EMIT(dataReceived(action));
    } else {
        qWarning() << "Invalid message";
    }
}
