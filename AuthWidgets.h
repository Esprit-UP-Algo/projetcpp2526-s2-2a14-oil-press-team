#ifndef AUTHWIDGETS_H
#define AUTHWIDGETS_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPoint>

class AuthWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit AuthWindow(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    void loginSuccessful(int roleIndex);

private:
    QStackedWidget *stack;
    QPoint m_dragPosition;
    QWidget* createTitleBar();
    QWidget* createLoginWidget();
    QWidget* createRegisterWidget();
};

#endif // AUTHWIDGETS_H
