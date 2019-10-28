#include "serialconnector.h"

#include <QDebug>
#include <QSerialPortInfo>

#include "protocol.h"

SerialConnector::SerialConnector(AppConfig *appConfig, QObject *parent) : DataReceiver(parent)
{
    /*
    connect(&m_port, &QSerialPort::errorOccurred, this, &SerialConnector::onErrorOccurred);
    connect(&m_port, &QSerialPort::readyRead, this, &SerialConnector::onReadRead);

    m_port.setPortName(appConfig->getValue("SERIAL_PORT", "COM22").toString());
    m_port.setBaudRate(appConfig->getValue("SERIAL_BAUDRATE", 9600).toInt());
    */

    connect(&m_handler, &SerialHandler::dataReceived, this, &SerialConnector::onDataReceived);

    m_handler.init(appConfig->getValue("SERIAL_PORT", "COM22").toString(), appConfig->getValue("SERIAL_BAUDRATE", 9600).toUInt());

    tryConnect();
}

void SerialConnector::sendData(Protocol::ProtocolAction action) {
    qDebug() << Q_FUNC_INFO;

    /*
    if (m_port.isWritable()) {
        QByteArray data = Protocol::serializeAction(action).toLatin1();
        qDebug() << "Writing serial data" << data;
        qint64 bytesWritten = m_port.write(data);
        qDebug() << "Written:" << bytesWritten;
        m_port.flush();
    } else {
        qWarning() << "Cannot write to serial port";
    }*/

    QByteArray data = Protocol::serializeAction(action).toLatin1();
    if (!m_handler.write(data)) {
        qWarning() << "Cannot write to serial port";
    }
}

void SerialConnector::tryConnect() {
    qDebug() << Q_FUNC_INFO;

    /*
    if (m_port.isOpen() && m_port.isReadable()) return;
    if (m_port.isOpen()) m_port.close();

    if (!m_port.open(QIODevice::ReadWrite)) {
        qWarning() << "Cannot open port" << m_port.portName() << m_port.errorString();

        printPortInfos();

        QTimer::singleShot(5000, this, &SerialConnector::tryConnect);
    } else {
        qDebug() << "Port connected" << m_port.portName();

        connect(&m_readCheckTimer, &QTimer::timeout, [this](){
            m_port.setDataTerminalReady(true);
            m_port.setRequestToSend(true);

            qDebug() << m_port.bytesAvailable();
        });

        qDebug() << "Re-Setting baud rate" << m_port.baudRate();

        m_port.setBaudRate(m_port.baudRate());
        m_port.setParity(QSerialPort::NoParity);
        m_port.setDataBits(QSerialPort::Data8);
        m_port.setStopBits(QSerialPort::OneStop);
        m_port.setFlowControl(QSerialPort::NoFlowControl);

        m_readCheckTimer.start(1000);
    }*/

    mDataBuffer.clear();

    if (!m_handler.connect()) {
        qWarning() << "Cannot open port" << m_handler.portName();

        printPortInfos();

        QTimer::singleShot(5000, this, &SerialConnector::tryConnect);
    } else {
        qDebug() << "Port connected" << m_handler.portName();
    }
}

void SerialConnector::onDataReceived(QByteArray data) {
    qDebug() << Q_FUNC_INFO << data.size();

    mDataBuffer.append(data);

    QList<QByteArray> tokens = mDataBuffer.split('\n');
    QListIterator<QByteArray> it(tokens);

    mDataBuffer.clear();

    //qDebug() << tokens.size();

    while(it.hasNext()) {
        QByteArray buff = it.next();

        Protocol::ProtocolAction action = Protocol::parseMessage(Protocol::SENDER_ARDUINO_SERIAL, buff);
        if (action.isValid) {
            Q_EMIT(dataReceived(action));
        } else {
            mDataBuffer.append(buff);
        }
    }
}

void SerialConnector::printPortInfos() {
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString s = QObject::tr("Port:") + info.portName() + "\n"
                    + QObject::tr("Location:") + info.systemLocation() + "\n"
                    + QObject::tr("Description:") + info.description() + "\n"
                    + QObject::tr("Manufacturer:") + info.manufacturer() + "\n"
                    + QObject::tr("Serial number:") + info.serialNumber() + "\n"
                    + QObject::tr("Vendor Identifier:") + (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Product Identifier:") + (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Busy:") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) + "\n";
        qDebug() << s;
    }
}

/*
void SerialConnector::onReadRead() {
    qDebug() << Q_FUNC_INFO << m_port.bytesAvailable();

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
*/
