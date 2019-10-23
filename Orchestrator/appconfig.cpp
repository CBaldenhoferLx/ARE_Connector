#include "appconfig.h"

AppConfig::AppConfig(QObject *parent) : QObject(parent)
{
    m_settings = new QSettings("config.ini", QSettings::IniFormat);
}

QVariant AppConfig::getValue(QString key, QVariant defaultValue) {
    if (!m_settings->contains(key)) {
        m_settings->setValue(key, defaultValue);
        m_settings->sync();
    }
    return m_settings->value(key, defaultValue);
}
