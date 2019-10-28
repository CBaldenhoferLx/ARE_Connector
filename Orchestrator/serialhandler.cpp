#include "serialhandler.h"

#include <QtConcurrent>

SerialHandler::SerialHandler(QObject *parent) : QObject(parent)
{

    //connect(&mReadTimer, &QTimer::timeout, this, &SerialHandler::onCheckRead);

    //mReadTimer.setInterval(10);
}

SerialHandler::~SerialHandler()
{
    //Check if we are connected before trying to disconnect
    if(this->connected)
    {
        //We're no longer connected
        this->connected = false;
        //Close the serial handler
        CloseHandle(this->hSerial);

        //mReadTimer.stop();
    }
}

void SerialHandler::init(QString portName, unsigned long baudRate) {
    //We're not yet connected
    this->connected = false;

    mPortName = portName;
    mBaudRate = baudRate;
}

bool SerialHandler::connect() {
    QString portToUse = "\\\\.\\" + mPortName;

    qDebug() << "Connecting to" << portToUse;

    wchar_t* ch = new wchar_t[portToUse.length() + 1];
    portToUse.toWCharArray(ch);

    //Try to connect to the given port throuh CreateFile
    this->hSerial = CreateFile(ch,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    //Check if the connection was successfull
    if(this->hSerial==INVALID_HANDLE_VALUE)
    {
        //If not success full display an Error
        if(GetLastError()==ERROR_FILE_NOT_FOUND){

            //Print Error if neccessary
            qWarning() << "Handle was not attached. Reason: port not available." << mPortName;

        }
        else
        {
            qWarning() << "ERROR!!!";
        }
    }
    else
    {
        //If connected we try to set the comm parameters
        DCB dcbSerialParams = {0};

        //Try to get the current
        if (!GetCommState(this->hSerial, &dcbSerialParams))
        {
            //If impossible, show an error
            qWarning() << "failed to get current serial parameters!";
        }
        else
        {
            //Define serial connection parameters for the arduino board
            dcbSerialParams.BaudRate=mBaudRate;
            dcbSerialParams.ByteSize=8;
            dcbSerialParams.StopBits=ONESTOPBIT;
            dcbSerialParams.Parity=NOPARITY;

             //Set the parameters and check for their proper application
             if(!SetCommState(hSerial, &dcbSerialParams))
             {
                qWarning() << "ALERT: Could not set Serial Port parameters";
             }
             else
             {
                 //If everything went fine we're connected
                 this->connected = true;
                 //We wait 2s as the arduino board will be reseting
                 Sleep(ARDUINO_WAIT_TIME);

                 qDebug() << "COnnected";

                 //mReadTimer.start();

                 mDoRead = true;
                 QtConcurrent::run(this, &SerialHandler::checkRead);

                 return true;
             }
        }
    }

    return false;
}

void SerialHandler::checkRead() {
    while(mDoRead) {
        char data[32] = {0};
        int bytesRead = ReadData(data, 32);

        if (bytesRead>0) {
            QByteArray data2(data, bytesRead);
            Q_EMIT(dataReceived(data2));
        }

        QThread::msleep(10);
    }
}

int SerialHandler::ReadData(char *buffer, unsigned int nbChar)
{
    //Number of bytes we'll have read
    DWORD bytesRead;
    //Number of bytes we'll really ask to read
    unsigned int toRead;

    //Use the ClearCommError function to get status info on the Serial port
    ClearCommError(this->hSerial, &this->errors, &this->status);

    //Check if there is something to read
    if(this->status.cbInQue>0)
    {
        //If there is we check if there is enough data to read the required number
        //of characters, if not we'll read only the available characters to prevent
        //locking of the application.
        if(this->status.cbInQue>nbChar)
        {
            toRead = nbChar;
        }
        else
        {
            toRead = this->status.cbInQue;
        }

        //Try to read the require number of chars, and return the number of read bytes on success
        if(ReadFile(this->hSerial, buffer, toRead, &bytesRead, NULL) && bytesRead != 0)
        {
            return bytesRead;
        }

    }

    //If nothing has been read, or that an error was detected return -1
    return -1;

}


bool SerialHandler::WriteData(char *buffer, unsigned int nbChar)
{
    DWORD bytesSend;

    //Try to write the buffer on the Serial port
    if(!WriteFile(this->hSerial, (void *)buffer, nbChar, &bytesSend, 0))
    {
        //In case it don't work get comm error and return false
        ClearCommError(this->hSerial, &this->errors, &this->status);

        return false;
    }
    else
        return true;
}

bool SerialHandler::write(QByteArray data) {
    return WriteData(data.begin(), data.length());
}

bool SerialHandler::IsConnected()
{
    //Simply return the connection status
    return this->connected;
}
