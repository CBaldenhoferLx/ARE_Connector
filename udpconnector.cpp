#include "udpconnector.h"

#include <QNetworkDatagram>
#include <QDebug>

#include "protocol.h"

UDPConnector::UDPConnector(QObject *parent) : DataReceiver (parent)
{
    initSocket();
}

void UDPConnector::sendData(Protocol::ProtocolAction action) {
    qDebug() << Q_FUNC_INFO;
    udpSocket->writeDatagram(Protocol::serializeAction(action).toLatin1(), QHostAddress("172.30.90.117"), 7766);
}

void UDPConnector::initSocket()
{
    qDebug() << Q_FUNC_INFO;

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, 7755);

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
