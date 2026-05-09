#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QSqlQueryModel>
#include <QRegularExpressionValidator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QTextStream>
#include <QtPrintSupport/QPrinter>
#include <QTextDocument>
#include <QFileInfo>
#include <memory>
#include <functional>

#include "machine.h"
#include "generic_bar_chart.h"

// --- Style Helpers ---
static QString getLabelStyle() { return "font-size: 14px; font-weight: 700; color: #333; margin-top: 5px; margin-bottom: 3px;"; }
static QString getInputStyle() { return "QLineEdit, QComboBox { background-color: #fcfcfc; border: 1px solid #e0e0e0; border-radius: 6px; padding: 6px 10px; font-size: 13px; color: #333; min-height: 35px; } QLineEdit:focus, QComboBox:focus { border: 2px solid #3DDC84; background-color: #ffffff; }"; }
static QString getButtonStyle() { return "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3DDC84, stop:1 #2DB66F); color: #FFFFFF; border: none; border-radius: 8px; padding: 12px 24px; font-size: 14px; font-weight: 700; } QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4EED95, stop:1 #3DDC84); }"; }
static QString getTabButtonStyle() { return "QPushButton { background-color: #ffffff; color: #555; border: 1px solid #ddd; border-radius: 6px; padding: 8px 16px; font-size: 13px; font-weight: 600; } QPushButton:checked { background-color: #e6f9ef; color: #3DDC84; border-color: #3DDC84; }"; }

static void setupTabNavigation(const QList<QPushButton *> &buttons, QStackedWidget *stack) {
    for (int i = 0; i < buttons.size(); ++i) {
        QObject::connect(buttons[i], &QPushButton::clicked, [buttons, stack, i]() {
            stack->setCurrentIndex(i);
            for (auto *btn : buttons) btn->setChecked(btn == buttons[i]);
        });
    }
}

// --- Main Page Factory ---
static QWidget *createMaintenancePage(QStackedWidget *&outNestedStack) {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(40, 40, 40, 40);
  layout->setSpacing(25);
  layout->addWidget(new QLabel("Maintenance Management - Machines"));

  QWidget *actionBar = new QWidget();
  QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
  actionLayout->setSpacing(12);

  outNestedStack = new QStackedWidget();
  QStringList tabNames = {"Add Asset", "Asset Hub", "Analytics"};
  QList<QPushButton *> tabButtons;

  auto machineTablePtr = std::make_shared<QTableWidget *>(nullptr);
  auto refreshMachineTable = std::make_shared<std::function<void()>>();

  *refreshMachineTable = [machineTablePtr, refreshMachineTable]() {
    QTableWidget *table = *machineTablePtr;
    if (!table) return;

    Machine m;
    QSqlQueryModel *model = m.afficher();

    table->setRowCount(0);
    int rows = model->rowCount();
    table->setRowCount(rows);

    for (int i = 0; i < rows; ++i) {
      int mid = model->data(model->index(i, 0)).toInt();
      table->setItem(i, 0, new QTableWidgetItem(QString::number(mid)));
      table->setItem(i, 1, new QTableWidgetItem(model->data(model->index(i, 1)).toString()));
      table->setItem(i, 2, new QTableWidgetItem(model->data(model->index(i, 2)).toString()));
      table->setItem(i, 3, new QTableWidgetItem(model->data(model->index(i, 3)).toString()));
      table->setItem(i, 4, new QTableWidgetItem(model->data(model->index(i, 4)).toString()));
      table->setItem(i, 5, new QTableWidgetItem(model->data(model->index(i, 5)).toString()));
      table->setItem(i, 6, new QTableWidgetItem(model->data(model->index(i, 6)).toString()));

      QWidget *actionWidget = new QWidget();
      QHBoxLayout *al = new QHBoxLayout(actionWidget);
      al->setContentsMargins(5, 2, 5, 2); al->setSpacing(8);

      QPushButton *btnMod = new QPushButton("Edit");
      btnMod->setStyleSheet("QPushButton { background-color: #ffffff; border: 1px solid #cccccc; border-radius: 6px; padding: 0px 8px; font-weight: 600; font-size: 13px; color: #333333; }");
      
      QPushButton *btnDel = new QPushButton("Remove");
      btnDel->setStyleSheet("QPushButton { background-color: #ffffff; border: 1px solid #d32f2f; color: #d32f2f; border-radius: 6px; padding: 0px 8px; font-weight: 600; font-size: 13px; }");

      al->addWidget(btnMod); al->addWidget(btnDel);
      table->setCellWidget(i, 7, actionWidget);

      QObject::connect(btnMod, &QPushButton::clicked, [table, mid, refreshMachineTable]() {
          int row = -1;
          for(int r=0; r<table->rowCount(); ++r) if(table->item(r, 0)->text().toInt() == mid) { row = r; break; }
          if(row == -1) return;

          QDialog dlg(table->window());
          dlg.setWindowTitle("Modify Machine");
          dlg.setMinimumWidth(400);
          QVBoxLayout *mainV = new QVBoxLayout(&dlg);
          mainV->setContentsMargins(30, 30, 30, 30);
          mainV->setSpacing(15);

          auto addField = [&](const QString &lblText, const QString &val) {
              mainV->addWidget(new QLabel(lblText));
              QLineEdit *le = new QLineEdit(val);
              le->setStyleSheet("QLineEdit { background-color: #f9fafb; border: 1px solid #eaeaea; border-radius: 8px; padding: 10px; font-size: 14px; }");
              mainV->addWidget(le);
              return le;
          };

          QLineEdit *edName = addField("Name:", table->item(row, 1)->text());
          QComboBox *edType = new QComboBox(); edType->addItems({"press", "filter"}); edType->setCurrentText(table->item(row, 2)->text());
          mainV->addWidget(new QLabel("Type:")); mainV->addWidget(edType);
          QComboBox *edStatus = new QComboBox(); edStatus->addItems({"Normal", "En panne", "En maintenance"}); edStatus->setCurrentText(table->item(row, 3)->text());
          mainV->addWidget(new QLabel("Status:")); mainV->addWidget(edStatus);
          QLineEdit *edHours = addField("Hours:", table->item(row, 4)->text());
          QLineEdit *edSeuil = addField("Threshold:", table->item(row, 5)->text());
          QLineEdit *edLoc = addField("Location:", table->item(row, 6)->text());

          QPushButton *btnSave = new QPushButton("Save Changes");
          btnSave->setStyleSheet("QPushButton { background-color: #3DDC84; color: white; border: none; border-radius: 8px; padding: 12px; font-weight: 700; }");
          mainV->addWidget(btnSave);

          QObject::connect(btnSave, &QPushButton::clicked, [=, &dlg]() {
              Machine updateObj(mid, edName->text().trimmed(), edType->currentText(), edStatus->currentText(), edHours->text().toInt(), edSeuil->text().toInt(), edLoc->text().trimmed());
              if (updateObj.modifier()) { dlg.accept(); (*refreshMachineTable)(); }
          });
          dlg.exec();
      });

      QObject::connect(btnDel, &QPushButton::clicked, [table, mid, refreshMachineTable]() {
          if (QMessageBox::warning(table->window(), "Confirm", "Delete machine?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
              Machine deleteObj; if (deleteObj.supprimer(mid)) (*refreshMachineTable)();
          }
      });
    }
    delete model;
  };

  for (const auto &name : tabNames) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(getTabButtonStyle());
    actionLayout->addWidget(btn);
    tabButtons.append(btn);

    QWidget *content = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout(content);

    if (name == "Add Asset") {
      QWidget *formContainer = new QWidget();
      QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
      formLayout->setSpacing(10);

      QLineEdit *nameInput = new QLineEdit();
      QComboBox *typeInput = new QComboBox(); typeInput->addItems({"press", "filter"});
      QComboBox *statusInput = new QComboBox(); statusInput->addItems({"Normal", "En panne", "En maintenance"});
      QLineEdit *hoursInput = new QLineEdit("0");
      QLineEdit *seuilInput = new QLineEdit("100");
      QLineEdit *locInput = new QLineEdit();

      auto addInput = [&](const QString &txt, QWidget *le) {
          QLabel *l = new QLabel(txt);
          l->setStyleSheet(getLabelStyle());
          le->setStyleSheet(getInputStyle());
          formLayout->addWidget(l);
          formLayout->addWidget(le);
      };
      addInput("Machine Name:", nameInput);
      addInput("Machine Type:", typeInput);
      addInput("Machine Status:", statusInput);
      addInput("Operating Hours:", hoursInput);
      addInput("Maintenance Threshold:", seuilInput);
      addInput("Machine Location:", locInput);

      QPushButton *btnAdd = new QPushButton("Add Machine Asset");
      btnAdd->setStyleSheet(getButtonStyle());
      btnAdd->setFixedHeight(45);
      formLayout->addWidget(btnAdd);
      formLayout->addStretch();
      
      cLayout->addWidget(formContainer);

      QObject::connect(btnAdd, &QPushButton::clicked, [=]() {
          QString nom = nameInput->text().trimmed();
          if (nom.isEmpty()) return;
          Machine newM(0, nom, typeInput->currentText(), statusInput->currentText(), hoursInput->text().toInt(), seuilInput->text().toInt(), locInput->text().trimmed());
          if (newM.ajouter()) { 
              QMessageBox::information(nullptr, "Success", "Machine added.");
              (*refreshMachineTable)(); 
              outNestedStack->setCurrentIndex(1); 
              tabButtons[1]->setChecked(true); 
          }
      });
    } else if (name == "Asset Hub") {
      QWidget *ctrlBar = new QWidget();
      QHBoxLayout *ctrl = new QHBoxLayout(ctrlBar);
      QPushButton *btnRefresh = new QPushButton("Refresh List"); btnRefresh->setStyleSheet(getButtonStyle());
      QPushButton *btnAlert = new QPushButton("Run Safety Check"); btnAlert->setStyleSheet("QPushButton { background-color: #f39c12; color: white; border-radius: 8px; padding: 10px; font-weight: 700; }");
      QPushButton *btnPrint = new QPushButton("EXPORT PDF"); btnPrint->setStyleSheet(getButtonStyle());
      
      ctrl->addWidget(btnRefresh); ctrl->addSpacing(10);
      ctrl->addWidget(btnAlert); ctrl->addStretch();
      ctrl->addWidget(btnPrint);
      cLayout->addWidget(ctrlBar);

      QTableWidget *table = new QTableWidget();
      *machineTablePtr = table;
      table->setColumnCount(8);
      table->setHorizontalHeaderLabels({"ID", "Name", "Type", "Status", "Hours", "Threshold", "Location", "Actions"});
      table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      table->setStyleSheet("QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; border-radius: 8px; }");
      cLayout->addWidget(table);

      QObject::connect(btnRefresh, &QPushButton::clicked, [refreshMachineTable]() { (*refreshMachineTable)(); });
      
      QObject::connect(btnAlert, &QPushButton::clicked, [table, refreshMachineTable]() {
          QString alertMsg; bool hasAlerts = false;
          for (int i = 0; i < table->rowCount(); ++i) {
             int hrs = table->item(i, 4)->text().toInt();
             int seuil = table->item(i, 5)->text().toInt();
             if (hrs >= seuil && seuil > 0) {
                alertMsg += "Asset " + table->item(i, 1)->text() + " reached threshold.\n";
                hasAlerts = true;
                for(int c=0; c<7; ++c) table->item(i, c)->setBackground(QColor(255, 230, 230));
             }
          }
          if (hasAlerts) QMessageBox::warning(nullptr, "Maintenance Required", alertMsg);
          else QMessageBox::information(nullptr, "System Check", "All assets are within safe operating hours.");
      });

      QObject::connect(btnPrint, &QPushButton::clicked, [table]() {
          QString fileName = QFileDialog::getSaveFileName(nullptr, "Save Report", "Maintenance_Report.pdf", "PDF (*.pdf)");
          if (fileName.isEmpty()) return;
          QPrinter printer(QPrinter::HighResolution);
          printer.setOutputFormat(QPrinter::PdfFormat);
          printer.setOutputFileName(fileName);
          QTextDocument doc;
          QString html = "<h1>Maintenance Assets Report</h1><table border='1' width='100%'><tr><th>Name</th><th>Type</th><th>Status</th><th>Hours</th></tr>";
          for(int i=0; i<table->rowCount(); ++i) {
              html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
                      .arg(table->item(i,1)->text(), table->item(i,2)->text(), table->item(i,3)->text(), table->item(i,4)->text());
          }
          html += "</table>";
          doc.setHtml(html);
          doc.print(&printer);
          QMessageBox::information(nullptr, "PDF Export", "Report generated successfully.");
      });
    } else if (name == "Analytics") {
      GenericBarChart *chart = new GenericBarChart("Asset Health Distribution");
      cLayout->addWidget(chart);
      auto loadAnalytics = [chart]() {
          chart->clearBars();
          Machine m; QSqlQueryModel *model = m.afficher();
          int n=0, p=0, mn=0;
          for(int i=0; i<model->rowCount(); ++i) {
              QString s = model->data(model->index(i, 3)).toString();
              if(s=="Normal") n++; else if(s=="En panne") p++; else mn++;
          }
          chart->addBar("Functional", n, QColor(46, 204, 113));
          chart->addBar("Critical", p, QColor(231, 76, 60));
          chart->addBar("In Service", mn, QColor(241, 196, 15));
          delete model;
      };
      loadAnalytics();
      QPushButton *btnUpdate = new QPushButton("Refresh Statistics");
      btnUpdate->setStyleSheet(getButtonStyle());
      cLayout->addWidget(btnUpdate);
      QObject::connect(btnUpdate, &QPushButton::clicked, loadAnalytics);
    }
    cLayout->addStretch();
    outNestedStack->addWidget(content);
  }
  setupTabNavigation(tabButtons, outNestedStack);
  tabButtons.first()->setChecked(true);
  layout->addWidget(actionBar);
  layout->addWidget(outNestedStack);
  return page;
}
