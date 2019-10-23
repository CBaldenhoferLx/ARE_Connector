#ifndef KEYFILTER_H
#define KEYFILTER_H

#include <QObject>

#include <QThread>

#include <iostream>
#include <windows.h>


class KeyFilter : public QThread
{
    Q_OBJECT
public:
    explicit KeyFilter(QObject *parent = nullptr);

    void run();

    WORD get_keypress(DWORD& control_key);

signals:
    void keyPressed(unsigned long key);

public slots:
};

#endif // KEYFILTER_H
