#ifndef LOGINSYSTEM_H
#define LOGINSYSTEM_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QString>
#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QCheckBox>
#include <QAction>
#include <QIcon>
#include <QLabel>

class LoginSystem : public QMainWindow {
    Q_OBJECT
public:
    explicit LoginSystem(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    void loginSuccessful(int roleIndex);

private:
    QStackedWidget *stack;
    QPoint m_dragPosition;
    
    // Store states for account activation
    QString currentActivationName;
    QString currentActivationCIN;
    QString generatedActivationCode;
    
    // UI Helpers
    QWidget* createTitleBar();
    QWidget* createModernLoginWidget();
    QWidget* createActivationSetupWidget();
    QWidget* createActivationVerifyWidget();
    
    // API Call
    void sendActivationEmail(const QString& toEmail, const QString& code);
    
    // Slide Animation
    void switchToPage(int index);
};

#endif // LOGINSYSTEM_H
