#include "fanboostwatcher.h"

FanBoostWatcher::FanBoostWatcher(const QString &path, QObject *parent)
    : QObject(parent)
    , m_path(path)
{
    m_watcher.addPath(path);

    connect(&m_watcher, &QFileSystemWatcher::fileChanged,
            this, &FanBoostWatcher::onFileChanged);
}

void FanBoostWatcher::onFileChanged()
{
    QString value = readValue();

    if (!value.isEmpty())
        emit fanBoostChanged(value);

    // Một số filesystem sysfs cần add lại
    m_watcher.addPath(m_path);
}

QString FanBoostWatcher::readValue() const
{
    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    return QString::fromLatin1(f.readAll()).trimmed();
}
