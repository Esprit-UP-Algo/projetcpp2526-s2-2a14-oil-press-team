#include "mainwindow.h"
#include "personnel.h"
#include "workforceplanner.h"
#include "consultantagent.h"
#include <QComboBox>
#include <QScrollBar>
#include <QDesktopServices>
#include <QUrl>
#include <QDate>
#include <QDateEdit>
#include <QSpinBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QIntValidator>
#include <QRadioButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPrinter>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPrintDialog>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextStream>
#include <QPainter>
#include <QPdfWriter>
#include <QStandardPaths>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLabel>
#include <QLineEdit>
#include <QLinearGradient>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>
#include <algorithm>
#include <functional>
#include <QMessageBox>
#include <QStackedWidget>

void setupTabNavigation(const QList<QPushButton *> &buttons, QStackedWidget *stack) {
  for (int i = 0; i < buttons.size(); ++i) {
    QPushButton *btn = buttons[i];
    QObject::connect(btn, &QPushButton::clicked, [=]() {
      stack->setCurrentIndex(i);
      for (auto *b : buttons) {
        b->setChecked(b == btn);
      }
    });
  }
}
class GenericBarChart : public QWidget {

public:
  struct BarData {
    QString label;
    double value;
    QColor color;
  };

  GenericBarChart(const QString &title, QWidget *parent = nullptr)
      : QWidget(parent), m_title(title) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(300);
  }

  void addBar(const QString &label, double value, const QColor &color) {
    m_bars.append({label, value, color});
    update();
  }

  void clearBars() {
    m_bars.clear();
    update();
  }

protected:
  void paintEvent(QPaintEvent *event) override {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), Qt::white); // White background for clarity

    if (m_bars.isEmpty())
      return;

    // --- Layout Constants ---
    int leftMargin = 80;   // Space for Y-axis labels
    int bottomMargin = 50; // Space for X-axis labels
    int topMargin = 60;    // Space for Title
    int rightMargin = 20;

    int chartWidth = width() - leftMargin - rightMargin;
    int chartHeight = height() - topMargin - bottomMargin;

    // --- Calculate Range ---
    double maxVal = 0;
    for (const auto &bar : m_bars)
      maxVal = qMax(maxVal, bar.value);
    if (maxVal == 0)
      maxVal = 1;
    // Round up to nice number for Y-axis
    double niceMax = maxVal * 1.1;

    // --- Draw Y-Axis (Scales & Grid) ---
    int gridLines = qMax(1, (int)niceMax); // one gridline per unit for small values
    if (gridLines > 8) gridLines = 5;      // cap at 5 lines for large values
    painter.setFont(QFont("Segoe UI", 9));
    QPen gridPen(QColor("#e0e0e0"));
    gridPen.setStyle(Qt::DashLine);

    for (int i = 0; i <= gridLines; ++i) {
      // Always use integers on the Y axis
      int intValue = (int)qRound((niceMax / gridLines) * i);
      int y = topMargin + chartHeight - (int)((intValue / niceMax) * chartHeight);

      // Grid Line
      painter.setPen(gridPen);
      painter.drawLine(leftMargin, y, width() - rightMargin, y);

      // Scale Label — always integer
      painter.setPen(QColor("#666666"));
      QString label;
      if (intValue >= 1000000)
        label = QString::number(intValue / 1000000.0, 'f', 1) + "M";
      else if (intValue >= 1000)
        label = QString::number(intValue / 1000.0, 'f', 0) + "k";
      else
        label = QString::number(intValue);

      painter.drawText(QRect(0, y - 10, leftMargin - 10, 20),
                       Qt::AlignRight | Qt::AlignVCenter, label);
    }

    // --- Draw X-Axis Line ---
    painter.setPen(QPen(QColor("#333333"), 2));
    painter.drawLine(leftMargin, topMargin + chartHeight, width() - rightMargin,
                     topMargin + chartHeight);

    // --- Draw Bars ---
    int count = m_bars.size();
    int availableSpace = chartWidth / count;
    int barWidth = qMin(availableSpace - 20, 80); // Max width 80px
    // Spacing calculation removed as it was unused and causing warnings/errors

    for (int i = 0; i < count; ++i) {
      const auto &bar = m_bars[i];
      int barH = (int)((bar.value / niceMax) * chartHeight);
static QString getLabelStyle() {
  return "font-size: 14px; font-weight: 700; color: #333; margin-top: 5px; "
         "margin-bottom: 3px;";
}

static QString getInputStyle() {
  return "QLineEdit, QDateEdit, QComboBox {"
         "background-color: #fcfcfc;"
         "border: 1px solid #e0e0e0;"
         "border-radius: 6px;"
         "padding: 6px 10px;"
         "font-size: 13px;"
         "color: #333;"
         "min-height: 35px;" /* Fix for squashed inputs */
         "}"
         "QLineEdit:focus, QDateEdit:focus, QComboBox:focus {"
         "border: 2px solid #3DDC84;"
         "background-color: #ffffff;"
         "outline: none;"
         "}";
}

static QString getButtonStyle() {
  return "QPushButton {"
         "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3DDC84, "
         "stop:1 #2DB66F);"
         "color: #FFFFFF;"
         "border: none;"
         "border-radius: 8px;"
         "padding: 12px 24px;"
         "font-size: 14px;"
         "font-weight: 700;"
         "} "
         "QPushButton:hover {"
         "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4EED95, "
         "stop:1 #3DDC84);"
         "margin-top: -1px;" // Subtle lift effect
         "} "
         "QPushButton:pressed {"
         "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2DB66F, "
         "stop:1 #228B5A);"
         "margin-top: 1px;"
         "}";
}

static QString getTabButtonStyle() {
  return "QPushButton {"
         "background-color: #ffffff;"
         "color: #555;"
         "border: 1px solid #ddd;"
         "border-radius: 6px;"
         "padding: 8px 16px;"
         "font-size: 13px;"
         "font-weight: 600;"
         "}"
         "QPushButton:checked { background-color: #e6f9ef; color: #3DDC84; "
         "border-color: #3DDC84; }"
         "QPushButton:hover { border-color: #bbb; color: #333; }";
}

static QString getCardStyle() {
  return "QWidget {"
         "background-color: #ffffff;"
         "border: 1px solid #eaeaea;"
         "border-radius: 12px;"
         "border-bottom: 2px solid #e0e0e0;"
         "}";
}

// --- Component Helpers ---

static QWidget *createStatCard(const QString &title, const QString &value,
                               const QString &change, const QString &color) {
  QWidget *card = new QWidget();
  card->setStyleSheet("QWidget { background-color: #ffffff; border-radius: "
                      "12px; border: 1px solid #f0f0f0; }"
                      "QLabel { border: none; background: transparent; }");
  card->setMinimumHeight(100);

  QVBoxLayout *layout = new QVBoxLayout(card);
  layout->setContentsMargins(20, 20, 20, 20);

  QLabel *lblTitle = new QLabel(title);
  lblTitle->setStyleSheet("font-size: 13px; color: #888; font-weight: 600; "
                          "text-transform: uppercase; letter-spacing: 0.5px;");

  QLabel *lblValue = new QLabel(value);
  lblValue->setStyleSheet(
      "font-size: 28px; color: #1a1a1a; font-weight: 800; margin-top: 5px;");

  QLabel *lblChange = new QLabel(change);
  lblChange->setStyleSheet("font-size: 12px; color: " + color +
                           "; font-weight: 700; margin-top: 5px;");

  layout->addWidget(lblTitle);
  layout->addWidget(lblValue);
  layout->addWidget(lblChange);
  layout->addStretch();

  // Add a colored bottom strip
  QWidget *strip = new QWidget();
  strip->setFixedHeight(4);
  strip->setStyleSheet(
      QString("background-color: %1; border-radius: 2px;").arg(color));
  layout->addWidget(strip);

  return card;
}



static QWidget *
createStyledTable(const QString &title, const QStringList &headers,
                  const QVector<QStringList> &data, bool showActions = false,
                  bool showQrAction = false, bool showViewAction = false) {
  QWidget *tableWidget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(tableWidget);
  layout->setContentsMargins(0, 0, 0, 0);

  if (!title.isEmpty()) {
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: "
                              "#1a1a1a; margin-bottom: 15px;");
    layout->addWidget(titleLabel);
  }

  QStringList finalHeaders = headers;
  if (showActions) {
    finalHeaders << "Actions";
  }

static QWidget *createPersonnelPage(QStackedWidget *&outNestedStack) {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(40, 40, 40, 40);
  layout->setSpacing(25);
  layout->addWidget(new QLabel("Personnel Management"));

  QWidget *actionBar = new QWidget();
  QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
  actionLayout->setSpacing(12);

  outNestedStack = new QStackedWidget();
  QStringList tabNames = {"Staff Hub",   "Add Staff", "Payroll",
                          "Capacity Planner", "Request Hub", "Analytics", "PDF Printing"};
  QList<QPushButton *> tabButtons;

  // Shared pointer for the table
  QTableWidget *personnelTable = nullptr;

  for (const auto &name : tabNames) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(getTabButtonStyle());
    actionLayout->addWidget(btn);
    tabButtons.append(btn);

    QWidget *content = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout(content);

    if (name == "Staff Hub") {
      // ========== AFFICHER (READ) + MODIFIER + SUPPRIMER ==========
      QWidget *listPageWidget = new QWidget();
      QVBoxLayout *listPageLayout = new QVBoxLayout(listPageWidget);
      listPageLayout->setContentsMargins(0, 0, 0, 0);
      listPageLayout->setSpacing(10);

      // Search Bar
      QWidget *controlBar = new QWidget();
      QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
      controlLayout->setContentsMargins(0, 0, 0, 0);

      QLineEdit *searchEdit = new QLineEdit();
      searchEdit->setPlaceholderText(" 🔍  Search by Name, CIN, or Role...");
      searchEdit->setStyleSheet(getInputStyle());
      searchEdit->setFixedWidth(280);

      QComboBox *statusCombo = new QComboBox();
      statusCombo->addItems({"Filter: All Status", "Filter: Active Only", "Filter: Inactive Only"});
      statusCombo->setStyleSheet("QComboBox { background: white; border: 1px solid #d1d5db; border-radius: 6px; padding: 5px 10px; font-weight: 600; min-width: 150px; } QComboBox::drop-down { border: none; }");

      QPushButton *btnRefresh = new QPushButton("Refresh List");
      btnRefresh->setObjectName("btnRefreshPersonnel");
      btnRefresh->setStyleSheet("QPushButton { background-color: #f3f4f6; color: #374151; border: 1px solid #d1d5db; border-radius: 6px; padding: 8px 15px; font-weight: bold; } QPushButton:hover { background-color: #e5e7eb; }");
      btnRefresh->setCursor(Qt::PointingHandCursor);

      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(statusCombo);
      controlLayout->addStretch();
      controlLayout->addWidget(btnRefresh);
      listPageLayout->addWidget(controlBar);

      // Table
      personnelTable = new QTableWidget();
      QStringList headers = {"CIN", "Name", "Salary", "Address", "Phone", "Exp", "Grade", "Role", "Email", "Status", "Actions"};
      personnelTable->setColumnCount(headers.size());
      personnelTable->setHorizontalHeaderLabels(headers);
      personnelTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      personnelTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      personnelTable->verticalHeader()->setVisible(false);
      personnelTable->verticalHeader()->setDefaultSectionSize(60);
      personnelTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      personnelTable->setSelectionBehavior(QAbstractItemView::SelectRows);
      personnelTable->setAlternatingRowColors(true);
      personnelTable->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; gridline-color: transparent; border-radius: 8px; alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; color: #666; text-transform: uppercase; font-size: 11px; letter-spacing: 0.5px; }"
          "QTableWidget::item { padding: 12px; border-bottom: 1px solid #f5f5f5; color: #333; }"
          "QTableWidget::item:selected { background-color: #ecfdf5; color: #065f46; }"
      );

      listPageLayout->addWidget(personnelTable);
      cLayout->addWidget(listPageWidget);

      auto refreshTable = [personnelTable, searchEdit, statusCombo, outNestedStack]() {
          personnelTable->setSortingEnabled(false); 
          QString filter = searchEdit->text().trimmed();
          int statusIdx = statusCombo->currentIndex();
          
          QString queryStr = "SELECT ID_PERSONNEL, NOM_PERSONNEL, SALAIRE_BRUT, ADRESSE, TEL, EXPERIENCE, GRADE, ROLE, EMAIL, STATUS FROM PERSONNEL WHERE 1=1";
          if (!filter.isEmpty()) {
              queryStr += QString(" AND (NOM_PERSONNEL LIKE '%%1%' OR ROLE LIKE '%%1%' OR CAST(ID_PERSONNEL AS VARCHAR(255)) LIKE '%%1%')").arg(filter);
          }
          if (statusIdx == 1) queryStr += " AND STATUS = 'Active'";
          else if (statusIdx == 2) queryStr += " AND STATUS != 'Active'";
          
          queryStr += " ORDER BY NOM_PERSONNEL ASC";
          
          QSqlQuery q(queryStr);
          personnelTable->setRowCount(0);
          int row = 0;
          while(q.next()) {
              personnelTable->insertRow(row);
              for (int j = 0; j < 10; ++j) {
                  personnelTable->setItem(row, j, new QTableWidgetItem(q.value(j).toString()));
              }

              // Action Buttons
              QWidget *actionWidget = new QWidget();
              QHBoxLayout *actionBtnLayout = new QHBoxLayout(actionWidget);
              actionBtnLayout->setContentsMargins(10, 0, 10, 0);
              actionBtnLayout->setSpacing(10);
              actionBtnLayout->setAlignment(Qt::AlignCenter);
              
              QPushButton *btnModify = new QPushButton("Edit");
              btnModify->setStyleSheet("QPushButton { background-color: white; color: #374151; border: 1px solid #d1d5db; border-radius: 6px; padding: 4px 8px; font-weight: 600; } QPushButton:hover { background-color: #f9fafb; }");
              QPushButton *btnDelete = new QPushButton("Remove");
              btnDelete->setStyleSheet("QPushButton { background-color: #fee2e2; color: #991b1b; border: 1px solid #fecaca; border-radius: 6px; padding: 4px 8px; font-weight: 600; } QPushButton:hover { background-color: #fecaca; }");
              
              actionBtnLayout->addWidget(btnModify);
              actionBtnLayout->addWidget(btnDelete);
              
              int currentCin = q.value(0).toInt();
              QObject::connect(btnDelete, &QPushButton::clicked, [currentCin, outNestedStack]() {
                  if (QMessageBox::question(nullptr, "Confirm Deletion", QString("Permanently delete Employee %1?").arg(currentCin)) == QMessageBox::Yes) {
                      QSqlQuery del; del.prepare("DELETE FROM PERSONNEL WHERE ID_PERSONNEL = :cin");
                      del.bindValue(":cin", currentCin);
                      if (del.exec()) {
                          QPushButton *btn = outNestedStack->window()->findChild<QPushButton*>("btnRefreshPersonnel");
                          if (btn) btn->click();
                      }
                  }
              });

              QObject::connect(btnModify, &QPushButton::clicked, [currentCin, outNestedStack, personnelTable]() {
                  QSqlQuery mq;
                  mq.prepare("SELECT * FROM PERSONNEL WHERE ID_PERSONNEL = :id");
                  mq.bindValue(":id", currentCin);
                  if (mq.exec() && mq.next()) {
                      QDialog *dialog = new QDialog();
                      dialog->setWindowTitle("Modify Employee Information");
                      dialog->setMinimumWidth(450);
                      QVBoxLayout *dLayout = new QVBoxLayout(dialog);
                      dLayout->setContentsMargins(25, 25, 25, 25);
                      dLayout->setSpacing(12);

                      auto inputStyle = getInputStyle();
                      auto addField = [&](QString label, QWidget *w) {
                          dLayout->addWidget(new QLabel(label));
                          w->setStyleSheet(inputStyle);
                          if (qobject_cast<QLineEdit*>(w) || qobject_cast<QComboBox*>(w)) w->setFixedHeight(40);
                          dLayout->addWidget(w);
                      };

                      QLineEdit *nameIn = new QLineEdit(mq.value("NOM_PERSONNEL").toString());
                      QLineEdit *salIn = new QLineEdit(mq.value("SALAIRE_BRUT").toString());
                      QLineEdit *addrIn = new QLineEdit(mq.value("ADRESSE").toString());
                      QLineEdit *telIn = new QLineEdit(mq.value("TEL").toString());
                      QLineEdit *emailIn = new QLineEdit(mq.value("EMAIL").toString());
                      QComboBox *statusIn = new QComboBox(); statusIn->addItems({"Active", "Inactive", "On Leave"});
                      statusIn->setCurrentText(mq.value("STATUS").toString());

                      addField("Full Name:", nameIn);
                      addField("Salary:", salIn);
                      addField("Address:", addrIn);
                      addField("Phone:", telIn);
                      addField("Email:", emailIn);
                      addField("Status:", statusIn);

                      QPushButton *btnSave = new QPushButton("Save Changes");
                      btnSave->setStyleSheet(getButtonStyle());
                      btnSave->setFixedHeight(45);
                      dLayout->addWidget(btnSave);

                      QObject::connect(btnSave, &QPushButton::clicked, [=]() {
                          QSqlQuery up;
                          up.prepare("UPDATE PERSONNEL SET NOM_PERSONNEL=:n, SALAIRE_BRUT=:s, ADRESSE=:a, TEL=:t, EMAIL=:e, STATUS=:st WHERE ID_PERSONNEL=:id");
                          up.bindValue(":n", nameIn->text());
                          up.bindValue(":s", salIn->text().toDouble());
                          up.bindValue(":a", addrIn->text());
                          up.bindValue(":t", telIn->text());
                          up.bindValue(":e", emailIn->text());
                          up.bindValue(":st", statusIn->currentText());
                          up.bindValue(":id", currentCin);
                          if (up.exec()) {
                              dialog->accept();
                              QPushButton *btn = outNestedStack->window()->findChild<QPushButton*>("btnRefreshPersonnel");
                              if (btn) btn->click();
                          }
                      });
                      dialog->exec();
                  }
              });
              
              personnelTable->setCellWidget(row, 10, actionWidget);
              row++;
          }
          personnelTable->setSortingEnabled(true);
      };

      QObject::connect(searchEdit, &QLineEdit::textChanged, refreshTable);
      QObject::connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [refreshTable](){ refreshTable(); });
      QObject::connect(btnRefresh, &QPushButton::clicked, [=](){ searchEdit->clear(); statusCombo->setCurrentIndex(0); refreshTable(); });
      
      
      refreshTable(); // Initial load
    } else if (name == "Add Staff") {
      // ========== AJOUTER (CREATE) ==========
      QWidget *formContainer = new QWidget();
      QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
      formLayout->setSpacing(10);
      formLayout->setContentsMargins(0, 0, 10, 0);

      QLabel *titleLabel = new QLabel("New Employee Registration");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; margin-bottom: 15px; border: none;");
      formLayout->addWidget(titleLabel);

      QString labelStyle = getLabelStyle();
      QString inputStyle = getInputStyle();

      auto attachVisualValidation = [](QLineEdit *le) {
          QObject::connect(le, &QLineEdit::textChanged, [le]() {
              bool valid = true;
              if (le->validator()) valid = le->hasAcceptableInput();
              if (valid && !le->text().trimmed().isEmpty()) {
                  le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #3DDC84; border-radius: 6px; padding: 6px 10px; font-size: 13px; min-height: 35px; }");
              } else {
                  le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #d32f2f; border-radius: 6px; padding: 6px 10px; font-size: 13px; min-height: 35px; }");
              }
          });
      };

      QLineEdit *cinInput = new QLineEdit(); cinInput->setPlaceholderText("CIN Number (8 Digits)");
      cinInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9]{8}$"), cinInput));
      attachVisualValidation(cinInput);

      QLineEdit *nameInput = new QLineEdit(); nameInput->setPlaceholderText("Full Name");
      nameInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z\\s]{2,50}$"), nameInput));
      attachVisualValidation(nameInput);

      QLineEdit *salInput = new QLineEdit(); salInput->setPlaceholderText("Salary (e.g. 1500.50)");
      QDoubleValidator *salVal = new QDoubleValidator(1.0, 99999.0, 2, salInput);
      salVal->setNotation(QDoubleValidator::StandardNotation);
      salInput->setValidator(salVal);
      attachVisualValidation(salInput);

      QLineEdit *addrInput = new QLineEdit(); addrInput->setPlaceholderText("Home Address");
      attachVisualValidation(addrInput);

      QLineEdit *telInput = new QLineEdit(); telInput->setPlaceholderText("Phone Number (8 Digits)");
      telInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9]{8}$"), telInput));
      attachVisualValidation(telInput);

      QLineEdit *expInput = new QLineEdit(); expInput->setPlaceholderText("Years of Exp (0 - 50)");
      expInput->setValidator(new QIntValidator(0, 50, expInput));
      attachVisualValidation(expInput);
      QComboBox *gradeInput = new QComboBox();
      gradeInput->addItems({"Junior", "Senior", "Lead", "Principal", "Chief"});
      QComboBox *roleInput = new QComboBox();
      roleInput->addItems({"Stock Management", "Product Management", "Maintenance Management", "Personnel Management", "Order Management", "Financial Management"});

      QLineEdit *emailInput = new QLineEdit(); emailInput->setPlaceholderText("Email Address");
      emailInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$"), emailInput));
      attachVisualValidation(emailInput);

      QComboBox *statusInput = new QComboBox();
      statusInput->addItems({"Active", "Inactive", "On Leave"});

      auto addField = [&](QString label, QWidget *w) {
        formLayout->addWidget(new QLabel(label));
        w->setStyleSheet(inputStyle);
        if (qobject_cast<QLineEdit*>(w) || qobject_cast<QComboBox*>(w))
            w->setFixedHeight(45);
        formLayout->addWidget(w);
      };

      addField("CIN:", cinInput);
      addField("Name:", nameInput);
      addField("Salary:", salInput);
      addField("Address:", addrInput);
      addField("Phone:", telInput);
      addField("Experience:", expInput);
      addField("Grade:", gradeInput);
      addField("Role:", roleInput);
      addField("Email:", emailInput);
      addField("Status:", statusInput);

      QPushButton *btnHire = new QPushButton("Hire Employee");
      btnHire->setStyleSheet(getButtonStyle());
      btnHire->setFixedHeight(45);
      formLayout->addWidget(btnHire);
      formLayout->addStretch();

      QObject::connect(btnHire, &QPushButton::clicked, [=]() {
          if (!cinInput->hasAcceptableInput() || !nameInput->hasAcceptableInput() || 
              !salInput->hasAcceptableInput() || !telInput->hasAcceptableInput() || 
              !expInput->hasAcceptableInput() || !emailInput->hasAcceptableInput() || addrInput->text().trimmed().isEmpty()) {
              QMessageBox::warning(nullptr, "Error Validation", "Please fill out all fields correctly. Check the red outlines for invalid elements.");
              return;
          }

          Personnel p;
          p.setCin(cinInput->text().toInt());
          p.setNom(nameInput->text());
          p.setSalaire(salInput->text().toDouble());
          p.setAdresse(addrInput->text());
          p.setTel(telInput->text());
          p.setExperience(expInput->text().toInt());
          p.setGrade(gradeInput->currentText());
          p.setRole(roleInput->currentText());
          p.setEmail(emailInput->text());
          p.setStatus(statusInput->currentText());

          if (p.ajouter()) {
              QMessageBox::information(nullptr, "Success", "Employee added successfully.");
              cinInput->clear(); nameInput->clear(); salInput->clear(); addrInput->clear();
              telInput->clear(); expInput->clear(); emailInput->clear();
              outNestedStack->setCurrentIndex(0);
              
              QPushButton *btn = outNestedStack->window()->findChild<QPushButton*>("btnRefreshPersonnel");
              if (btn) btn->click();
          } else {
              QMessageBox::critical(nullptr, "Error", p.getLastError());
          }
      });

      cLayout->addWidget(formContainer);

    } else if (name == "Capacity Planner") {
      WorkforcePlanner *planner = new WorkforcePlanner();
      cLayout->addWidget(planner);
    } else if (name == "Payroll") {
      QWidget *payrollWidget = new QWidget();
      QVBoxLayout *payrollLayout = new QVBoxLayout(payrollWidget);
      payrollLayout->setContentsMargins(0, 0, 0, 0);
      payrollLayout->setSpacing(20);

      QWidget *headerWidget = new QWidget();
      QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
      headerLayout->setContentsMargins(0, 0, 0, 5);
      
      QLabel *title = new QLabel("Global Payroll & Operational Burn");
      title->setStyleSheet("font-size: 26px; font-weight: 800; color: #111827; letter-spacing: -0.5px;");
      
      QPushButton *btnRefreshPayroll = new QPushButton("    Sync Week %1 Analytics    ");
      btnRefreshPayroll->setText(btnRefreshPayroll->text().arg(QDate::currentDate().weekNumber()));
      btnRefreshPayroll->setStyleSheet("QPushButton { background-color: #1D9E75; color: white; font-weight: bold; font-size: 13px; border-radius: 8px; padding: 10px 20px; border: none; } QPushButton:hover { background-color: #147b5b; }");
      btnRefreshPayroll->setCursor(Qt::PointingHandCursor);
      btnRefreshPayroll->setFixedHeight(42);
      
      headerLayout->addWidget(title);
      headerLayout->addStretch();
      headerLayout->addWidget(btnRefreshPayroll);
      payrollLayout->addWidget(headerWidget);

      QWidget *kpiRow = new QWidget();
      QHBoxLayout *kpiLayout = new QHBoxLayout(kpiRow);
      kpiLayout->setContentsMargins(0, 0, 0, 0);
      kpiLayout->setSpacing(15);

      auto createKpi = [](const QString &label, const QString &gradient) -> QPair<QWidget*, QLabel*> {
          QWidget *card = new QWidget();
          card->setStyleSheet(QString("QWidget { background: %1; border-radius: 12px; }").arg(gradient));
          QVBoxLayout *l = new QVBoxLayout(card);
          l->setContentsMargins(20, 15, 20, 15);
          QLabel *title = new QLabel(label);
          title->setStyleSheet("font-size: 11px; font-weight: 700; color: rgba(255,255,255,0.8); text-transform: uppercase; border: none; background: transparent;");
          QLabel *val = new QLabel("...");
          val->setStyleSheet("font-size: 24px; font-weight: 900; color: white; border: none; background: transparent;");
          l->addWidget(title);
          l->addWidget(val);
          return {card, val};
      };

      auto kpiTotal = createKpi("Projected Weekly Burn", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #111827, stop:1 #374151)");
      auto kpiStaff = createKpi("Active Human Capital", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #10b981, stop:1 #059669)");
      auto kpiEfficiency = createKpi("Cost Per Shift", "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3b82f6, stop:1 #2563eb)");

      kpiLayout->addWidget(kpiTotal.first);
      kpiLayout->addWidget(kpiStaff.first);
      kpiLayout->addWidget(kpiEfficiency.first);
      payrollLayout->addWidget(kpiRow);

      QLabel *lblTotal = kpiTotal.second;
      QLabel *lblCount = kpiStaff.second;
      QLabel *lblEfficiency = kpiEfficiency.second;

      QHBoxLayout *tablesLayout = new QHBoxLayout();
      tablesLayout->setSpacing(20);

      QWidget *activeContainer = new QWidget();
      QVBoxLayout *activeLayout = new QVBoxLayout(activeContainer);
      activeLayout->setContentsMargins(0, 0, 0, 0);
      QLabel *activeTitle = new QLabel("Active Personnel");
      activeTitle->setStyleSheet("font-weight: 800; font-size: 16px; color: #1f2937; margin-bottom: 5px;");
      
      QTableWidget *activeTable = new QTableWidget();
      activeTable->setColumnCount(4);
      activeTable->setHorizontalHeaderLabels({"CIN", "Employee Name", "Role", "Gross Salary"});
      activeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      activeTable->verticalHeader()->setVisible(false);
      activeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      activeTable->setStyleSheet(
          "QTableWidget { border: 1px solid #e5e7eb; border-radius: 8px; background-color: #ffffff; gridline-color: #f3f4f6; alternate-background-color: #fafaf9; }"
          "QHeaderView::section { background-color: #f9fafb; padding: 14px 12px; border: none; border-bottom: 2px solid #e5e7eb; font-weight: 700; color: #4b5563; text-transform: uppercase; font-size: 11px; letter-spacing: 0.5px; border-top-left-radius: 8px; border-top-right-radius: 8px; }"
          "QTableWidget::item { padding: 8px 12px; border-bottom: 1px solid #f3f4f6; color: #1f2937; font-size: 13px; }"
          "QTableWidget::item:selected { background-color: #ecfdf5; color: #065f46; }"
      );
      activeLayout->addWidget(activeTitle);
      activeLayout->addWidget(activeTable);

      QWidget *inactiveContainer = new QWidget();
      QVBoxLayout *inactiveLayout = new QVBoxLayout(inactiveContainer);
      inactiveLayout->setContentsMargins(0, 0, 0, 0);
      QLabel *inactiveTitle = new QLabel("Inactive / On Leave");
      inactiveTitle->setStyleSheet("font-weight: 800; font-size: 16px; color: #1f2937; margin-bottom: 5px;");
      
      QTableWidget *inactiveTable = new QTableWidget();
      inactiveTable->setColumnCount(4);
      inactiveTable->setHorizontalHeaderLabels({"CIN", "Employee Name", "Role", "Gross Salary"});
      inactiveTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      inactiveTable->verticalHeader()->setVisible(false);
      inactiveTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      inactiveTable->setStyleSheet(activeTable->styleSheet());
      
      inactiveLayout->addWidget(inactiveTitle);
      inactiveLayout->addWidget(inactiveTable);

      tablesLayout->addWidget(activeContainer);
      tablesLayout->addWidget(inactiveContainer);
      
      payrollLayout->addLayout(tablesLayout);

      auto refreshPayrollData = [lblTotal, lblCount, lblEfficiency, activeTable, inactiveTable]() {
          int cw = QDate::currentDate().weekNumber();
          int cy = QDate::currentDate().year();
          
          double weekTotal = 0.0;
          int activeCount = 0;
          int totalShifts = 0;

          // 1. Identify everyone who has APPROVED leave this week
          QSet<int> onLeaveIds;
          QSqlQuery lq;
          lq.prepare("SELECT DISTINCT ID_PERSONNEL FROM SHIFT_SCHEDULE WHERE WEEK_NUMBER = :wk AND YEAR = :yr AND LEAVE_APPROVED = 'Approved'");
          lq.bindValue(":wk", cw);
          lq.bindValue(":yr", cy);
          lq.exec();
          while(lq.next()) onLeaveIds.insert(lq.value(0).toInt());

          // 2. Count total planned shifts for efficiency ratio
          QSqlQuery sq;
          sq.prepare("SELECT COUNT(*) FROM SHIFT_SCHEDULE WHERE WEEK_NUMBER = :wk AND YEAR = :yr AND LEAVE_START IS NULL");
          sq.bindValue(":wk", cw);
          sq.bindValue(":yr", cy);
          sq.exec();
          if (sq.next()) totalShifts = sq.value(0).toInt();

          activeTable->setRowCount(0);
          inactiveTable->setRowCount(0);

          QSqlQuery q("SELECT ID_PERSONNEL, NOM_PERSONNEL, ROLE, SALAIRE_BRUT, STATUS FROM PERSONNEL");
          int aRow = 0, iRow = 0;
          while(q.next()) {
              int id = q.value(0).toInt();
              QString name = q.value(1).toString();
              QString role = q.value(2).toString();
              double salary = q.value(3).toDouble();
              QString status = q.value(4).toString();
              
              bool currentlyOnLeave = onLeaveIds.contains(id) || (status != "Active");
              QTableWidget *target = currentlyOnLeave ? inactiveTable : activeTable;
              int row = currentlyOnLeave ? iRow++ : aRow++;
              
              target->insertRow(row);
              target->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
              target->setItem(row, 1, new QTableWidgetItem(name));
              target->setItem(row, 2, new QTableWidgetItem(role));
              
              double weeklyShare = salary / 4.0;
              QTableWidgetItem *valItem = new QTableWidgetItem(QString("%1 TND").arg(weeklyShare, 0, 'f', 2));
              valItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
              target->setItem(row, 3, valItem);
              
              if (!currentlyOnLeave) {
                  weekTotal += weeklyShare;
                  activeCount++;
              }
          }

          lblTotal->setText(QString("%1 TND").arg(weekTotal, 0, 'f', 2));
          lblCount->setText(QString("%1 Staff Scheduled").arg(activeCount));
          
          double costPerShift = (totalShifts > 0) ? (weekTotal / totalShifts) : 0.0;
          lblEfficiency->setText(QString("%1 TND").arg(costPerShift, 0, 'f', 2));
      };

      QObject::connect(btnRefreshPayroll, &QPushButton::clicked, refreshPayrollData);
      refreshPayrollData(); 
      
      cLayout->addWidget(payrollWidget);
    } else if (name == "Analytics") {
      QWidget *statsContainer = new QWidget();
      statsContainer->setObjectName("StatsContainer");
      statsContainer->setStyleSheet("QWidget#StatsContainer { background-color: transparent; }");
      QVBoxLayout *statsLayout = new QVBoxLayout(statsContainer);
      statsLayout->setSpacing(20);

      QWidget *headerWidget = new QWidget();
      QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
      headerLayout->setContentsMargins(0, 0, 0, 10);
      
      QLabel *title = new QLabel("Executive Dashboard");
      title->setStyleSheet("font-size: 26px; font-weight: 800; color: #111827; letter-spacing: -0.5px;");
      
      QPushButton *btnRefreshAnalytics = new QPushButton("    Refresh Analytics    ");
      btnRefreshAnalytics->setStyleSheet("QPushButton { background-color: #f3f4f6; color: #374151; font-weight: bold; font-size: 13px; border-radius: 8px; padding: 10px 20px; border: 1px solid #d1d5db; } QPushButton:hover { background-color: #e5e7eb; border-color: #9ca3af; }");
      btnRefreshAnalytics->setCursor(Qt::PointingHandCursor);
      btnRefreshAnalytics->setFixedHeight(42);
      
      headerLayout->addWidget(title);
      headerLayout->addStretch();
      headerLayout->addWidget(btnRefreshAnalytics);
      statsLayout->addWidget(headerWidget);

      QWidget *kpiContainer = new QWidget();
      QHBoxLayout *kpiLayout = new QHBoxLayout(kpiContainer);
      kpiLayout->setSpacing(15);
      kpiLayout->setContentsMargins(0, 0, 0, 0);

      auto createKpiCard = [](const QString &label, QLabel*& valLabelRef) -> QWidget* {
          QWidget *card = new QWidget();
          card->setStyleSheet("QWidget { background-color: #ffffff; border: 1px solid #e5e7eb; border-radius: 12px; }");
          QVBoxLayout *l = new QVBoxLayout(card);
          l->setContentsMargins(20, 20, 20, 20);
          
          QLabel *lbl = new QLabel(label);
          lbl->setStyleSheet("font-weight: 700; color: #6b7280; font-size: 12px; text-transform: uppercase; border: none;");
          valLabelRef = new QLabel("...");
          valLabelRef->setStyleSheet("font-weight: 900; color: #111827; font-size: 24px; border: none;");
          
          l->addWidget(lbl);
          l->addWidget(valLabelRef);
          return card;
      };

      QLabel *lblTotalHeadcount, *lblActiveRatio, *lblAvgSalary;
      kpiLayout->addWidget(createKpiCard("Total Directory Headcount", lblTotalHeadcount));
      kpiLayout->addWidget(createKpiCard("Active Personnel Ratio", lblActiveRatio));
      kpiLayout->addWidget(createKpiCard("Global Average Salary", lblAvgSalary));
      
      statsLayout->addWidget(kpiContainer);

      QGridLayout *chartsLayout = new QGridLayout();
      chartsLayout->setSpacing(20);
      statsLayout->addLayout(chartsLayout);
      
      auto refreshAnalytics = [lblTotalHeadcount, lblActiveRatio, lblAvgSalary, chartsLayout]() {
          QSqlQuery kpiQ("SELECT COUNT(*), SUM(CASE WHEN STATUS = 'Active' THEN 1 ELSE 0 END), AVG(SALAIRE_BRUT) FROM PERSONNEL");
          if (kpiQ.next()) {
              int totalCount = kpiQ.value(0).toInt();
              int activeCount = kpiQ.value(1).toInt();
              double avgSal = kpiQ.value(2).toDouble();
              
              lblTotalHeadcount->setText(QString::number(totalCount));
              double ratio = totalCount > 0 ? (double)activeCount / totalCount * 100.0 : 0.0;
              lblActiveRatio->setText(QString("%1% (%2 Active)").arg(ratio, 0, 'f', 1).arg(activeCount));
              lblAvgSalary->setText(QString("%1 TND").arg(avgSal, 0, 'f', 2));
          }

          QLayoutItem *child;
          while ((child = chartsLayout->takeAt(0)) != nullptr) {
              if (child->widget()) {
                  child->widget()->deleteLater();
              }
              delete child;
          }
          
          QList<QColor> colors = {QColor(59, 130, 246), QColor(16, 185, 129), QColor(245, 158, 11), QColor(139, 92, 246), QColor(239, 68, 68), QColor(14, 165, 233)};
          
          auto wrapChart = [](QWidget* chart) -> QWidget* {
              QWidget *w = new QWidget();
              w->setStyleSheet("QWidget { background-color: #ffffff; border: 1px solid #e5e7eb; border-radius: 12px; }");
              QVBoxLayout *v = new QVBoxLayout(w);
              v->setContentsMargins(15, 15, 15, 15);
              v->addWidget(chart);
              return w;
          };

          GenericBarChart *chart1 = new GenericBarChart("Employee Headcount by Role");
          QSqlQuery q1("SELECT ROLE, COUNT(*) FROM PERSONNEL GROUP BY ROLE");
          int cIdx = 0;
          while (q1.next()) {
              chart1->addBar(q1.value(0).toString(), q1.value(1).toInt(), colors[cIdx % colors.size()]);
              cIdx++;
          }
          chartsLayout->addWidget(wrapChart(chart1), 0, 0);

          GenericBarChart *chart2 = new GenericBarChart("Active Monthly Expense by Role");
          QSqlQuery q2("SELECT ROLE, SUM(SALAIRE_BRUT) FROM PERSONNEL WHERE STATUS = 'Active' GROUP BY ROLE");
          cIdx = 0;
          while (q2.next()) {
              chart2->addBar(q2.value(0).toString(), q2.value(1).toInt(), colors[(cIdx+1) % colors.size()]);
              cIdx++;
          }
          chartsLayout->addWidget(wrapChart(chart2), 0, 1);

          GenericBarChart *chart3 = new GenericBarChart("Personnel by Employment Status");
          QSqlQuery q3("SELECT COALESCE(STATUS, 'Unknown'), COUNT(*) FROM PERSONNEL GROUP BY STATUS");
          cIdx = 0;
          while (q3.next()) {
              QString statusName = q3.value(0).toString();
              if (statusName.trimmed().isEmpty()) statusName = "Unknown";
              chart3->addBar(statusName, q3.value(1).toInt(), colors[(cIdx+2) % colors.size()]);
              cIdx++;
          }
          chartsLayout->addWidget(wrapChart(chart3), 1, 0);

          GenericBarChart *chart4 = new GenericBarChart("Average Active Salary by Role");
          QSqlQuery q4("SELECT ROLE, AVG(SALAIRE_BRUT) FROM PERSONNEL WHERE STATUS = 'Active' GROUP BY ROLE");
          cIdx = 0;
          while (q4.next()) {
              chart4->addBar(q4.value(0).toString(), q4.value(1).toInt(), colors[(cIdx+3) % colors.size()]);
              cIdx++;
          }
          chartsLayout->addWidget(wrapChart(chart4), 1, 1);
      };

      QObject::connect(btnRefreshAnalytics, &QPushButton::clicked, refreshAnalytics);
      refreshAnalytics();

      cLayout->addWidget(statsContainer);

    } else if (name == "PDF Printing") {
      QWidget *formContainer = new QWidget();
      QVBoxLayout *vbox = new QVBoxLayout(formContainer);
      vbox->setSpacing(15);

      QLabel *title = new QLabel("Absence Request Form");
      title->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; "
                           "margin-bottom: 20px;");
      vbox->addWidget(title);

      QString labelStyle = getLabelStyle();
      QString inputStyle = getInputStyle();

      vbox->addWidget(new QLabel("Employee CIN (ID):"));
      QLineEdit *eCin = new QLineEdit();
      eCin->setStyleSheet(inputStyle);
      eCin->setPlaceholderText("Enter Employee ID (e.g. 1001)");
      vbox->addWidget(eCin);
      
      vbox->addWidget(new QLabel("Start Date (YYYY-MM-DD):"));
      QLineEdit *sDate = new QLineEdit();
      sDate->setStyleSheet(inputStyle);
      sDate->setInputMask("0000-00-00");
      sDate->setText(QDate::currentDate().toString("yyyy-MM-dd"));
      vbox->addWidget(sDate);
      
      vbox->addWidget(new QLabel("End Date (YYYY-MM-DD):"));
      QLineEdit *eDate = new QLineEdit();
      eDate->setStyleSheet(inputStyle);
      eDate->setInputMask("0000-00-00");
      eDate->setText(QDate::currentDate().toString("yyyy-MM-dd"));
      vbox->addWidget(eDate);
      vbox->addWidget(new QLabel("Reason:"));
      QLineEdit *reason = new QLineEdit();
      reason->setStyleSheet(inputStyle);
      vbox->addWidget(reason);

      vbox->addSpacing(30);

      QPushButton *btnSubmit = new QPushButton("Submit Leave Request \u2705");
      btnSubmit->setStyleSheet("QPushButton { background-color: #1D9E75; color: white; border-radius: 6px; padding: 12px; font-weight: bold; font-size: 14px; border: none; } QPushButton:hover { background-color: #147b5b; }");
      btnSubmit->setCursor(Qt::PointingHandCursor);
      btnSubmit->setFixedHeight(50);
      vbox->addWidget(btnSubmit);
      
      vbox->addSpacing(10);

      QPushButton *btnPrint = new QPushButton("Print Request to PDF");
      btnPrint->setStyleSheet("QPushButton { background-color: #f3f4f6; color: #374151; border: 1px solid #d1d5db; border-radius: 6px; padding: 10px; font-weight: bold; } QPushButton:hover { background-color: #e5e7eb; }");
      btnPrint->setCursor(Qt::PointingHandCursor);
      vbox->addWidget(btnPrint);
      vbox->addStretch();

      cLayout->addWidget(formContainer);

      QObject::connect(btnSubmit, &QPushButton::clicked, [=]() {
          QString cin = eCin->text().trimmed();
          QDate start = QDate::fromString(sDate->text(), "yyyy-MM-dd");
          QDate end = QDate::fromString(eDate->text(), "yyyy-MM-dd");
          QString note = reason->text().trimmed();
          
          if (cin.isEmpty() || !start.isValid() || !end.isValid()) {
              QMessageBox::warning(nullptr, "Error", "Please provide a valid CIN and Dates (YYYY-MM-DD)");
              return;
          }
          
          // Verify Personnel exists
          QSqlQuery v;
          v.prepare("SELECT NOM_PERSONNEL FROM PERSONNEL WHERE ID_PERSONNEL = :id");
          v.bindValue(":id", cin);
          v.exec();
          if (!v.next()) {
              QMessageBox::warning(nullptr, "Error", "Employee ID " + cin + " not found!");
              return;
          }
          
          QSqlQuery q;
          q.prepare("INSERT INTO SHIFT_SCHEDULE (ID, ID_PERSONNEL, WEEK_NUMBER, YEAR, SHIFT_TYPE, DAY_OF_WEEK, LEAVE_START, LEAVE_END, LEAVE_APPROVED) "
                    "VALUES (NVL((SELECT MAX(ID) FROM SHIFT_SCHEDULE), 0) + 1, :id, :wk, :yr, 0, 0, :ls, :le, 'Pending')");
          q.bindValue(":id", cin);
          q.bindValue(":wk", start.weekNumber());
          q.bindValue(":yr", start.year());
          q.bindValue(":ls", start);
          q.bindValue(":le", end);
          
          if (q.exec()) {
              QMessageBox::information(nullptr, "Success", "Leave request for " + v.value(0).toString() + " submitted successfully!");
              eCin->clear();
              reason->clear();
          } else {
              QMessageBox::critical(nullptr, "Database Error", q.lastError().text());
          }
      });

      QObject::connect(btnPrint, &QPushButton::clicked, [=]() {
          QString cin = eCin->text().trimmed();
          if (cin.isEmpty()) {
              QMessageBox::warning(nullptr, "Incomplete Data", "Please enter a CIN before printing.");
              return;
          }

          // Fetch Employee Details for the PDF
          QString name = "Unknown", phone = "N/A", email = "N/A";
          QSqlQuery detailQuery;
          detailQuery.prepare("SELECT NOM_PERSONNEL, TEL, EMAIL FROM PERSONNEL WHERE ID_PERSONNEL = :id");
          detailQuery.bindValue(":id", cin);
          if (detailQuery.exec() && detailQuery.next()) {
              name = detailQuery.value(0).toString();
              phone = detailQuery.value(1).toString();
              email = detailQuery.value(2).toString();
          }

          // 1. Resolve Path (Desktop)
          QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
          QString fileName = QString("%1/Leave_Request_%2.pdf").arg(desktopPath, name.replace(" ", "_"));
          
          // 2. Setup PDF Writer
          QPdfWriter pdf(fileName);
          pdf.setPageSize(QPageSize(QPageSize::A4));
          pdf.setPageMargins(QMargins(30, 30, 30, 30));
          
          QPainter painter(&pdf);
          if (!painter.isActive()) {
              QMessageBox::critical(nullptr, "Error", "Failed to initialize PDF writer. Possible permission issue.");
              return;
          }

          // 3. Draw Content
          int y = 100;
          
          // Header / Branding
          painter.setPen(QPen(QColor("#1D9E75"), 5));
          painter.setFont(QFont("Arial", 24, QFont::Bold));
          painter.drawText(QRect(0, y, 9000, 500), Qt::AlignCenter, "OIL PRESS TEAM");
          y += 600;
          
          painter.setPen(QPen(Qt::black, 2));
          painter.setFont(QFont("Arial", 16, QFont::DemiBold));
          painter.drawText(QRect(0, y, 9000, 300), Qt::AlignCenter, "OFFICIAL LEAVE APPLICATION FORM");
          y += 800;

          // Body Content
          auto drawLine = [&](QString label, QString val) {
              painter.setFont(QFont("Arial", 11, QFont::Bold));
              painter.drawText(500, y, label);
              painter.setFont(QFont("Arial", 11, QFont::Normal));
              painter.drawText(3000, y, val);
              y += 400;
          };

          drawLine("EMPLOYEE ID (CIN):", cin);
          drawLine("EMPLOYEE NAME:", name);
          drawLine("PHONE CONTACT:", phone);
          drawLine("EMAIL ADDRESS:", email);
          drawLine("REQUEST DATE:", QDate::currentDate().toString("MMMM d, yyyy"));
          drawLine("LEAVE START:", sDate->text());
          drawLine("LEAVE END:", eDate->text());
          drawLine("REASON / NOTES:", reason->text().isEmpty() ? "Standard Absence" : reason->text());
          
          y += 1000;
          
          // Guidelines
          painter.setPen(QPen(Qt::gray));
          QFont italicFont("Arial", 9);
          italicFont.setItalic(true);
          painter.setFont(italicFont);
          painter.drawText(500, y, "NOTE: This request is subject to administrative approval via the Request Hub.");
          y += 1200;

          // Signature Area
          painter.setPen(QPen(Qt::black, 2));
          painter.drawLine(500, y, 4000, y);
          painter.drawLine(5000, y, 8500, y);
          y += 200;
          painter.setFont(QFont("Arial", 10));
          painter.drawText(500, y, "Employee Signature");
          painter.drawText(5000, y, "Authorized Admin Signature");

          painter.end();
          
          QMessageBox::information(nullptr, "PDF Generated", QString("Success! Document saved to your desktop:\n%1").arg(fileName));
          QDesktopServices::openUrl(QUrl::fromLocalFile(desktopPath));
      });

    } else if (name == "Request Hub") {
      QWidget *reqPage = new QWidget();
      QVBoxLayout *reqLayout = new QVBoxLayout(reqPage);
      reqLayout->setSpacing(15);
      
      QHBoxLayout *headerLine = new QHBoxLayout();
      QLabel *reqTitle = new QLabel("Global Absence & Leave Database");
      reqTitle->setStyleSheet("font-size: 20px; font-weight: 700; color: #1a1a1a;");
      
      QPushButton *btnRefresh = new QPushButton("Refresh List");
      btnRefresh->setFixedWidth(120);
      btnRefresh->setStyleSheet("QPushButton { background-color: #f3f4f6; color: #374151; border: 1px solid #d1d5db; border-radius: 4px; padding: 5px; font-weight: bold; } QPushButton:hover { background-color: #e5e7eb; }");
      
      headerLine->addWidget(reqTitle);
      headerLine->addStretch();
      headerLine->addWidget(btnRefresh);
      reqLayout->addLayout(headerLine);
      
      QTableWidget *reqTable = new QTableWidget();
      reqTable->setColumnCount(5);
      reqTable->setHorizontalHeaderLabels({"Employee CIN", "Year / Week", "Leave Date", "Status", "Actions"});
      reqTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      reqTable->verticalHeader()->setVisible(false);
      reqTable->setStyleSheet(
          "QTableWidget { border: 1px solid #e5e7eb; border-radius: 8px; background: white; gridline-color: #f3f4f6; }"
          "QHeaderView::section { background-color: #f9fafb; padding: 12px; border: none; border-bottom: 2px solid #e5e7eb; font-weight: bold; color: #4b5563; }"
      );
      
      std::function<void()> refreshRequests;
      refreshRequests = [reqTable, btnRefresh]() {
          reqTable->setRowCount(0);
          QSqlQuery qr("SELECT ID, ID_PERSONNEL, YEAR, WEEK_NUMBER, LEAVE_START, LEAVE_APPROVED, LEAVE_END FROM SHIFT_SCHEDULE WHERE LEAVE_START IS NOT NULL ORDER BY YEAR DESC, WEEK_NUMBER DESC, LEAVE_START DESC");
          int r = 0;
          while(qr.next()){
              reqTable->insertRow(r);
              reqTable->setItem(r, 0, new QTableWidgetItem(qr.value(1).toString()));
              reqTable->setItem(r, 1, new QTableWidgetItem(qr.value(2).toString() + " / Week " + qr.value(3).toString()));
              
              QDate startDate = qr.value(4).toDate();
              QDate endDate = qr.value(6).isNull() ? startDate : qr.value(6).toDate();
              QString dateDisplay;
              if (startDate != endDate) {
                  dateDisplay = startDate.toString("MMM d") + " - " + endDate.toString("MMM d, yyyy");
              } else {
                  dateDisplay = startDate.toString("ddd, MMM d, yyyy");
              }
              reqTable->setItem(r, 2, new QTableWidgetItem(dateDisplay));
              
              QString statusText = qr.value(5).toString().isEmpty() ? "PENDING" : qr.value(5).toString().toUpper();
              QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
              statusItem->setTextAlignment(Qt::AlignCenter);
              if (statusText == "APPROVED") statusItem->setForeground(QColor("#1D9E75"));
              else if (statusText == "REJECTED") statusItem->setForeground(QColor("#E24B4A"));
              else statusItem->setForeground(QColor("#EF9F27"));
              reqTable->setItem(r, 3, statusItem);
              
              QWidget *actionGroup = new QWidget();
              QHBoxLayout *aLayout = new QHBoxLayout(actionGroup);
              aLayout->setContentsMargins(4,4,4,4);
              aLayout->setSpacing(4);
              
              auto createActionBtn = [&](QString text, QString color, QString hover) {
                  QPushButton *b = new QPushButton(text);
                  b->setStyleSheet(QString("QPushButton { background: %1; color: white; border-radius: 4px; padding: 4px 8px; font-weight: bold; font-size: 11px; } QPushButton:hover { background: %2; }").arg(color, hover));
                  b->setCursor(Qt::PointingHandCursor);
                  return b;
              };

              QPushButton *btnApprove = createActionBtn("Approve", "#1D9E75", "#147b5b");
              QPushButton *btnReject = createActionBtn("Reject", "#EF9F27", "#d98e1f");
              QPushButton *btnDelete = createActionBtn("Delete", "#E24B4A", "#c53030");

              int id = qr.value(0).toInt();
              int cinId = qr.value(1).toInt();

              QObject::connect(btnApprove, &QPushButton::clicked, [id, cinId, startDate, endDate, btnRefresh](){
                  QSqlQuery up; up.prepare("UPDATE SHIFT_SCHEDULE SET LEAVE_APPROVED = 'Approved' WHERE ID = :id");
                  up.bindValue(":id", id); up.exec(); 
                  
                  // Enforce conflict removal: cancel all actual assigned shifts taking place within the approved leave window
                  QDate current = startDate;
                  while (current <= endDate) {
                      int cw = current.weekNumber();
                      int cy = current.year();
                      int cd = current.dayOfWeek() - 1; // Qt 1-7 (Mon-Sun) -> Database 0-6
                      QSqlQuery del; del.prepare("DELETE FROM SHIFT_SCHEDULE WHERE ID_PERSONNEL = :cin AND WEEK_NUMBER = :wk AND YEAR = :yr AND DAY_OF_WEEK = :day AND LEAVE_START IS NULL");
                      del.bindValue(":cin", cinId);
                      del.bindValue(":wk", cw);
                      del.bindValue(":yr", cy);
                      del.bindValue(":day", cd);
                      del.exec();
                      current = current.addDays(1);
                  }
                  btnRefresh->click();
              });
              QObject::connect(btnReject, &QPushButton::clicked, [id, btnRefresh](){
                  QSqlQuery up; up.prepare("UPDATE SHIFT_SCHEDULE SET LEAVE_APPROVED = 'Rejected' WHERE ID = :id");
                  up.bindValue(":id", id); up.exec(); btnRefresh->click();
              });
              QObject::connect(btnDelete, &QPushButton::clicked, [id, btnRefresh](){
                  if (QMessageBox::question(nullptr, "Confirm", "Delete this leave record permanently?") == QMessageBox::Yes) {
                      QSqlQuery del; del.prepare("DELETE FROM SHIFT_SCHEDULE WHERE ID = :id");
                      del.bindValue(":id", id); del.exec(); btnRefresh->click();
                  }
              });

              aLayout->addWidget(btnApprove);
              aLayout->addWidget(btnReject);
              aLayout->addWidget(btnDelete);
              reqTable->setCellWidget(r, 4, actionGroup);
              r++;
          }
      };
      
      QObject::connect(btnRefresh, &QPushButton::clicked, refreshRequests);
      refreshRequests();
      reqLayout->addWidget(reqTable);
      cLayout->addWidget(reqPage);
    }
    cLayout->addStretch();
    outNestedStack->addWidget(content);
  }

  actionLayout->addStretch();
  setupTabNavigation(tabButtons, outNestedStack);
  tabButtons.first()->setChecked(true);

  layout->addWidget(actionBar);
  layout->addWidget(outNestedStack);
  return page;
}

