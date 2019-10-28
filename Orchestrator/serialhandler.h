#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QDebug>
#include <QTimer>

#include <windows.h>

#define ARDUINO_WAIT_TIME 2000


class SerialHandler : public QObject
{
    Q_OBJECT
public:
    explicit SerialHandler(QObject *parent = nullptr);
    ~SerialHandler();

    void init(QString portName, unsigned long baudRate);

    bool connect();

    int ReadData(char *buffer, unsigned int nbChar);

    bool WriteData(char *buffer, unsigned int nbChar);

    bool write(QByteArray data);

    QString portName() {
        return mPortName;
    }

private:
    bool IsConnected();

    QString mPortName;
    unsigned long mBaudRate;

    QSerialPort port;
    QTimer t;

    bool connected;

    HANDLE hSerial;

    DWORD errors;

    COMSTAT status;

    bool mDoRead = true;

    //QTimer mReadTimer;

signals:
    void dataReceived(QByteArray data);

private slots:
    void checkRead();

public slots:
};

#endif // SERIALHANDLER_H
