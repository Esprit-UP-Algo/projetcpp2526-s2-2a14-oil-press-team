#include "LoginSystem.h"
#include "AuthWidgets.h"
#include "smtp.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QRandomGenerator>
#include <QSqlRecord>
#include <QThread>
#include <QTimer>

// Dark Theme Styles
static QString getAuthDarkStyle() {
    return "QWidget { font-family: 'Segoe UI', sans-serif; background-color: #000000; }"
           "QLabel { color: #ecf0f1; font-weight: 500; background: transparent; }"
           "QLineEdit { background-color: rgba(0, 0, 0, 0.1); border: none; border-bottom: 2px solid #7f8c8d; border-radius: 4px; color: #ecf0f1; padding: 10px; font-size: 14px; margin-bottom: 12px; }" 
           "QLineEdit:focus { background-color: rgba(0, 0, 0, 0.2); border-bottom: 2px solid #2ecc71; }"
           "QLineEdit::placeholder { color: #95a5a6; }";
}

static QString getPrimaryBtnStyle() {
    return "QPushButton { background-color: #2ecc71; color: white; border-radius: 20px; padding: 12px; font-size: 15px; font-weight: bold; border: none; }"
           "QPushButton:hover { background-color: #27ae60; }"
           "QPushButton:pressed { background-color: #2ecc71; }";
}

static QString getSecondaryBtnStyle() {
    return "QPushButton { background-color: transparent; color: #ecf0f1; border-radius: 20px; padding: 12px; font-size: 14px; font-weight: bold; border: 2px solid #95a5a6; }"
           "QPushButton:hover { border-color: #ecf0f1; background-color: rgba(255, 255, 255, 0.05); }"
           "QPushButton:pressed { background-color: rgba(255, 255, 255, 0.1); }";
}

static QString getDebugBtnStyle() {
    return "QPushButton { background-color: transparent; color: #e74c3c; font-size: 12px; border: none; text-decoration: underline; margin-top: 10px; } "
           "QPushButton:hover { color: #c0392b; }";
}

static void showStyledMessage(QWidget* parent, const QString& title, const QString& text, bool isError = false) {
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    if(isError) msgBox.setIcon(QMessageBox::Warning);
    else msgBox.setIcon(QMessageBox::Information);
    msgBox.setStyleSheet("QMessageBox { background-color: #000000; border: 1px solid #2ecc71; border-radius: 8px; }"
                         "QLabel { color: #ecf0f1; font-family: 'Segoe UI'; font-size: 14px; }"
                         "QPushButton { background-color: #2ecc71; color: white; border-radius: 4px; padding: 8px 16px; font-weight: bold; }");
    msgBox.exec();
}

LoginSystem::LoginSystem(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Oil Press System Login");
    setFixedSize(380, 550);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window); 
    setAttribute(Qt::WA_TranslucentBackground);

    QWidget *central = new QWidget(this);
    central->setObjectName("authContainer");
    central->setStyleSheet("#authContainer { background-color: #000000; border-radius: 20px; }");
    setCentralWidget(central);
    central->setStyleSheet(central->styleSheet() + getAuthDarkStyle());

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 150)); // Deep premium dark shadow
    shadow->setOffset(0, 5);
    central->setGraphicsEffect(shadow);

    QVBoxLayout *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    stack = new QStackedWidget(this);
    rootLayout->addWidget(stack);

    stack->addWidget(createModernLoginWidget());
    stack->addWidget(createActivationSetupWidget());
    stack->addWidget(createActivationVerifyWidget());
}

void LoginSystem::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void LoginSystem::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

QWidget* LoginSystem::createTitleBar() {
    QWidget *bar = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(15, 15, 15, 0);
    layout->setSpacing(8);

    QPushButton *closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(24, 24);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border-radius: 12px; font-weight: bold; border: none; } QPushButton:hover { background-color: #c0392b; }");
    
    QPushButton *minBtn = new QPushButton("−");
    minBtn->setFixedSize(24, 24);
    minBtn->setCursor(Qt::PointingHandCursor);
    minBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border-radius: 12px; font-weight: bold; border: none; } QPushButton:hover { background-color: #7f8c8d; }");

    layout->addStretch();
    layout->addWidget(minBtn);
    layout->addWidget(closeBtn);

    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);

    return bar;
}

QWidget* LoginSystem::createModernLoginWidget() {
    QWidget *widget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 20);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(createTitleBar());

    QVBoxLayout *content = new QVBoxLayout();
    content->setContentsMargins(35, 20, 35, 20);
    content->setSpacing(10);

    // Logo
    QLabel *logoLabel = new QLabel();
    logoLabel->setAlignment(Qt::AlignCenter);
    QPixmap logoPixmap(":/logo.png");
    if (!logoPixmap.isNull()) {
        logoLabel->setPixmap(logoPixmap.scaledToHeight(100, Qt::SmoothTransformation));
    } else {
        logoLabel->setText("🔒"); 
        logoLabel->setStyleSheet("font-size: 60px; color: #2ecc71; margin-bottom: 20px;");
    }
    content->addWidget(logoLabel);

    QLabel *subtitle = new QLabel("Welcome Back");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 20px; font-weight: bold; margin-bottom: 5px;");
    content->addWidget(subtitle);

    QHBoxLayout *connLayout = new QHBoxLayout();
    QLabel *dot = new QLabel("●");
    dot->setStyleSheet("color: #2ecc71; font-size: 16px; margin: 0; padding: 0;");
    QLabel *connTxt = new QLabel("Connected");
    connTxt->setStyleSheet("color: #95a5a6; font-size: 12px; margin: 0; padding: 0; font-weight: bold;");
    connLayout->addStretch();
    connLayout->addWidget(dot);
    connLayout->addWidget(connTxt);
    connLayout->addStretch();
    content->addLayout(connLayout);
    content->addSpacing(10);

    QLineEdit *nameInput = new QLineEdit();
    nameInput->setPlaceholderText("Full Name");
    content->addWidget(nameInput);

    QLineEdit *passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    
    QAction *eyeAction = passwordInput->addAction(QIcon(":/eye_open.png"), QLineEdit::TrailingPosition);
    // Fallback text if icon missing
    if (eyeAction->icon().isNull()) eyeAction->setText("👁");

    connect(eyeAction, &QAction::triggered, [passwordInput, eyeAction]() {
        if(passwordInput->echoMode() == QLineEdit::Password) {
            passwordInput->setEchoMode(QLineEdit::Normal);
            eyeAction->setText("🙈");
        } else {
            passwordInput->setEchoMode(QLineEdit::Password);
            eyeAction->setText("👁");
        }
    });

    content->addWidget(passwordInput);

    QCheckBox *rememberMe = new QCheckBox("Remember my name");
    rememberMe->setStyleSheet("QCheckBox { color: #95a5a6; font-size: 13px; margin-bottom: 10px; } QCheckBox::indicator { width: 14px; height: 14px; border-radius: 3px; border: 1px solid #7f8c8d; } QCheckBox::indicator:checked { background-color: #2ecc71; border: 1px solid #2ecc71; }");
    content->addWidget(rememberMe);

    // Initial QSettings load
    QSettings settings("OilPress", "App");
    if(settings.contains("lastUsername")) {
        nameInput->setText(settings.value("lastUsername").toString());
        rememberMe->setChecked(true);
    }

    content->addSpacing(5);

    QPushButton *loginBtn = new QPushButton("Access Account");
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(getPrimaryBtnStyle());
    content->addWidget(loginBtn);

    QPushButton *activateBtn = new QPushButton("Activate My Account");
    activateBtn->setCursor(Qt::PointingHandCursor);
    activateBtn->setStyleSheet(getSecondaryBtnStyle());
    content->addWidget(activateBtn);

    QPushButton *debugBtn = new QPushButton("🛠 Fallback Debug Login");
    debugBtn->setCursor(Qt::PointingHandCursor);
    debugBtn->setStyleSheet(getDebugBtnStyle());
    content->addWidget(debugBtn);

    content->addStretch();
    mainLayout->addLayout(content);

    // -- Connect Login
    connect(loginBtn, &QPushButton::clicked, [this, nameInput, passwordInput, loginBtn, rememberMe, activateBtn]() {
        QString name = nameInput->text().trimmed();
        QString pass = passwordInput->text().trimmed();
        
        if(name.isEmpty() || pass.isEmpty()) {
            showStyledMessage(this, "Error", "Please fill in all fields.", true);
            return;
        }

        // Comprehensive loading state
        loginBtn->setText("Authenticating...");
        loginBtn->setEnabled(false);
        nameInput->setEnabled(false);
        passwordInput->setEnabled(false);
        rememberMe->setEnabled(false);
        activateBtn->setEnabled(false);

        QTimer::singleShot(600, this, [=]() {
            loginBtn->setText("Access Account");
            loginBtn->setEnabled(true);
            nameInput->setEnabled(true);
            passwordInput->setEnabled(true);
            rememberMe->setEnabled(true);
            activateBtn->setEnabled(true);

            QSqlQuery q;
            q.prepare("SELECT ROLE FROM PERSONNEL WHERE NOM_PERSONNEL = :name AND MOT_DE_PASSE = :pass");
            q.bindValue(":name", name);
            q.bindValue(":pass", pass);

            if(q.exec() && q.next()) {
                QString role = q.value(0).toString();
                // Map string role to roleIndex if needed, matching AuthWidgets slider order
                int roleIndex = -1;
                if(role == "Order Manager" || role == "Order Management" || role == "Order" || role == "Command") roleIndex = 1;
                else if(role == "Financial Manager" || role == "Financial Management" || role == "Financial" || role == "Finance") roleIndex = 2;
                else if(role == "Inventory Manager" || role == "Stock Management" || role == "Stock" || role == "Inventory") roleIndex = 3;
                else if(role == "Maintenance Manager" || role == "Maintenance Management" || role == "Maintenance") roleIndex = 4;
                else if(role == "Product Manager" || role == "Product Management" || role == "Product") roleIndex = 5;
                else if(role == "Personnel Manager" || role == "Personnel Management" || role == "Personnel" || role == "HR") roleIndex = 6;
                else if(role == "Super Admin" || role == "Admin") roleIndex = 0;
                
                if (roleIndex == -1) {
                    showStyledMessage(this, "Access Denied", "Your role is unrecognized by the system.\nGot: '" + role + "'", true);
                    return;
                }

                if(rememberMe->isChecked()) {
                    QSettings settings("OilPress", "App");
                    settings.setValue("lastUsername", name);
                } else {
                    QSettings settings("OilPress", "App");
                    settings.remove("lastUsername");
                }

                emit loginSuccessful(roleIndex);
            } else {
                // Failed login
                showStyledMessage(this, "Access Denied", "Incorrect Name or Password.", true);
            }
        });
    });

    // -- Connect Activation
    connect(activateBtn, &QPushButton::clicked, [this]() {
        switchToPage(1); // Activation Setup
    });

    // -- Connect Debug
    connect(debugBtn, &QPushButton::clicked, [this]() {
        AuthWindow *oldAuth = new AuthWindow();
        connect(oldAuth, &AuthWindow::loginSuccessful, [this, oldAuth](int r) {
            oldAuth->hide();
            emit loginSuccessful(r);
            oldAuth->deleteLater();
        });
        oldAuth->show();
        this->hide();
    });

    return widget;
}

QWidget* LoginSystem::createActivationSetupWidget() {
    QWidget *widget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 20);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(createTitleBar());

    QVBoxLayout *content = new QVBoxLayout();
    content->setContentsMargins(35, 10, 35, 20);
    content->setSpacing(10);

    QLabel *title = new QLabel("Account Activation");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #2ecc71; margin-bottom: 5px;");
    content->addWidget(title);
    
    QLabel *sub = new QLabel("Enter your employee details to receive a confirmation code.");
    sub->setWordWrap(true);
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 13px; color: #8892b0; margin-bottom: 15px;");
    content->addWidget(sub);

    QLineEdit *nameInput = new QLineEdit(); 
    nameInput->setPlaceholderText("Full Name"); 
    content->addWidget(nameInput);

    QLineEdit *cinInput = new QLineEdit(); 
    cinInput->setPlaceholderText("ID Personnel (CIN)"); 
    content->addWidget(cinInput);

    QLineEdit *emailInput = new QLineEdit(); 
    emailInput->setPlaceholderText("Email Address"); 
    content->addWidget(emailInput);

    content->addSpacing(15);
    
    // Auto-fetch CIN when name is entered
    connect(nameInput, &QLineEdit::textChanged, [this, nameInput, cinInput]() {
        QString name = nameInput->text().trimmed();
        if (name.length() > 2) {
            QSqlQuery q;
            q.prepare("SELECT ID_PERSONNEL FROM PERSONNEL WHERE NOM_PERSONNEL = :name");
            q.bindValue(":name", name);
            if (q.exec() && q.next()) {
                cinInput->setText(q.value(0).toString());
            } else {
                cinInput->clear();
            }
        }
    });

    QPushButton *sendBtn = new QPushButton("Send Code");
    sendBtn->setStyleSheet(getPrimaryBtnStyle());
    sendBtn->setCursor(Qt::PointingHandCursor);
    content->addWidget(sendBtn);

    QPushButton *backBtn = new QPushButton("Back to Login");
    backBtn->setStyleSheet(getSecondaryBtnStyle());
    backBtn->setCursor(Qt::PointingHandCursor);
    content->addWidget(backBtn);

    content->addStretch();
    mainLayout->addLayout(content);

    // -- Activation Logic
    connect(sendBtn, &QPushButton::clicked, [this, nameInput, cinInput, emailInput, sendBtn]() {
        QString name = nameInput->text().trimmed();
        QString cin = cinInput->text().trimmed();
        QString email = emailInput->text().trimmed();

        if (name.isEmpty() || cin.isEmpty() || email.isEmpty()) {
            showStyledMessage(this, "Validation", "Please fill in all fields.", true);
            return;
        }

        // Check if employee exists
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT EMAIL FROM PERSONNEL WHERE NOM_PERSONNEL = :name AND ID_PERSONNEL = :cin");
        checkQuery.bindValue(":name", name);
        checkQuery.bindValue(":cin", cin.toInt());

        if (checkQuery.exec() && checkQuery.next()) {
            QString dbEmail = checkQuery.value(0).toString();
            
            // If email is empty in db, update it
            if (dbEmail.isEmpty()) {
                QSqlQuery updt;
                updt.prepare("UPDATE PERSONNEL SET EMAIL = :email WHERE ID_PERSONNEL = :cin");
                updt.bindValue(":email", email);
                updt.bindValue(":cin", cin.toInt());
                if (!updt.exec()) {
                    showStyledMessage(this, "Database Error", "Failed to register email: " + updt.lastError().text(), true);
                    return;
                }
            } else if (dbEmail.toLower() != email.toLower()) {
                // If it exists but doesn't match
                showStyledMessage(this, "Email Mismatch", "The email you entered does not match our records.", true);
                return;
            }

            // Generate 6 digit code
            int codeInt = QRandomGenerator::global()->bounded(100000, 1000000);
            generatedActivationCode = QString::number(codeInt);
            currentActivationName = name;
            currentActivationCIN = cin;

            sendBtn->setText("Sending...");
            sendBtn->setEnabled(false);
            
            QTimer::singleShot(200, this, [=]() {
                sendBtn->setText("Send Code");
                sendBtn->setEnabled(true);
                
                sendActivationEmail(email, generatedActivationCode);

                showStyledMessage(this, "Success", "A 6-digit code has been sent to " + email);
                
                // Clear fields
                nameInput->clear();
                cinInput->clear();
                emailInput->clear();

                switchToPage(2); // Go to verification view
            });
        } else {
            showStyledMessage(this, "Not Found", "We couldn't find an employee with that Name and CIN.", true);
        }
    });

    connect(backBtn, &QPushButton::clicked, [this]() { switchToPage(0); });
    return widget;
}

QWidget* LoginSystem::createActivationVerifyWidget() {
    QWidget *widget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 20);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(createTitleBar());

    QVBoxLayout *content = new QVBoxLayout();
    content->setContentsMargins(35, 10, 35, 20);
    content->setSpacing(10);

    QLabel *title = new QLabel("Verify Code");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #2ecc71; margin-bottom: 5px;");
    content->addWidget(title);

    QLabel *sub = new QLabel("Enter the 6-digit confirmation code we just sent to your email.");
    sub->setWordWrap(true);
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 13px; color: #8892b0; margin-bottom: 15px;");
    content->addWidget(sub);

    QLineEdit *codeInput = new QLineEdit(); 
    codeInput->setPlaceholderText("0 0 0 0 0 0");
    codeInput->setAlignment(Qt::AlignCenter);
    codeInput->setStyleSheet(codeInput->styleSheet() + " font-size: 24px; letter-spacing: 10px;");
    content->addWidget(codeInput);

    content->addSpacing(15);
    
    QPushButton *verifyBtn = new QPushButton("Verify & Authorize");
    verifyBtn->setStyleSheet(getPrimaryBtnStyle());
    verifyBtn->setCursor(Qt::PointingHandCursor);
    content->addWidget(verifyBtn);

    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setStyleSheet(getSecondaryBtnStyle());
    cancelBtn->setCursor(Qt::PointingHandCursor);
    content->addWidget(cancelBtn);

    content->addStretch();
    mainLayout->addLayout(content);

    // -- Verification Logic
    connect(verifyBtn, &QPushButton::clicked, [this, codeInput]() {
        QString code = codeInput->text().trimmed();
        
        if (code == generatedActivationCode) {
            // Apply password (CIN) to DB
            QSqlQuery updt;
            updt.prepare("UPDATE PERSONNEL SET MOT_DE_PASSE = :pass WHERE ID_PERSONNEL = :cin");
            updt.bindValue(":pass", currentActivationCIN);
            updt.bindValue(":cin", currentActivationCIN.toInt());

            if (updt.exec()) {
                showStyledMessage(this, "Activated!", "Your account is active.\nYour temporary password is: " + currentActivationCIN);
                switchToPage(0); // Back to login
                codeInput->clear();
            } else {
                showStyledMessage(this, "Database Error", "Failed to set password: " + updt.lastError().text(), true);
            }
        } else {
            showStyledMessage(this, "Invalid", "The code you entered is invalid.", true);
        }
    });

    connect(cancelBtn, &QPushButton::clicked, [this]() { switchToPage(0); });
    return widget;
}

void LoginSystem::sendActivationEmail(const QString& toEmail, const QString& code) {
    // We instantiate the Smtp class. Since you haven't provided credentials in the code yet,
    // this will connect to smtp.gmail.com with placeholder details that you will need to fill in.
    
    // TODO: REPLACE "YOUR_APP_PASSWORD" WITH A REAL GMAIL APP PASSWORD
    Smtp* smtp = new Smtp("oilpress71@gmail.com", "misaoqheryptlqju", "smtp.gmail.com", 465, 30000);
    
    QString body = "Hello " + currentActivationName + ",\n\n"
                   "Your account activation code for Oil Press Manager is: " + code + "\n\n"
                   "Once verified, you will be able to log in using your ID Personnel as your password.\n";

    // Ensure the sender matches your authentication email
    smtp->sendMail("oilpress71@gmail.com", toEmail, "Account Activation Code", body);
    
    // We delete it later just like in the other usages
    connect(smtp, &Smtp::status, [smtp](const QString & /*statusStr*/){
        // cleanup after sent
        smtp->deleteLater();
    });
}

void LoginSystem::switchToPage(int index) {
    if(stack->currentIndex() == index || (centralWidget() && !centralWidget()->isEnabled())) return;
    
    // Quick guard to prevent overlapping animations
    centralWidget()->setEnabled(false);
    
    QWidget *currentWidget = stack->currentWidget();
    QWidget *nextWidget = stack->widget(index);
    
    int width = stack->width();
    nextWidget->setGeometry(0, 0, width, stack->height());
    
    int offset = (stack->currentIndex() < index) ? width : -width;
    
    QPropertyAnimation *animCurrent = new QPropertyAnimation(currentWidget, "pos");
    animCurrent->setDuration(400);
    animCurrent->setStartValue(QPoint(0, 0));
    animCurrent->setEndValue(QPoint(-offset, 0));
    animCurrent->setEasingCurve(QEasingCurve::OutCubic);
    
    QPropertyAnimation *animNext = new QPropertyAnimation(nextWidget, "pos");
    animNext->setDuration(400);
    animNext->setStartValue(QPoint(offset, 0));
    animNext->setEndValue(QPoint(0, 0));
    animNext->setEasingCurve(QEasingCurve::OutCubic);
    
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animCurrent);
    group->addAnimation(animNext);
    
    nextWidget->show();
    nextWidget->raise();
    
    connect(group, &QParallelAnimationGroup::finished, [this, index, group](){
        stack->setCurrentIndex(index);
        centralWidget()->setEnabled(true);
        group->deleteLater();
    });
    
    group->start();
}

