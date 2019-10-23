#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>

#define DATAGRAM_START "@"
#define DATAGRAM_SEP ","
#define DATAGRAM_END ";"

/*
 * SET TOUCH MAX
 * @2,5;
 *
 * SET TOUCH OFF
 * @2,0;
*/

class Protocol : public QObject
{
    Q_OBJECT

public:
    enum Senders { SENDER_ML_UDP, SENDER_ARDUINO_SERIAL, SENDER_UL_LIB, SENDER_MAX };
    enum PAction { ACTION_NONE, ACTION_SET_FAN, ACTION_TOUCH_TRIGGERED, ACTION_MAX };

    Q_ENUM(Senders)
    Q_ENUM(PAction)

    struct ProtocolAction {
        Senders sender;
        bool isValid = false;
        PAction action = ACTION_NONE;
        double param = 0.0;
    };

    explicit Protocol(QObject *parent = nullptr);

    static ProtocolAction parseMessage(Senders sender, QString data);

    static QString serializeAction(ProtocolAction action);

signals:

public slots:

};

#endif // PROTOCOL_H