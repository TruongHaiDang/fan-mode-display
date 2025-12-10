#pragma once

#include <QObject>
#include <QFileSystemWatcher>
#include <QFile>
#include <QDebug>

class FanBoostWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FanBoostWatcher(const QString &path, QObject *parent = nullptr);

signals:
    void fanBoostChanged(const QString &value);

private slots:
    void onFileChanged();

private:
    QString readValue() const;

private:
    QString m_path;
    QFileSystemWatcher m_watcher;
};
