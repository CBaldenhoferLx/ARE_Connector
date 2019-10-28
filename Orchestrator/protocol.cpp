#include "protocol.h"
#include <QDebug>

Protocol::Protocol(QObject *parent) : QObject(parent)
{

}

Protocol::ProtocolAction Protocol::parseMessage(Senders sender, QString data) {
    qDebug() << Q_FUNC_INFO << sender << data;

    ProtocolAction action;

    action.sender = sender;

    int s = data.indexOf(DATAGRAM_START);

    if (s>=0) {
        if (s>0) {
            //qDebug() << "Removing trash" << s;
            data = data.mid(s);
            //qDebug() << "Cleaned: " << data;
        }

        int i = data.indexOf(DATAGRAM_SEP);

        if (i>=0) {
            int actionId = data.mid(1, i-1).toInt();
            qDebug() << actionId;

            if (actionId<ACTION_MAX && actionId>0) {
                int o = data.indexOf(DATAGRAM_END, i);

                if (o>i) {
                    double param = data.mid(i+1, o-i-1).toDouble();

                    qDebug() << param;

                    action.action = static_cast<Protocol::PAction>(actionId);
                    action.param = param;
                    action.isValid = true;
                } else {
                    //qWarning() << "No datagram terminator";
                }
            } else {
                //qWarning() << "Invalid actionId" << actionId;
            }
        } else {
            //qWarning() << "Datagram corrupted";
        }
    } else {
        //qWarning() << "Missing datagram header";
    }

    return action;
}

QString Protocol::serializeAction(ProtocolAction action) {
    return DATAGRAM_START + QString::number(action.action) + DATAGRAM_SEP + QString::number(action.param) + DATAGRAM_END;
}
