#ifndef EYESAVERBUTTON_H
#define EYESAVERBUTTON_H

#include <QPushButton>
#include <QWidget>
#include <QSlider>

class EyeSaverButton : public QWidget {
    Q_OBJECT
public:
    explicit EyeSaverButton(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void toggleMode();
    void updateIntensity(int value);

private:
    bool m_active;
    QWidget *m_overlay;
    QPushButton *m_btn;
    QSlider *m_slider;
    int m_intensity;

    void updateOverlayStyle();
};

#endif // EYESAVERBUTTON_H
