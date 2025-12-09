#include "mainpopup.h"

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
    setFixedSize(450, 450);
    // Nền widget gốc trong suốt, chỉ card có màu
    setStyleSheet(QStringLiteral(
        "#popupWindow { background-color: transparent; }"
    ));

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *card = new QWidget(this);
    card->setObjectName("card");
    card->setFixedWidth(380);
    card->setStyleSheet(QStringLiteral(
        "#card {"
        "  background-color: #1c2530;"
        "  border-radius: 12px;"
        "}"
        "QLabel { color: #e8edf4; }"
    ));
    card->setAttribute(Qt::WA_StyledBackground, true);

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(32);
    shadow->setColor(QColor(0, 0, 0, 140));
    shadow->setOffset(0, 18);
    card->setGraphicsEffect(shadow);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 16, 20, 20);
    cardLayout->setSpacing(16);

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
    iconLabel->setPixmap(buildIconPixmap(84));
    cardLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);

    m_modeLabel = makeLabel(QStringLiteral("Performance Mode"), 16, QFont::DemiBold, "#f7f9fc");
    m_modeLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(m_modeLabel);

    m_descriptionLabel = makeLabel(
        QStringLiteral("This mode prioritizes maximum cooling for intensive tasks, ensuring optimal performance."),
        10,
        QFont::Normal,
        "#99a5b7");
    m_descriptionLabel->setAlignment(Qt::AlignCenter);
    m_descriptionLabel->setWordWrap(true);
    cardLayout->addWidget(m_descriptionLabel);

    auto *divider = new QFrame();
    divider->setFixedHeight(1);
    divider->setStyleSheet(QStringLiteral("background-color: #2b3542; border: none;"));
    cardLayout->addSpacing(4);
    cardLayout->addWidget(divider);

    auto metricColumn = [](const QString &title, QLabel **valueHolder) {
        auto *container = new QWidget();
        auto *layout = new QVBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);

        auto *titleLabel = makeLabel(title, 9, QFont::Normal, "#8b95a5");
        auto *valueLabel = makeLabel(QStringLiteral("--"), 10, QFont::DemiBold, "#f2f6fb");

        layout->addWidget(titleLabel);
        layout->addWidget(valueLabel);

        *valueHolder = valueLabel;
        return container;
    };

    auto *metricsLayout = new QHBoxLayout();
    metricsLayout->setContentsMargins(0, 0, 0, 0);
    metricsLayout->setSpacing(24);
    metricsLayout->addWidget(metricColumn(QStringLiteral("Fan Speed"), &m_fanSpeedValue), 0, Qt::AlignLeft);
    metricsLayout->addStretch();
    metricsLayout->addWidget(metricColumn(QStringLiteral("CPU Temperature"), &m_cpuTempValue), 0, Qt::AlignRight);
    cardLayout->addLayout(metricsLayout);

    auto *settingsButton = new QPushButton(QStringLiteral("Change Mode"));
    settingsButton->setCursor(Qt::PointingHandCursor);
    settingsButton->setFixedHeight(42);
    settingsButton->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: #2f8dff;"
        "  color: #f7fbff;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 10.5pt;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #3e99ff; }"
        "QPushButton:pressed { background-color: #2b82ea; }"
    ));

    cardLayout->addSpacing(4);
    cardLayout->addWidget(settingsButton);

    rootLayout->addStretch();
    rootLayout->addWidget(card, 0, Qt::AlignHCenter);
    rootLayout->addStretch();
}

void MainPopup::setFanData(const QString &modeName,
                           const QString &description,
                           const QString &fanSpeed,
                           const QString &cpuTemperature)
{
    m_modeLabel->setText(modeName);
    m_descriptionLabel->setText(description);
    m_fanSpeedValue->setText(fanSpeed);
    m_cpuTempValue->setText(cpuTemperature);
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
