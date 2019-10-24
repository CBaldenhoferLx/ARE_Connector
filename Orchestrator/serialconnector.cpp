#include "serialconnector.h"

#include <QDebug>

#include "protocol.h"

SerialConnector::SerialConnector(AppConfig *appConfig, QObject *parent) : DataReceiver(parent)
{
    connect(&m_port, &QSerialPort::errorOccurred, this, &SerialConnector::onErrorOccurred);
    connect(&m_port, &QSerialPort::readyRead, this, &SerialConnector::onReadRead);

    m_port.setPortName(appConfig->getValue("SERIAL_PORT", "COM22").toString());
    m_port.setBaudRate(appConfig->getValue("SERIAL_BAUDRATE", 9600).toInt());

    tryConnect();
}

void SerialConnector::sendData(Protocol::ProtocolAction action) {
    qDebug() << Q_FUNC_INFO;

    if (m_port.isWritable()) {
        QByteArray data = Protocol::serializeAction(action).toLatin1();
        qDebug() << "Writing serial data" << data;
        qint64 bytesWritten = m_port.write(data);
        qDebug() << "Written:" << bytesWritten;
    } else {
        qWarning() << "Cannot write to serial port";
    }
}

void SerialConnector::tryConnect() {
    qDebug() << Q_FUNC_INFO;

    if (m_port.isOpen() && m_port.isReadable()) return;
    if (m_port.isOpen()) m_port.close();

    if (!m_port.open(QIODevice::ReadWrite)) {
        qWarning() << "Cannot open port" << m_port.portName() << m_port.errorString();

        QTimer::singleShot(5000, this, &SerialConnector::tryConnect);
    } else {
        qDebug() << "Port connected";
    }
}

void SerialConnector::onReadRead() {
    qDebug() << Q_FUNC_INFO;

    QByteArray responseData = m_port.readAll();
    Protocol::ProtocolAction action = Protocol::parseMessage(Protocol::SENDER_ARDUINO_SERIAL, responseData);

    if (action.isValid) {
        Q_EMIT(dataReceived(action));
    }
}

void SerialConnector::onErrorOccurred(QSerialPort::SerialPortError error) {
    if (error==QSerialPort::NoError) {
        // do nothing
    } else if (error==QSerialPort::ResourceError) {
        qWarning() << Q_FUNC_INFO << "Reconnecting due to" << error;
        m_port.close();
        QTimer::singleShot(5000, this, &SerialConnector::tryConnect);
    } else {
        qWarning() << Q_FUNC_INFO << error;
    }
}
