#ifndef SERIALCONNECTOR_H
#define SERIALCONNECTOR_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>

#include "datareceiver.h"

class SerialConnector : public DataReceiver
{
    Q_OBJECT
public:
    explicit SerialConnector(QObject *parent = nullptr);

    void sendData(Protocol::ProtocolAction action);

private:
    QSerialPort m_port;

signals:

private slots:
    void tryConnect();

    void onReadRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

};

#endif // SERIALCONNECTOR_H
