#include "main.h"

#include <QVector>
#include <vector>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

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

    MainPopup popup;

    QObject::connect(&popup, &MainPopup::dismissed,
        &app, &QCoreApplication::quit);

    popup.setFanData(
        modeLabel,
        QStringLiteral("Live fan telemetry for ASUS TUF FX705GE."),
        fanValues,
        tempValues);
    popup.show();

    return app.exec();
}
