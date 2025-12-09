#pragma once

#include <QPixmap>
#include <QWidget>
#include <QFont>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include "asus_tuf_fx705ge.h"

class QLabel;
class QPushButton;

class MainPopup : public QWidget
{
    Q_OBJECT

public:
    explicit MainPopup(QWidget *parent = nullptr);

    void setFanData(const QString &modeName,
                    const QString &description,
                    const QString &fanSpeed,
                    const QString &cpuTemperature);

signals:
    void dismissed();

private:
    QLabel *m_modeLabel{};
    QLabel *m_descriptionLabel{};
    QLabel *m_fanSpeedValue{};
    QLabel *m_cpuTempValue{};

    QPixmap buildIconPixmap(int diameter) const;
};
