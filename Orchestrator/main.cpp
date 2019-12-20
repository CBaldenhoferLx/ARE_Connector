#include <QCoreApplication>

#include "udpconnector.h"
#include "serialconnector.h"
#include "keyfilter.h"
#include "ultrahapticsconnector.h"
#include "orchestrator.h"

#include "appconfig.h"

int main(int argc, char *argv[])
{
    qDebug() << Q_FUNC_INFO;

    QCoreApplication a(argc, argv);

    AppConfig appConfig;

    Orchestrator orch;

    UDPConnector udpConn(&appConfig);
    SerialConnector serialConn(&appConfig);
    UltrahapticsConnector uhConn;

    uhConn.start();

    KeyFilter kvt;
    kvt.start();

    orch.addReceiver(SerialProtocol::SENDER_ML_UDP, &udpConn);
    orch.addReceiver(SerialProtocol::SENDER_ARDUINO_SERIAL, &serialConn);
    orch.addReceiver(SerialProtocol::SENDER_UL_LIB, &uhConn);

    QObject::connect(&kvt, &KeyFilter::keyPressed, [&orch](unsigned long key) {
        orch.handleKeyPressed(key);
    });

    qDebug() << "RUN";

    return a.exec();
}

