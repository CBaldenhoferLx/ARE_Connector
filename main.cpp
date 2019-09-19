#include <QCoreApplication>

#include "udpconnector.h"
#include "serialconnector.h"
#include "keyfilter.h"
#include "orchestrator.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Orchestrator orch;

    UDPConnector udpConn;
    SerialConnector serialConn;

    KeyFilter kvt;
    kvt.start();

    orch.addReceiver(Protocol::SENDER_ML_UDP, &udpConn);
    orch.addReceiver(Protocol::SENDER_ARDUINO_SERIAL, &serialConn);

    QObject::connect(&kvt, &KeyFilter::keyPressed, [&orch](unsigned long key) {
        orch.handleKeyPressed(key);
    });

    qDebug() << "RUN";

    return a.exec();
}

