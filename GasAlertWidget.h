#ifndef GASALERTWIDGET_H
#define GASALERTWIDGET_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>

class GasAlertWidget : public QDialog {
    Q_OBJECT
public:
    enum AlertType { Gas, Smoke };

    explicit GasAlertWidget(int machineId, AlertType type = Gas, QWidget *parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(450, 320);

        QWidget *container = new QWidget(this);
        container->setObjectName("alertContainer");
        
        QString bgColor = (type == Gas) ? "#d32f2f" : "#424242"; // Red for Gas, Dark Gray/Orange for Smoke
        QString secondColor = (type == Gas) ? "#b71c1c" : "#212121";
        QString accentColor = (type == Gas) ? "#ff5252" : "#ff9800";

        container->setStyleSheet(
            QString("#alertContainer { "
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2); "
            "  border-radius: 20px; "
            "  border: 3px solid %3; "
            "}").arg(bgColor, secondColor, accentColor)
        );

        QVBoxLayout *layout = new QVBoxLayout(container);
        layout->setContentsMargins(30, 30, 30, 30);
        layout->setSpacing(15);

        QLabel *iconLabel = new QLabel(type == Gas ? "⚠️" : "🔥");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("font-size: 70px; color: white;");
        layout->addWidget(iconLabel);

        QLabel *titleLabel = new QLabel(type == Gas ? "GAS LEAK DETECTED!" : "SMOKE DETECTED!");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 26px; font-weight: 800; color: white; letter-spacing: 1px;");
        layout->addWidget(titleLabel);

        QLabel *descLabel = new QLabel(QString("Critical levels detected on Machine #%1.\nEvacuate area and take immediate action.").arg(machineId));
        descLabel->setAlignment(Qt::AlignCenter);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet("font-size: 15px; font-weight: 500; color: #ffcdd2;");
        layout->addWidget(descLabel);

        QPushButton *btnDismiss = new QPushButton("ACKNOWLEDGE HAZARD");
        btnDismiss->setCursor(Qt::PointingHandCursor);
        btnDismiss->setFixedHeight(50);
        btnDismiss->setStyleSheet(
            "QPushButton { "
            "  background-color: white; color: #d32f2f; "
            "  border-radius: 12px; font-weight: 700; font-size: 14px; "
            "  border: none; margin-top: 10px;"
            "} "
            "QPushButton:hover { background-color: #ffebee; } "
            "QPushButton:pressed { background-color: #ffcdd2; }"
        );
        layout->addWidget(btnDismiss);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(container);

        connect(btnDismiss, &QPushButton::clicked, this, &QDialog::accept);

        // --- Animations ---
        // 1. Fade and Scale In
        setWindowOpacity(0);
        QPropertyAnimation *fadeAnim = new QPropertyAnimation(this, "windowOpacity");
        fadeAnim->setDuration(500);
        fadeAnim->setStartValue(0);
        fadeAnim->setEndValue(1);
        fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
        fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);

        // 2. Pulse effect on the icon
        QTimer *pulseTimer = new QTimer(this);
        connect(pulseTimer, &QTimer::timeout, [iconLabel]() {
            static bool bigger = true;
            iconLabel->setStyleSheet(bigger ? "font-size: 65px; color: white;" : "font-size: 60px; color: white;");
            bigger = !bigger;
        });
        pulseTimer->start(400);
    }
};

#endif // GASALERTWIDGET_H
