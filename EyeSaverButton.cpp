#include "EyeSaverButton.h"
#include <QPainter>
#include <QApplication>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QLabel>

class EyeIconButton : public QPushButton {
public:
    EyeIconButton(QWidget *parent = nullptr) : QPushButton(parent) {
        setFixedSize(40, 40);
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setToolTip("Toggle Eye Saver Mode");
        setStyleSheet("QPushButton { border: none; background: transparent; outline: none; }");
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        bool active = isChecked();
        
        // Background Circle
        painter.setBrush(active ? QColor("#FF9800") : QColor("#333333"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect().adjusted(2, 2, -2, -2));

        // Eye Outline
        painter.setPen(QPen(Qt::white, 2));
        painter.setBrush(Qt::NoBrush);
        QRectF eyeRect(10, 14, 20, 12);
        painter.drawEllipse(eyeRect);
        
        // Pupil
        painter.setBrush(Qt::white);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(17, 17, 6, 6);
    }
};

EyeSaverButton::EyeSaverButton(QWidget *parent) 
    : QWidget(parent), m_active(false), m_overlay(nullptr), m_intensity(80) {
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    m_btn = new EyeIconButton();
    m_btn->setCursor(Qt::PointingHandCursor);
    
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(20, 150);
    m_slider->setValue(m_intensity);
    m_slider->setFixedWidth(100);
    m_slider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "  border: 1px solid #444;"
        "  height: 4px;"
        "  background: #222;"
        "  margin: 2px 0;"
        "  border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: #FF9800;"
        "  border: 1px solid #FF9800;"
        "  width: 14px;"
        "  height: 14px;"
        "  margin: -5px 0;"
        "  border-radius: 7px;"
        "}"
    );

    layout->addWidget(m_btn);
    layout->addWidget(m_slider);

    connect(m_btn, &QPushButton::clicked, this, &EyeSaverButton::toggleMode);
    connect(m_slider, &QSlider::valueChanged, this, &EyeSaverButton::updateIntensity);
}

void EyeSaverButton::toggleMode() {
    m_active = m_btn->isChecked();
    
    QWidget *window = this->window();
    if (!window) return;

    if (m_active) {
        if (!m_overlay) {
            m_overlay = new QWidget(window);
            m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
            m_overlay->setAttribute(Qt::WA_NoSystemBackground, false);
            m_overlay->setAttribute(Qt::WA_StyledBackground, true);
            window->installEventFilter(this);
        }
        updateOverlayStyle();
        m_overlay->move(0, 0);
        m_overlay->resize(window->size());
        m_overlay->raise();
        m_overlay->show();
    } else {
        if (m_overlay) m_overlay->hide();
    }
}

void EyeSaverButton::updateIntensity(int value) {
    m_intensity = value;
    if (m_active) {
        updateOverlayStyle();
    }
}

void EyeSaverButton::updateOverlayStyle() {
    if (m_overlay) {
        m_overlay->setStyleSheet(QString("background-color: rgba(255, 140, 0, %1); border: none;").arg(m_intensity));
    }
}

bool EyeSaverButton::eventFilter(QObject *watched, QEvent *event) {
    if (m_active && m_overlay && watched == this->window() && event->type() == QEvent::Resize) {
        m_overlay->resize(this->window()->size());
    }
    return QWidget::eventFilter(watched, event);
}
