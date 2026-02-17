#include "AuthWidgets.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QScreen>
#include <QGuiApplication>
#include <QPropertyAnimation> 
#include <QEasingCurve>
#include <QMouseEvent>

// Dark Theme Styles
static QString getAuthDarkStyle() {
    return "QWidget { font-family: 'Segoe UI', sans-serif; }"
           "QLabel { color: #ecf0f1; font-weight: 500; background: transparent; }"
           "QLineEdit { background-color: rgba(0, 0, 0, 0.1); border: none; border-bottom: 2px solid #7f8c8d; border-radius: 4px; color: #ecf0f1; padding: 8px; font-size: 14px; margin-bottom: 12px; }" 
           "QLineEdit:focus { background-color: rgba(0, 0, 0, 0.2); border-bottom: 2px solid #2ecc71; }"
           "QLineEdit::placeholder { color: #95a5a6; }";
}

static QString getPrimaryBtnStyle() {
    return "QPushButton { background-color: #2ecc71; color: white; border-radius: 20px; padding: 12px; font-size: 14px; font-weight: bold; border: none; }"
           "QPushButton:hover { background-color: #27ae60; }"
           "QPushButton:pressed { background-color: #2ecc71; }";
}

static QString getSecondaryBtnStyle() {
    return "QPushButton { background-color: transparent; color: #ecf0f1; border-radius: 20px; padding: 12px; font-size: 14px; font-weight: bold; border: 2px solid #95a5a6; }"
           "QPushButton:hover { border-color: #ecf0f1; background-color: rgba(255, 255, 255, 0.05); }"
           "QPushButton:pressed { background-color: rgba(255, 255, 255, 0.1); }";
}

AuthWindow::AuthWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("oil press log in");
    setFixedSize(360, 520);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window); 
    setAttribute(Qt::WA_TranslucentBackground); // Enable translucency for rounded corners

    // Main Container for rounded edges
    QWidget *central = new QWidget(this);
    central->setObjectName("authContainer");
    central->setStyleSheet("#authContainer { background-color: #000000; border-radius: 20px; }");
    setCentralWidget(central);

    QVBoxLayout *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    stack = new QStackedWidget(this);
    rootLayout->addWidget(stack);

    stack->addWidget(createLoginWidget());
    stack->addWidget(createRegisterWidget());
}

void AuthWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void AuthWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

QWidget* AuthWindow::createTitleBar() {
    QWidget *bar = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(10, 10, 10, 0);
    layout->setSpacing(8);

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(24, 24);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border-radius: 12px; font-size: 12px; font-weight: bold; border: none; } QPushButton:hover { background-color: #c0392b; }");
    
    QPushButton *minBtn = new QPushButton("−"); // Using a proper dash
    minBtn->setFixedSize(24, 24);
    minBtn->setCursor(Qt::PointingHandCursor);
    minBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border-radius: 12px; font-size: 14px; font-weight: bold; border: none; } QPushButton:hover { background-color: #7f8c8d; }");

    layout->addWidget(closeBtn);
    layout->addWidget(minBtn);
    layout->addStretch();

    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);

    return bar;
}

QWidget* AuthWindow::createLoginWidget() {
    QWidget *widget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 20);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createTitleBar());

    QVBoxLayout *content = new QVBoxLayout();
    content->setContentsMargins(30, 20, 30, 20);
    content->setSpacing(10);

    // Logo
    QLabel *logoLabel = new QLabel();
    logoLabel->setAlignment(Qt::AlignCenter);
    QPixmap logoPixmap(":/logo.png");
    if (!logoPixmap.isNull()) {
        logoLabel->setPixmap(logoPixmap.scaledToHeight(100, Qt::SmoothTransformation));
    } else {
        logoLabel->setText("🔒"); 
        logoLabel->setStyleSheet("font-size: 60px; color: #2ecc71;");
    }
    content->addWidget(logoLabel);
    content->addSpacing(20);

    // Inputs
    QLineEdit *usernameInput = new QLineEdit();
    usernameInput->setPlaceholderText("User name");
    content->addWidget(usernameInput);

    QLineEdit *passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    content->addWidget(passwordInput);

    content->addSpacing(20);

    // Buttons
    QPushButton *loginBtn = new QPushButton("Log In");
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(getPrimaryBtnStyle());
    content->addWidget(loginBtn);

    QPushButton *signupBtn = new QPushButton("Sign Up");
    signupBtn->setCursor(Qt::PointingHandCursor);
    signupBtn->setStyleSheet(getSecondaryBtnStyle());
    content->addWidget(signupBtn);

    QPushButton *forgotLink = new QPushButton("Forgot password?");
    forgotLink->setStyleSheet("QPushButton { border: none; background: transparent; color: #bdc3c7; font-size: 12px; margin-top: 10px; } QPushButton:hover { color: #ecf0f1; text-decoration: underline; }");
    content->addWidget(forgotLink);

    content->addStretch();
    mainLayout->addLayout(content);

    connect(loginBtn, &QPushButton::clicked, this, &AuthWindow::loginSuccessful);
    connect(signupBtn, &QPushButton::clicked, [this]() { stack->setCurrentIndex(1); });
    connect(forgotLink, &QPushButton::clicked, []() {
        QMessageBox::information(nullptr, "Info", "Contact admin for reset.");
    });

    return widget;
}

QWidget* AuthWindow::createRegisterWidget() {
    QWidget *widget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 20);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createTitleBar());

    QVBoxLayout *content = new QVBoxLayout();
    content->setContentsMargins(30, 20, 30, 20);
    content->setSpacing(10);

    QLabel *title = new QLabel("Create Account");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #ecf0f1; margin-bottom: 20px;");
    content->addWidget(title);

    QLineEdit *fullInput = new QLineEdit(); fullInput->setPlaceholderText("Full Name"); content->addWidget(fullInput);
    QLineEdit *userInput = new QLineEdit(); userInput->setPlaceholderText("Username"); content->addWidget(userInput);
    QLineEdit *passInput = new QLineEdit(); passInput->setPlaceholderText("Password"); passInput->setEchoMode(QLineEdit::Password); content->addWidget(passInput);
    QLineEdit *emailInput = new QLineEdit(); emailInput->setPlaceholderText("Email"); content->addWidget(emailInput);

    content->addSpacing(20);
    
    QPushButton *createBtn = new QPushButton("Sign Up");
    createBtn->setStyleSheet(getPrimaryBtnStyle());
    content->addWidget(createBtn);

    QPushButton *backBtn = new QPushButton("Back to Login");
    backBtn->setStyleSheet(getSecondaryBtnStyle());
    content->addWidget(backBtn);

    content->addStretch();
    mainLayout->addLayout(content);

    connect(createBtn, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Success", "Account created!");
        stack->setCurrentIndex(0);
    });
    connect(backBtn, &QPushButton::clicked, [this]() { stack->setCurrentIndex(0); });

    return widget;
}
