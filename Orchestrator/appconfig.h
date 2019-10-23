#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QObject>
#include <QSettings>

class AppConfig : public QObject
{
    Q_OBJECT
public:
    explicit AppConfig(QObject *parent = nullptr);

    QVariant getValue(QString key, QVariant defaultValue);

private:
    QSettings *m_settings;


signals:

public slots:
};

#endif // APPCONFIG_H
