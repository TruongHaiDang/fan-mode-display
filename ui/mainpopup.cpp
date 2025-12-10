#include "mainpopup.h"

#include <algorithm>
#include <cmath>

#include <QFont>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPalette>
#include <QProgressBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
QLabel *makeLabel(const QString &text, int pointSize, QFont::Weight weight, const QString &color)
{
    auto *label = new QLabel(text);
    QFont font = label->font();
    font.setPointSize(pointSize);
    font.setWeight(weight);
    label->setFont(font);
    label->setStyleSheet(QStringLiteral("color: %1;").arg(color));
    return label;
}

QColor temperatureColor(double celsius)
{
    if (celsius < 35.0) {
        return QColor("#1d8cf8");
    }
    if (celsius < 50.0) {
        return QColor("#2ecc71");
    }
    if (celsius < 65.0) {
        return QColor("#f39c12");
    }
    return QColor("#e74c3c");
}

QString temperatureText(int index, double celsius)
{
    return QStringLiteral("Temp %1: %2 \u00B0C").arg(index + 1).arg(QString::number(celsius, 'f', 1));
}
} // namespace

MainPopup::MainPopup(QWidget *parent)
    : QWidget(parent)
{
    // 1. Ẩn khung cửa sổ, hạn chế hiện trên taskbar
    setWindowFlags(windowFlags()
                   | Qt::FramelessWindowHint
                   | Qt::Tool);

    // 2. Cho phép nền trong suốt để chỉ thấy cái card
    setAttribute(Qt::WA_TranslucentBackground);
    
    setObjectName("popupWindow");
    setFixedSize(580, 680);
    // Nền widget gốc trong suốt, chỉ card có màu
    setStyleSheet(QStringLiteral(
        "#popupWindow { background-color: transparent; }"
    ));

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *card = new QWidget(this);
    card->setObjectName("card");
    card->setFixedWidth(520);
    card->setStyleSheet(QStringLiteral(
        "#card {"
        "  background-color: #1c2530;"
        "  border-radius: 18px;"
        "  border: 1px solid rgba(255,255,255,0.03);"
        "}"
        "QLabel { color: #e8edf4; }"
    ));
    card->setAttribute(Qt::WA_StyledBackground, true);

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(42);
    shadow->setColor(QColor(0, 0, 0, 140));
    shadow->setOffset(0, 20);
    card->setGraphicsEffect(shadow);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(22, 18, 22, 22);
    cardLayout->setSpacing(18);

    auto *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto *titleColumn = new QVBoxLayout();
    titleColumn->setContentsMargins(0, 0, 0, 0);
    titleColumn->setSpacing(4);
    auto *titleLabel = makeLabel(QStringLiteral("Fan Status"), 12, QFont::DemiBold, "#f1f5fb");
    auto *subtitleLabel = makeLabel(QStringLiteral("Current operating mode"), 9, QFont::Normal, "#8b95a5");
    titleColumn->addWidget(titleLabel);
    titleColumn->addWidget(subtitleLabel);

    auto *closeButton = new QToolButton();
    closeButton->setText(QStringLiteral("x"));
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFixedSize(28, 28);
    closeButton->setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  color: #c7ceda;"
        "  background-color: #2b3542;"
        "  border: none;"
        "  border-radius: 14px;"
        "  font-size: 12px;"
        "  font-weight: 600;"
        "}"
        "QToolButton:hover { background-color: #324054; }"
    ));
    connect(closeButton, &QToolButton::clicked, this, [this]() {
        emit dismissed();
        close();
    });

    headerLayout->addLayout(titleColumn);
    headerLayout->addStretch();
    headerLayout->addWidget(closeButton);
    cardLayout->addLayout(headerLayout);

    auto *iconLabel = new QLabel();
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setPixmap(buildIconPixmap(90));
    cardLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);

    m_modeLabel = makeLabel(QStringLiteral("Performance Mode"), 18, QFont::DemiBold, "#f7f9fc");
    m_modeLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(m_modeLabel);

    m_descriptionLabel = makeLabel(
        QStringLiteral("Live fan telemetry for ASUS TUF FX705GE."),
        10,
        QFont::Normal,
        "#9fb0c3");
    m_descriptionLabel->setAlignment(Qt::AlignCenter);
    m_descriptionLabel->setWordWrap(true);
    cardLayout->addWidget(m_descriptionLabel);

    auto *divider = new QFrame();
    divider->setFixedHeight(1);
    divider->setStyleSheet(QStringLiteral("background-color: #2b3542; border: none;"));
    cardLayout->addSpacing(6);
    cardLayout->addWidget(divider);

    auto *metricHeaders = new QHBoxLayout();
    metricHeaders->setContentsMargins(4, 0, 4, 0);
    metricHeaders->addWidget(makeLabel(QStringLiteral("Fan Speeds"), 10, QFont::DemiBold, "#9db0c6"));
    metricHeaders->addStretch();
    metricHeaders->addWidget(makeLabel(QStringLiteral("Temperatures"), 10, QFont::DemiBold, "#9db0c6"));
    cardLayout->addLayout(metricHeaders);

    auto *metricsLayout = new QHBoxLayout();
    metricsLayout->setContentsMargins(0, 0, 0, 0);
    metricsLayout->setSpacing(16);

    auto *fansColumn = new QVBoxLayout();
    fansColumn->setContentsMargins(0, 0, 0, 0);
    fansColumn->setSpacing(12);
    m_fanListLayout = new QVBoxLayout();
    m_fanListLayout->setContentsMargins(0, 0, 0, 0);
    m_fanListLayout->setSpacing(12);
    fansColumn->addLayout(m_fanListLayout);
    fansColumn->addStretch();
    metricsLayout->addLayout(fansColumn, 1);

    auto *tempsColumn = new QVBoxLayout();
    tempsColumn->setContentsMargins(0, 0, 0, 0);
    tempsColumn->setSpacing(8);
    m_tempGrid = new QGridLayout();
    m_tempGrid->setContentsMargins(0, 0, 0, 0);
    m_tempGrid->setHorizontalSpacing(10);
    m_tempGrid->setVerticalSpacing(10);
    m_tempGrid->setColumnStretch(0, 1);
    m_tempGrid->setColumnStretch(1, 1);
    tempsColumn->addLayout(m_tempGrid);
    tempsColumn->addStretch();
    metricsLayout->addLayout(tempsColumn, 1);

    cardLayout->addLayout(metricsLayout);

    rootLayout->addStretch();
    rootLayout->addWidget(card, 0, Qt::AlignHCenter);
    rootLayout->addStretch();
}

void MainPopup::setFanData(const QString &modeName,
                           const QString &description,
                           const QVector<int> &fanSpeedsRpm,
                           const QVector<double> &temperaturesC)
{
    m_modeLabel->setText(modeName);
    m_descriptionLabel->setText(description);
    renderFans(fanSpeedsRpm);
    renderTemperatures(temperaturesC);
}

void MainPopup::renderFans(const QVector<int> &fanSpeedsRpm)
{
    clearLayout(m_fanListLayout);

    if (fanSpeedsRpm.isEmpty()) {
        m_fanListLayout->addWidget(makeLabel(QStringLiteral("No fan sensors found"), 10, QFont::Normal, "#94a4b8"));
        return;
    }

    const int maxRpm = std::max(1, *std::max_element(fanSpeedsRpm.begin(), fanSpeedsRpm.end()));

    for (int i = 0; i < fanSpeedsRpm.size(); ++i) {
        const int rpm = fanSpeedsRpm.at(i);
        const int percent = std::clamp(static_cast<int>(std::round((rpm * 100.0) / maxRpm)), 0, 100);

        auto *card = new QWidget();
        card->setObjectName(QStringLiteral("fanCard"));
        card->setStyleSheet(QStringLiteral(
            "#fanCard {"
            "  background-color: #27303b;"
            "  border-radius: 12px;"
            "  border: 1px solid rgba(255,255,255,0.03);"
            "}"));

        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(12, 10, 12, 10);
        layout->setSpacing(8);

        auto *row = new QHBoxLayout();
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(6);

        auto *label = makeLabel(QStringLiteral("Fan %1: %2 RPM").arg(i + 1).arg(rpm), 10, QFont::DemiBold, "#e7edf5");
        row->addWidget(label);
        row->addStretch();
        auto *glyph = new QLabel();
        glyph->setFixedSize(20, 20);
        glyph->setStyleSheet(QStringLiteral(
            "QLabel { background: #324054; border-radius: 10px; }"));
        row->addWidget(glyph);

        layout->addLayout(row);

        auto *bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setValue(percent);
        bar->setFixedHeight(18);
        bar->setTextVisible(true);
        bar->setFormat(QStringLiteral("%1%").arg(percent));
        bar->setAlignment(Qt::AlignCenter);
        bar->setStyleSheet(QStringLiteral(
            "QProgressBar {"
            "  background-color: #1f2732;"
            "  border: none;"
            "  border-radius: 9px;"
            "  color: #dbe6f4;"
            "  font-size: 9pt;"
            "  font-weight: 700;"
            "}"
            "QProgressBar::chunk {"
            "  background-color: #2f8dff;"
            "  border-radius: 9px;"
            "}"
        ));
        layout->addWidget(bar);

        m_fanListLayout->addWidget(card);
    }
}

void MainPopup::renderTemperatures(const QVector<double> &temperaturesC)
{
    clearLayout(m_tempGrid);

    if (temperaturesC.isEmpty()) {
        m_tempGrid->addWidget(makeLabel(QStringLiteral("No temperature sensors found"), 10, QFont::Normal, "#94a4b8"), 0, 0);
        return;
    }

    const int columns = 2;
    for (int i = 0; i < temperaturesC.size(); ++i) {
        const double temp = temperaturesC.at(i);
        const QColor color = temperatureColor(temp);
        const QString bg = color.name(QColor::HexRgb);

        auto *chip = new QLabel(temperatureText(i, temp));
        chip->setObjectName(QStringLiteral("tempChip"));
        chip->setAlignment(Qt::AlignCenter);
        chip->setMinimumHeight(32);
        chip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        chip->setStyleSheet(QStringLiteral(
            "#tempChip {"
            "  padding: 8px 12px;"
            "  border-radius: 12px;"
            "  font-weight: 700;"
            "  font-size: 10pt;"
            "  color: #f8fbff;"
            "  background-color: %1;"
            "}").arg(bg));

        const int row = i / columns;
        const int col = i % columns;
        m_tempGrid->addWidget(chip, row, col);
    }
}

void MainPopup::clearLayout(QLayout *layout)
{
    if (!layout) {
        return;
    }

    while (auto *item = layout->takeAt(0)) {
        if (auto *childLayout = item->layout()) {
            clearLayout(childLayout);
            delete childLayout;
        }
        if (auto *widget = item->widget()) {
            delete widget;
        }
        delete item;
    }
}

QPixmap MainPopup::buildIconPixmap(int diameter) const
{
    QPixmap pixmap(diameter, diameter);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setBrush(QColor("#2c3644"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QRectF(0, 0, diameter, diameter));

    QPolygonF bolt;
    const qreal w = static_cast<qreal>(diameter);
    const qreal h = static_cast<qreal>(diameter);
    bolt << QPointF(w * 0.56, h * 0.18)
         << QPointF(w * 0.46, h * 0.46)
         << QPointF(w * 0.60, h * 0.46)
         << QPointF(w * 0.40, h * 0.84)
         << QPointF(w * 0.50, h * 0.56)
         << QPointF(w * 0.36, h * 0.56);

    painter.setBrush(QColor("#f2a33b"));
    painter.drawPolygon(bolt);

    return pixmap;
}
