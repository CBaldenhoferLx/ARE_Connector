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

class SerialProtocol : public QObject
{
    Q_OBJECT

public:
    enum Senders { SENDER_ML_UDP, SENDER_ARDUINO_SERIAL, SENDER_UL_LIB, SENDER_MAX };
    enum PAction { ACTION_NONE, ACTION_SET_FAN_LEFT, ACTION_SET_FAN_MID, ACTION_SET_FAN_RIGHT, ACTION_TOUCH_TRIGGERED, ACTION_SCAN_ENABLED, ACTION_MAX };

    Q_ENUM(Senders)
    Q_ENUM(PAction)

    struct SerialProtocolAction {
        Senders sender;
        bool isValid = false;
        PAction action = ACTION_NONE;
        double param = 0.0;
    };

    explicit SerialProtocol(QObject *parent = nullptr);

    static SerialProtocolAction parseMessage(Senders sender, QString data);

    static QString serializeAction(SerialProtocolAction action);

signals:

public slots:

};

#endif // PROTOCOL_H
