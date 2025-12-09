#include "main.h"

#include <optional>

namespace {
QString formatFanSpeed(const std::optional<int> &rpm)
{
    if (!rpm) {
        return QStringLiteral("--");
    }
    return QStringLiteral("%1 RPM").arg(*rpm);
}

QString formatTemperature(const std::optional<double> &celsius)
{
    if (!celsius) {
        return QStringLiteral("--");
    }
    return QStringLiteral("%1 C").arg(QString::number(*celsius, 'f', 1));
}
} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AsusTufFx705ge telemetry;
    const auto fanSpeed = telemetry.readFanSpeedRpm();
    const auto averageTemp = telemetry.readAverageTemperatureCelsius();

    MainPopup popup;

    QObject::connect(&popup, &MainPopup::dismissed,
        &app, &QCoreApplication::quit);

    popup.setFanData(
        QStringLiteral("Performance Mode"),
        QStringLiteral("Live fan telemetry for ASUS TUF FX705GE."),
        formatFanSpeed(fanSpeed),
        formatTemperature(averageTemp));
    popup.show();

    return app.exec();
}
