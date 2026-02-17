#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPoint>
#include <QPushButton>
#include <QList>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void applyRole(int roleIndex);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void logoutRequested();

private:
    QStackedWidget *stackedWidget;
    QPoint m_dragPosition;
    bool m_isResizing = false;
    bool m_isMoving = false;
    int m_edgeMargin = 8;
    
    QWidget* createTitleBar();
    Qt::Edges getEdge(const QPoint &pos);

    struct NavItem { QString title; QPushButton *btn; };
    QList<NavItem> navItems;
};

#endif
