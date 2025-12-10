#include "main.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainPopup popup;

    QObject::connect(&popup, &MainPopup::dismissed, &app, &QCoreApplication::quit);
    
    FanBoostWatcher watcher("/sys/devices/platform/asus-nb-wmi/fan_boost_mode");
    QObject::connect(&watcher, &FanBoostWatcher::fanBoostChanged, [&](const QString &raw) {
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
