#pragma once

#include <QPixmap>
#include <QVector>
#include <QWidget>

class QLabel;
class QVBoxLayout;
class QGridLayout;

class MainPopup : public QWidget
{
    Q_OBJECT

public:
    explicit MainPopup(QWidget *parent = nullptr);

    void setFanData(const QString &modeName,
                    const QString &description,
                    const QVector<int> &fanSpeedsRpm,
                    const QVector<double> &temperaturesC);

signals:
    void dismissed();

private:
    QLabel *m_modeLabel{};
    QLabel *m_descriptionLabel{};
    QVBoxLayout *m_fanListLayout{};
    QGridLayout *m_tempGrid{};

    void renderFans(const QVector<int> &fanSpeedsRpm);
    void renderTemperatures(const QVector<double> &temperaturesC);
    static void clearLayout(QLayout *layout);
    QPixmap buildIconPixmap(int diameter) const;
};
