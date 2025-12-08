#include <QApplication>

#include "mainpopup.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainPopup popup;
    popup.setFanData(
        QStringLiteral("Performance Mode"),
        QStringLiteral("This mode prioritizes maximum cooling for intensive tasks, ensuring optimal performance."),
        QStringLiteral("5200 RPM"),
        QStringLiteral("75 C"));
    popup.show();

    return app.exec();
}
