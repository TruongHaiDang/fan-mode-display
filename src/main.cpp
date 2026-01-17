#include "main.h"

#include <QSettings>
#include <QFile>
#include <QDebug>

namespace {
constexpr auto kFanBoostModeKey = "fan_boost_mode";

QString readSysfsValue(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    return QString::fromLatin1(f.readAll()).trimmed();
}

bool writeSysfsValue(const QString &path, const QString &value)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray data = value.trimmed().toLatin1();
    if (!data.endsWith('\n')) {
        data.append('\n');
    }

    return f.write(data) == data.size();
}

bool isValidModeValue(const QString &value)
{
    bool ok = false;
    const int mode = value.toInt(&ok);
    return ok && mode >= 0 && mode <= 2;
}

QString loadSavedFanBoostMode()
{
    QSettings settings;
    return settings.value(QLatin1String(kFanBoostModeKey)).toString().trimmed();
}

void saveFanBoostMode(const QString &value)
{
    QSettings settings;
    settings.setValue(QLatin1String(kFanBoostModeKey), value.trimmed());
}
} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("fan-mode-display"));
    QCoreApplication::setApplicationName(QStringLiteral("fan-mode-display"));

    MainPopup popup;

    QObject::connect(&popup, &MainPopup::dismissed, &app, &QCoreApplication::quit);
    
    const QString fanBoostPath = QStringLiteral("/sys/devices/platform/asus-nb-wmi/fan_boost_mode");
    const QString savedMode = loadSavedFanBoostMode();
    if (isValidModeValue(savedMode)) {
        const QString currentMode = readSysfsValue(fanBoostPath);
        if (currentMode != savedMode) {
            if (!writeSysfsValue(fanBoostPath, savedMode)) {
                qWarning() << "Failed to restore fan_boost_mode to" << savedMode;
            }
        }
    }

    FanBoostWatcher watcher(fanBoostPath);
    QObject::connect(&watcher, &FanBoostWatcher::fanBoostChanged, [&](const QString &raw) {
        if (isValidModeValue(raw)) {
            saveFanBoostMode(raw);
        }

        AsusTufFx705ge telemetry;
        const auto fanSpeeds = telemetry.readAllFanSpeedsRpm();
        const auto temps = telemetry.readAllTemperaturesCelsius();
        const auto modeLabel = QString::fromStdString(telemetry.readFanBoostModeLabel());

        QVector<int> fanValues;
        fanValues.reserve(static_cast<int>(fanSpeeds.size()));
        for (const auto value : fanSpeeds) {
            fanValues.append(value);
        }
        QVector<double> tempValues;
        tempValues.reserve(static_cast<int>(temps.size()));
        for (const auto value : temps) {
            tempValues.append(value);
        }
        
        popup.setFanData(
            modeLabel, 
            QStringLiteral("Live fan telemetry for ASUS TUF FX705GE."), 
            fanValues, 
            tempValues
        );
        popup.show();
        QTimer::singleShot(2000, &popup, &MainPopup::close);
    });
    
    return app.exec();
}
