#ifndef SERIALCONNECTOR_H
#define SERIALCONNECTOR_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>

#include "appconfig.h"
#include "datareceiver.h"
#include "serialhandler.h"

class SerialConnector : public DataReceiver
{
    Q_OBJECT
public:
    explicit SerialConnector(AppConfig *appConfig, QObject *parent = nullptr);

    void sendData(SerialProtocol::SerialProtocolAction action);

private:
    //QSerialPort m_port;

    //QTimer m_readCheckTimer;

    void printPortInfos();

    SerialHandler m_handler;

    QByteArray mDataBuffer;

signals:

private slots:
    void tryConnect();

    void onDataReceived(QByteArray data);

    //void onReadRead();
    //void onErrorOccurred(QSerialPort::SerialPortError error);

};

#endif // SERIALCONNECTOR_H
