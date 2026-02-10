#include "mainwindow.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLinearGradient>
#include <QPalette>
#include <QPixmap>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QDateTime>
#include <QScrollArea>
#include <QPainter>
#include <QFileDialog>
#include <QMap>

// --- Mock Chart Widget ---
class MockBarChart : public QWidget {
public:
    explicit MockBarChart(const QString &title, const QMap<QString, int> &data, QWidget *parent = nullptr)
        : QWidget(parent), m_title(title), m_data(data) {
        setMinimumHeight(250);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Background
        painter.setBrush(QColor("#ffffff"));
        painter.setPen(QPen(QColor("#e0e0e0"), 1));
        painter.drawRoundedRect(rect().adjusted(1,1,-1,-1), 10, 10);

        // Title
        painter.setPen(QColor("#333333"));
        QFont titleFont = font();
        titleFont.setBold(true);
        titleFont.setPointSize(12);
        painter.setFont(titleFont);
        painter.drawText(QRect(20, 15, width()-40, 30), Qt::AlignLeft | Qt::AlignVCenter, m_title);

        if (m_data.isEmpty()) return;

        // Bars
        int margin = 40;
        int barWidth = (width() - 2 * margin) / m_data.size() - 30;
        if (barWidth < 10) barWidth = 10;

        int maxVal = 0;
        for (int v : m_data) if (v > maxVal) maxVal = v;
        if (maxVal == 0) maxVal = 1;

        int x = margin;
        int bottomY = height() - margin;
        int plotHeight = height() - 2 * margin - 30;

        QColor barColor("#3DDC84");
        painter.setFont(QFont("Segoe UI", 9));

        int i = 0;
        for (auto it = m_data.begin(); it != m_data.end(); ++it) {
            int h = (it.value() * plotHeight) / maxVal;
            QRect barRect(x, bottomY - h, barWidth, h);

            // Draw Bar
            painter.setBrush(barColor);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(barRect, 4, 4);

            // Label (X-axis)
            painter.setPen(QColor("#666666"));
            painter.drawText(QRect(x - 5, bottomY + 5, barWidth + 10, 20), Qt::AlignCenter, it.key());

            // Value (Top of bar)
            painter.setPen(QColor("#333333"));
            painter.drawText(QRect(x, bottomY - h - 20, barWidth, 20), Qt::AlignCenter, QString::number(it.value()));

            x += barWidth + 30;
            i++;
        }
    }

private:
    QString m_title;
    QMap<QString, int> m_data;
};

// --- Style Helpers ---

static QString getLabelStyle() {
    return "font-size: 14px; font-weight: 700; color: #333; margin-top: 10px; margin-bottom: 6px;";
}



static QString getInputStyle() {
    return "QLineEdit {"
           "background-color: #fcfcfc;"
           "border: 1px solid #e0e0e0;"
           "border-radius: 8px;"
           "padding: 10px 14px;"
           "font-size: 14px;"
           "color: #333;"
           "min-height: 45px;" /* Fix for squashed inputs */
           "}"
           "QLineEdit:focus {"
           "border: 2px solid #3DDC84;"
           "background-color: #ffffff;"
           "outline: none;"
           "}";
}

static QString getButtonStyle() {
    return "QPushButton {"
           "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3DDC84, stop:1 #2DB66F);"
           "color: #FFFFFF;"
           "border: none;"
           "border-radius: 8px;"
           "padding: 12px 24px;"
           "font-size: 14px;"
           "font-weight: 700;"
           "} "
           "QPushButton:hover {"
           "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4EED95, stop:1 #3DDC84);"
           "margin-top: -1px;" // Subtle lift effect
           "} "
           "QPushButton:pressed {"
           "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2DB66F, stop:1 #228B5A);"
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
           "QPushButton:checked { background-color: #e6f9ef; color: #3DDC84; border-color: #3DDC84; }"
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

static QWidget* createStatCard(const QString &title, const QString &value, const QString &change, const QString &color) {
    QWidget *card = new QWidget();
    card->setStyleSheet(
        "QWidget { background-color: #ffffff; border-radius: 12px; border: 1px solid #f0f0f0; }"
        "QLabel { border: none; background: transparent; }"
        );
    card->setMinimumHeight(100);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("font-size: 13px; color: #888; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px;");

    QLabel *lblValue = new QLabel(value);
    lblValue->setStyleSheet("font-size: 28px; color: #1a1a1a; font-weight: 800; margin-top: 5px;");

    QLabel *lblChange = new QLabel(change);
    lblChange->setStyleSheet("font-size: 12px; color: " + color + "; font-weight: 700; margin-top: 5px;");

    layout->addWidget(lblTitle);
    layout->addWidget(lblValue);
    layout->addWidget(lblChange);
    layout->addStretch();

    // Add a colored bottom strip
    QWidget *strip = new QWidget();
    strip->setFixedHeight(4);
    strip->setStyleSheet(QString("background-color: %1; border-radius: 2px;").arg(color));
    layout->addWidget(strip);

    return card;
}

static QWidget* createStyledForm(const QString &title, const QList<QPair<QString, QString>> &fields, const QString &submitText) {
    QWidget *formContainer = new QWidget();
    formContainer->setStyleSheet(".QWidget { background-color: #ffffff; border-radius: 10px; border: 1px solid #eee; }");

    QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
    outerLayout->setContentsMargins(30, 30, 30, 30);

    QWidget *formContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(formContent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(15);

    if (!title.isEmpty()) {
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; margin-bottom: 25px; border: none;");
        layout->addWidget(titleLabel);
    }

    QString labelStyle = getLabelStyle();
    QString inputStyle = getInputStyle();

    for (const auto &field : fields) {
        QLabel *lbl = new QLabel(field.first);
        lbl->setStyleSheet(labelStyle);
        QLineEdit *input = new QLineEdit();
        input->setStyleSheet(inputStyle);
        input->setPlaceholderText(field.second);

        layout->addWidget(lbl);
        layout->addWidget(input);
    }

    if (!submitText.isEmpty()) {
        layout->addSpacing(20);
        QPushButton *btnSubmit = new QPushButton(submitText);
        btnSubmit->setStyleSheet(getButtonStyle());
        btnSubmit->setCursor(Qt::PointingHandCursor);
        btnSubmit->setFixedHeight(45);
        layout->addWidget(btnSubmit);
    }

    layout->addStretch();
    outerLayout->addWidget(formContent);
    return formContainer;
}

// --- Auth Helpers ---



static QWidget* createStyledTable(const QString &title, const QStringList &headers, const QVector<QStringList> &data, bool showActions = false, bool showQrAction = false) {
    QWidget *tableWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tableWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    if (!title.isEmpty()) {
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: #1a1a1a; margin-bottom: 15px;");
        layout->addWidget(titleLabel);
    }

    QStringList finalHeaders = headers;
    if (showActions) {
        finalHeaders << "Actions";
    }

    QTableWidget *table = new QTableWidget();
    table->setColumnCount(finalHeaders.size());
    table->setHorizontalHeaderLabels(finalHeaders);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(60); // Enforce taller rows for buttons
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setStyleSheet(
        "QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; gridline-color: transparent; border-radius: 8px; alternate-background-color: #f9fafb; }"
        "QHeaderView::section { background-color: #ffffff; padding: 12px; border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; color: #666; text-transform: uppercase; font-size: 12px; }"
        "QTableWidget::item { padding: 12px; border-bottom: 1px solid #f5f5f5; color: #333; }"
        "QTableWidget::item:selected { background-color: #e6f9ef; color: #1a1a1a; }"
        );

    table->setRowCount(data.size());
    for (int i = 0; i < data.size(); ++i) {
        for (int j = 0; j < data[i].size(); ++j) {
            table->setItem(i, j, new QTableWidgetItem(data[i][j]));
        }

        if (showActions) {
            QWidget *actionWidget = new QWidget();
            QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
            actionLayout->setContentsMargins(0, 5, 0, 5); // Add top/bottom padding
            actionLayout->setSpacing(5);

            QPushButton *btnEdit = new QPushButton("Edit");
            btnEdit->setCursor(Qt::PointingHandCursor);
            btnEdit->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; border-radius: 4px; padding: 6px 10px; font-size: 11px; font-weight: bold; } QPushButton:hover { background-color: #2980b9; }");

            QPushButton *btnDelete = new QPushButton("Delete");
            btnDelete->setCursor(Qt::PointingHandCursor);
            btnDelete->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border: none; border-radius: 4px; padding: 6px 10px; font-size: 11px; font-weight: bold; } QPushButton:hover { background-color: #c0392b; }");

            actionLayout->addWidget(btnEdit);
            actionLayout->addWidget(btnDelete);

            if (showQrAction) {
                QPushButton *btnQr = new QPushButton("QR Code");
                btnQr->setCursor(Qt::PointingHandCursor);
                btnQr->setStyleSheet("QPushButton { background-color: #9b59b6; color: white; border: none; border-radius: 4px; padding: 6px 10px; font-size: 11px; font-weight: bold; } QPushButton:hover { background-color: #8e44ad; }");
                QObject::connect(btnQr, &QPushButton::clicked, [=]() {
                    QMessageBox::information(nullptr, "QR Code", "Generate QR Code for " + data[i][1]);
                });
                actionLayout->addWidget(btnQr);
            }

            actionLayout->addStretch();

            table->setCellWidget(i, finalHeaders.size() - 1, actionWidget);
        }
    }

    layout->addWidget(table);
    return tableWidget;
}

// --- Specific Page Factories ---

static void setupTabNavigation(const QList<QPushButton*> &buttons, QStackedWidget *stack) {
    for (int i = 0; i < buttons.size(); ++i) {
        QPushButton* btn = buttons[i];
        QObject::connect(btn, &QPushButton::clicked, [=]() {
            stack->setCurrentIndex(i);
            for(auto *b : buttons) b->setChecked(b == btn);
        });
    }
}



// --- New Modules from Integration ---

static QWidget* createOrderPage(QStackedWidget* &outNestedStack) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(25);

    layout->addWidget(new QLabel("Order Management"));

    QWidget *actionBar = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setSpacing(12);

    outNestedStack = new QStackedWidget();
    QStringList tabNames = {"Add Order", "Edit Order", "Order History", "Statistics"};
    QList<QPushButton*> tabButtons;

    for (const auto &name : tabNames) {
        QPushButton *btn = new QPushButton(name);
        btn->setCheckable(true);
        btn->setStyleSheet(getTabButtonStyle());
        actionLayout->addWidget(btn);
        tabButtons.append(btn);

        QWidget *content = new QWidget();
        QVBoxLayout *cLayout = new QVBoxLayout(content);

        if (name == "Add Order") {
            cLayout->addWidget(createStyledForm("New Order Registration", {
                                                                              {"Company Name:", "Enter company name"},
                                                                              {"Contact Person:", "Enter contact person name"},
                                                                              {"Email Address:", "email@example.com"},
                                                                              {"Phone Number:", "+1 (555) ..."},
                                                                              {"Address:", "Street, City, Zip"}
                                                                          }, "Register Client"));
        } else if (name == "Edit Order") {
            QWidget *searchBar = new QWidget();
            searchBar->setStyleSheet(getCardStyle());
            QHBoxLayout *sLayout = new QHBoxLayout(searchBar);
            QLineEdit *searchInp = new QLineEdit();
            searchInp->setStyleSheet("border: none; font-size: 14px;");
            searchInp->setPlaceholderText("Search Client ID or Name...");
            QPushButton *sBtn = new QPushButton("Search");
            sBtn->setStyleSheet(getButtonStyle());
            sBtn->setFixedWidth(100);
            sLayout->addWidget(searchInp);
            sLayout->addWidget(sBtn);
            cLayout->addWidget(searchBar);
            cLayout->addSpacing(15);
            cLayout->addWidget(createStyledForm("Edit Details", {{"Company Name:", "Acme Corp"}, {"Email:", "contact@acme.com"}}, "Update Profile"));
        } else if (name == "Order History") {
            cLayout->addWidget(createStyledTable("Client Interaction History", {"Date", "Client", "Action", "Notes"}, {
                                                                                                                          {"2023-10-25", "Acme Corp", "Meeting", "Discussed Q4 Renewal"},
                                                                                                                          {"2023-10-24", "Globex Inc", "Support", "Fixed login API issue"},
                                                                                                                          {"2023-10-22", "Soylent Corp", "Order", "Placed bulk order #992"},
                                                                                                                          {"2023-10-20", "Umbrella Corp", "Call", "Security audit check"}
                                                                                                                      }, true));
        } else {
            // Container for statistics page to hold button + table
            QWidget *statsPageWidget = new QWidget();
            QVBoxLayout *statsPageLayout = new QVBoxLayout(statsPageWidget);
            statsPageLayout->setContentsMargins(0,0,0,0);
            statsPageLayout->setSpacing(10);

            QPushButton *btnPrint = new QPushButton("PRINT PDF");
            btnPrint->setStyleSheet(getButtonStyle());
            btnPrint->setFixedWidth(150);
            statsPageLayout->addWidget(btnPrint, 0, Qt::AlignRight);

            QWidget *tableWidget = createStyledTable("Statistics", {"Period", "New Clients", "Churn Rate", "Revenue"}, {
                                                                                                                           {"Q3 2023", "45", "2.1%", "$120,000"},
                                                                                                                           {"Q2 2023", "52", "1.5%", "$145,000"}
                                                                                                                       });

            statsPageLayout->addWidget(tableWidget);
            cLayout->addWidget(statsPageWidget);
        }
        cLayout->addStretch();
        outNestedStack->addWidget(content);
    }

    actionLayout->addStretch();
    setupTabNavigation(tabButtons, outNestedStack);
    tabButtons.first()->setChecked(true); // Default

    layout->addWidget(actionBar);
    layout->addWidget(outNestedStack);
    return page;
}

static QWidget* createFinancePage(QStackedWidget* &outNestedStack) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(25);
    layout->addWidget(new QLabel("Financial Management"));

    QWidget *actionBar = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setSpacing(12);

    outNestedStack = new QStackedWidget();
    QStringList tabNames = {"Create Invoice", "View Transactions", "Expense Tracking", "Financial Reports"};
    QList<QPushButton*> tabButtons;

    for (const auto &name : tabNames) {
        QPushButton *btn = new QPushButton(name);
        btn->setCheckable(true);
        btn->setStyleSheet(getTabButtonStyle());
        actionLayout->addWidget(btn);
        tabButtons.append(btn);

        QWidget *content = new QWidget();
        QVBoxLayout *cLayout = new QVBoxLayout(content);

        if (name == "Create Invoice") {
            cLayout->addWidget(createStyledForm("Invoice Creation Form", {{"Customer Name:", "Enter customer name"}, {"Amount:", "0.00"}, {"Date:", "YYYY-MM-DD"}}, "Submit Invoice"));
        } else if (name == "View Transactions") {
            cLayout->addWidget(createStyledTable("Recent Transactions", {"ID", "Date", "Description", "Amount", "Status"}, {
                                                                                                                               {"TRX-1024", "2023-10-25", "Office Supplies", "$150.00", "Completed"},
                                                                                                                               {"TRX-1023", "2023-10-24", "Client Payment - Acme", "$1,200.00", "Completed"},
                                                                                                                               {"TRX-1022", "2023-10-24", "Server Hosting", "$340.50", "Pending"}
                                                                                                                           }, true));
        } else if (name == "Expense Tracking") {
            cLayout->addWidget(createStyledTable("Expense Log", {"Date", "Category", "Description", "Amount"}, {
                                                                                                                   {"2023-10-25", "Travel", "Flight to NY Conference", "$450.00"},
                                                                                                                   {"2023-10-24", "Meals", "Team Lunch", "$125.00"}
                                                                                                               }, true));
        } else {
            cLayout->addWidget(createStyledTable("Financial Summary (YTD)", {"Metric", "Value", "Change"}, {
                                                                                                               {"Total Revenue", "$1,250,500", "+15%"},
                                                                                                               {"Total Expenses", "$850,000", "+5%"}
                                                                                                           }));
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

static QWidget* createInventoryPage(QStackedWidget* &outNestedStack) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(25);
    layout->addWidget(new QLabel("Inventory Management"));

    QWidget *actionBar = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setSpacing(12);

    outNestedStack = new QStackedWidget();
    QStringList tabNames = {"Add Stock Item", "Update Stock Levels", "Supplier Management", "Inventory Reports"};
    QList<QPushButton*> tabButtons;

    for (const auto &name : tabNames) {
        QPushButton *btn = new QPushButton(name);
        btn->setCheckable(true);
        btn->setStyleSheet(getTabButtonStyle());
        actionLayout->addWidget(btn);
        tabButtons.append(btn);

        QWidget *content = new QWidget();
        QVBoxLayout *cLayout = new QVBoxLayout(content);

        if (name == "Add Stock Item") {
            cLayout->addWidget(createStyledForm("Add New Inventory Item", {{"Item Name:", "Widget A"}, {"SKU:", "WGT-001"}}, "Add Item"));
        } else if (name == "Update Stock Levels") {
            cLayout->addWidget(createStyledForm("Adjust Stock Quantity", {{"SKU / Item ID:", "Scan or type SKU"}, {"Quantity:", "0"}}, "Update Stock"));
        } else if (name == "Supplier Management") {
            cLayout->addWidget(createStyledTable("Supplier Directory", {"Supplier Name", "Contact", "Phone", "Rating"}, {
                                                                                                                            {"Alpha Supplies", "Alice Johnson", "(555) 123-4567", "5/5"},
                                                                                                                            {"Beta Logistics", "Bob Smith", "(555) 987-6543", "4/5"}
                                                                                                                        }, true));
        } else {
            cLayout->addWidget(createStyledTable("Low Stock Alert", {"Item Name", "SKU", "Current Qty", "Status"}, {
                                                                                                                       {"Printer Paper", "OFF-001", "5", "CRITICAL"},
                                                                                                                       {"HDMI Cables", "CBL-202", "12", "LOW"}
                                                                                                                   }));
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

static QWidget* createMaintenancePage(QStackedWidget* &outNestedStack) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(25);
    layout->addWidget(new QLabel("Maintenance Management"));

    QWidget *actionBar = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setSpacing(12);

    outNestedStack = new QStackedWidget();
    QStringList tabNames = {"Schedule Maintenance", "Work Orders", "Equipment Status", "Maintenance Logs"};
    QList<QPushButton*> tabButtons;

    for (const auto &name : tabNames) {
        QPushButton *btn = new QPushButton(name);
        btn->setCheckable(true);
        btn->setStyleSheet(getTabButtonStyle());
        actionLayout->addWidget(btn);
        tabButtons.append(btn);

        QWidget *content = new QWidget();
        QVBoxLayout *cLayout = new QVBoxLayout(content);

        if (name == "Schedule Maintenance") {
            cLayout->addWidget(createStyledForm("Schedule Maintenance Task", {{"Equipment Name:", "Conveyor Belt A"}, {"Scheduled Date:", "YYYY-MM-DD"}}, "Schedule Task"));
        } else if (name == "Work Orders") {
            cLayout->addWidget(createStyledTable("Active Work Orders", {"Order ID", "Equipment", "Issue", "Status"}, {
                                                                                                                         {"WO-101", "Hydraulic Press", "Oil Leak", "In Progress"},
                                                                                                                         {"WO-102", "Forklift 3", "Battery Replace", "Pending"}
                                                                                                                     }, true));
        } else if (name == "Equipment Status") {
            cLayout->addWidget(createStyledTable("Equipment Status Overview", {"Equipment ID", "Name", "Location", "Status"}, {
                                                                                                                                  {"EQ-001", "CNC Machine", "Floor 1", "Operational"},
                                                                                                                                  {"EQ-002", "Lathe A", "Floor 1", "Maintenance"}
                                                                                                                              }, true));
        } else {
            cLayout->addWidget(createStyledTable("Historical Maintenance Logs", {"Date", "Equipment", "Action", "Result"}, {
                                                                                                                               {"2023-09-01", "EQ-001", "Belt Replacement", "Fixed"}
                                                                                                                           }, true));
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

static QWidget* createProductPage(QStackedWidget* &outNestedStack) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(25);
    layout->addWidget(new QLabel("Product Management"));

    QWidget *actionBar = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setSpacing(12);

    outNestedStack = new QStackedWidget();
    QStringList tabNames = {"Product List", "Add Item", "Edit Item", "Remove Item"};
    QList<QPushButton*> tabButtons;

    for (const auto &name : tabNames) {
        QPushButton *btn = new QPushButton(name);
        btn->setCheckable(true);
        btn->setStyleSheet(getTabButtonStyle());
        actionLayout->addWidget(btn);
        tabButtons.append(btn);

        QWidget *content = new QWidget();
        QVBoxLayout *cLayout = new QVBoxLayout(content);

        if (name == "Product List") {
            QWidget *listPageWidget = new QWidget();
            QVBoxLayout *listPageLayout = new QVBoxLayout(listPageWidget);
            listPageLayout->setContentsMargins(0,0,0,0);
            listPageLayout->setSpacing(10);

            QPushButton *btnPrint = new QPushButton("PRINT PDF");
            btnPrint->setStyleSheet(getButtonStyle());
            btnPrint->setFixedWidth(150);
            listPageLayout->addWidget(btnPrint, 0, Qt::AlignRight);

            QWidget *tableWidget = createStyledTable("Current Product Catalog",
                                                     {"ID Container", "Date Pressage", "Capacité", "Ref Testeur", "Qualité", "Viscosity", "Color", "Ref Press"}, {
                                                         {"CONT-001", "2023-10-25", "500L", "TEST-A1", "Premium", "0.85", "Golden", "PRESS-X1"},
                                                         {"CONT-002", "2023-10-26", "200L", "TEST-B2", "Standard", "0.90", "Yellow", "PRESS-X2"},
                                                         {"CONT-003", "2023-10-27", "1000L", "TEST-A1", "Premium", "0.84", "Golden", "PRESS-X1"}
                                                     }, true);

            listPageLayout->addWidget(tableWidget);
            cLayout->addWidget(listPageWidget);
        } else if (name == "Add Item") {
            cLayout->addWidget(createStyledForm("Add New Product", {
                                                                       {"ID Container:", "Enter Container ID"},
                                                                       {"Date Pressage:", "YYYY-MM-DD"},
                                                                       {"Capacité:", "e.g., 500L"},
                                                                       {"Ref Testeur:", "Enter Tester Ref"},
                                                                       {"Qualité:", "Enter Quality Grade"},
                                                                       {"Viscosity:", "Enter Viscosity"},
                                                                       {"Color:", "Enter Color"},
                                                                       {"Ref Press:", "Enter Press Ref"}
                                                                   }, "Add Product"));
        } else if (name == "Edit Item") {
            QWidget *searchBar = new QWidget();
            searchBar->setStyleSheet(getCardStyle());
            QHBoxLayout *sLayout = new QHBoxLayout(searchBar);
            QLineEdit *searchInp = new QLineEdit();
            searchInp->setStyleSheet("border: none; font-size: 14px;");
            searchInp->setPlaceholderText("Search Product ID...");
            QPushButton *sBtn = new QPushButton("Search");
            sBtn->setStyleSheet(getButtonStyle());
            sBtn->setFixedWidth(100);
            sLayout->addWidget(searchInp);
            sLayout->addWidget(sBtn);
            cLayout->addWidget(searchBar);
            cLayout->addSpacing(15);

            cLayout->addWidget(createStyledForm("Edit Product Details", {
                                                                            {"ID Container:", "CONT-001"},
                                                                            {"Date Pressage:", "2023-10-25"},
                                                                            {"Capacité:", "500L"},
                                                                            {"Ref Testeur:", "TEST-A1"},
                                                                            {"Qualité:", "Premium"},
                                                                            {"Viscosity:", "0.85"},
                                                                            {"Color:", "Golden"},
                                                                            {"Ref Press:", "PRESS-X1"}
                                                                        }, "Update Product"));
        } else if (name == "Remove Item") {
            QWidget *removeContainer = new QWidget();
            QVBoxLayout *rLayout = new QVBoxLayout(removeContainer);
            rLayout->setContentsMargins(0,0,0,0);
            rLayout->addWidget(createStyledForm("Remove Product from Inventory", {
                                                                                     {"ID Container:", "Enter Container ID to Remove"}
                                                                                 }, "Delete Product Permanently"));
            QLabel *warning = new QLabel("Warning: This action cannot be undone.");
            warning->setStyleSheet("color: #e74c3c; font-style: italic; margin-top: 10px; margin-left: 30px;");
            rLayout->addWidget(warning);
            rLayout->addStretch();
            cLayout->addWidget(removeContainer);
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

static QWidget* createPersonnelPage(QStackedWidget* &outNestedStack) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(25);
    layout->addWidget(new QLabel("Personnel Management"));

    QWidget *actionBar = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setSpacing(12);

    outNestedStack = new QStackedWidget();
    QStringList tabNames = {"Employee Directory", "Add Employee", "Payroll", "Leave Requests", "Statistics", "PDF Printing"};
    QList<QPushButton*> tabButtons;

    for (const auto &name : tabNames) {
        QPushButton *btn = new QPushButton(name);
        btn->setCheckable(true);
        btn->setStyleSheet(getTabButtonStyle());
        actionLayout->addWidget(btn);
        tabButtons.append(btn);

        QWidget *content = new QWidget();
        QVBoxLayout *cLayout = new QVBoxLayout(content);

        if (name == "Employee Directory") {
            cLayout->addWidget(createStyledTable("Staff Directory (Updated)", {"ID", "Name", "Position", "Department", "Start Date"}, {
                                                                                                                                          {"EMP-001", "John Doe", "Manager", "Sales", "2020-01-15"},
                                                                                                                                          {"EMP-002", "Jane Smith", "Engineer", "Product", "2021-03-22"},
                                                                                                                                          {"EMP-003", "Robert Brown", "Technician", "Maintenance", "2019-11-05"},
                                                                                                                                          {"EMP-004", "Emily White", "Accountant", "Finance", "2022-06-01"}
                                                                                                                                      }, true, true));
        } else if (name == "Add Employee") {
            // Custom styled form with internal scroll area
            QWidget *formContainer = new QWidget();
            formContainer->setStyleSheet(".QWidget { background-color: #ffffff; border-radius: 10px; border: 1px solid #eee; }");

            QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
            outerLayout->setContentsMargins(30, 30, 30, 30);
            outerLayout->setSpacing(20);

            // Fixed Title
            QLabel *titleLabel = new QLabel("New Employee Registration");
            titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; border: none; margin-bottom: 5px;");
            outerLayout->addWidget(titleLabel);

            // Scroll Area (The Viewport)
            QScrollArea *scrollArea = new QScrollArea();
            scrollArea->setWidgetResizable(true);
            scrollArea->setFrameShape(QFrame::NoFrame);
            scrollArea->setStyleSheet(
                "QScrollArea { border: none; background: transparent; }"
                "QScrollBar:vertical {"
                "    border: none;"
                "    background: #f0f0f0;"
                "    width: 10px;"
                "    border-radius: 5px;"
                "    margin: 0px 0px 0px 0px;"
                "}"
                "QScrollBar::handle:vertical {"
                "    background: #ccc;"
                "    min-height: 20px;"
                "    border-radius: 5px;"
                "}"
                "QScrollBar::handle:vertical:hover {"
                "    background: #bbb;"
                "}"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
                "    height: 0px;"
                "}"
                "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
                "    background: none;"
                "}"
                );

            // Internal Form Content
            QWidget *formContent = new QWidget();
            formContent->setStyleSheet("background: transparent;");
            QVBoxLayout *layout = new QVBoxLayout(formContent);
            layout->setContentsMargins(0, 0, 20, 0); // Increased right margin
            layout->setSpacing(15);

            QString labelStyle = getLabelStyle();
            QString inputStyle = getInputStyle();

            QList<QPair<QString, QString>> fields = {
                {"Full Name:", "Enter full name"},
                {"Position:", "Job Title"},
                {"Department:", "Select Department"},
                {"Email:", "company@email.com"},
                {"Phone Number:", "+1 (555) ..."},
                {"Address:", "Street, City, Zip"},
                {"Start Date:", "YYYY-MM-DD"},
                {"Emergency Contact:", "Name and Phone"},
                {"Notes:", "Additional comments"}
            };

            for (const auto &field : fields) {
                QLabel *lbl = new QLabel(field.first);
                lbl->setStyleSheet(labelStyle);
                QLineEdit *input = new QLineEdit();
                input->setStyleSheet(inputStyle);
                input->setPlaceholderText(field.second);
                layout->addWidget(lbl);
                layout->addWidget(input);
            }

            layout->addSpacing(20);
            QPushButton *btnSubmit = new QPushButton("Hire Employee");
            btnSubmit->setStyleSheet(getButtonStyle());
            btnSubmit->setCursor(Qt::PointingHandCursor);
            btnSubmit->setFixedHeight(45);
            layout->addWidget(btnSubmit);
            layout->addStretch();

            scrollArea->setWidget(formContent);
            outerLayout->addWidget(scrollArea);

            cLayout->addWidget(formContainer);
        } else if (name == "Payroll") {
            cLayout->addWidget(createStyledTable("Recent Payroll Runs", {"Period", "Total Payout", "Status", "Date Processed"}, {
                                                                                                                                    {"October 2023", "$145,000", "Completed", "2023-10-31"},
                                                                                                                                    {"September 2023", "$142,500", "Completed", "2023-09-30"}
                                                                                                                                }, true));
        } else if (name == "Statistics") {
            QWidget *statsContainer = new QWidget();
            QVBoxLayout *statsLayout = new QVBoxLayout(statsContainer);
            statsLayout->setSpacing(30);

            QMap<QString, int> headcount;
            headcount["Sales"] = 12;
            headcount["Product"] = 8;
            headcount["Maintenance"] = 5;
            headcount["Finance"] = 4;
            statsLayout->addWidget(new MockBarChart("Department Headcount", headcount));

            QMap<QString, int> absence;
            absence["John Doe"] = 2;
            absence["Jane Smith"] = 5;
            absence["Robert Brown"] = 0;
            absence["Emily White"] = 3;
            statsLayout->addWidget(new MockBarChart("Absence Days (YTD)", absence));

            statsLayout->addStretch();
            cLayout->addWidget(statsContainer);

        } else if (name == "PDF Printing") {
            QWidget *formContainer = new QWidget();
            formContainer->setStyleSheet(".QWidget { background-color: #ffffff; border-radius: 10px; border: 1px solid #eee; }");
            QVBoxLayout *vbox = new QVBoxLayout(formContainer);
            vbox->setContentsMargins(40, 40, 40, 40);
            vbox->setSpacing(15);

            QLabel *title = new QLabel("Vacation Demand Form");
            title->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; margin-bottom: 20px; border: none;");
            vbox->addWidget(title);

            QString labelStyle = getLabelStyle();
            QString inputStyle = getInputStyle();

            auto createField = [&](const QString &label, const QString &placeholder) -> QLineEdit* {
                QLabel *lbl = new QLabel(label);
                lbl->setStyleSheet(labelStyle);
                QLineEdit *edt = new QLineEdit();
                edt->setStyleSheet(inputStyle);
                edt->setPlaceholderText(placeholder);
                vbox->addWidget(lbl);
                vbox->addWidget(edt);
                return edt;
            };

            /*
             QLineEdit *edtName = createField("Employee Name:", "Full Name");
             QLineEdit *edtStart = createField("Start Date:", "YYYY-MM-DD");
             QLineEdit *edtEnd = createField("End Date:", "YYYY-MM-DD");
             QLineEdit *edtReason = createField("Reason:", "Vacation / Personal / Medical");
             */
            // Placeholder fields that don't need variables for demo
            createField("Employee Name:", "Full Name");
            createField("Start Date:", "YYYY-MM-DD");
            createField("End Date:", "YYYY-MM-DD");
            createField("Reason:", "Vacation / Personal / Medical");

            vbox->addSpacing(30);

            QPushButton *btnPrint = new QPushButton("Print Request to PDF");
            btnPrint->setStyleSheet(getButtonStyle());
            btnPrint->setCursor(Qt::PointingHandCursor);
            btnPrint->setFixedHeight(45);
            vbox->addWidget(btnPrint);
            vbox->addStretch();

            cLayout->addWidget(formContainer);

            // Connect Print Button
            QObject::connect(btnPrint, &QPushButton::clicked, [=]() {
                QMessageBox::information(nullptr, "Info", "PDF Generation UI Demo Only");
            });
        } else {
            cLayout->addWidget(createStyledTable("Pending Leave Requests", {"Employee", "Type", "Dates", "Status"}, {
                                                                                                                        {"Jane Smith", "Vacation", "Nov 5 - Nov 12", "Pending Approval"},
                                                                                                                        {"Robert Brown", "Sick Leave", "Oct 26", "Approved"}
                                                                                                                    }, true));
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

// --- Authentication Widgets ---
static QWidget* createMainAppContent(QWidget* parent);

static QWidget* createLoginWidget(QStackedWidget* authStack, QStackedWidget* rootStack) {
    QWidget *loginWidget = new QWidget();
    loginWidget->setStyleSheet("background-color: #f0f2f5;"); // Neutral background

    QVBoxLayout *mainLayout = new QVBoxLayout(loginWidget);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Card Container
    QWidget *card = new QWidget();
    card->setFixedWidth(420);
    card->setStyleSheet(
        "background-color: #ffffff;"
        "border-radius: 16px;"
        "border: 1px solid #e0e0e0;"
        );

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(40, 50, 40, 50);
    layout->setSpacing(20);

    // Logo
    QLabel *logoLabel = new QLabel();
    logoLabel->setAlignment(Qt::AlignCenter);
    QPixmap logoPixmap(":/logo.png");
    if (!logoPixmap.isNull()) {
        logoLabel->setPixmap(logoPixmap.scaledToHeight(60, Qt::SmoothTransformation));
    } else {
        logoLabel->setText("NEXUS ERP");
        logoLabel->setStyleSheet("font-size: 28px; font-weight: 900; color: #1a1a1a;");
    }
    layout->addWidget(logoLabel);

    QLabel *subtitle = new QLabel("Sign in to your account");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 16px; color: #666; margin-bottom: 20px;");
    layout->addWidget(subtitle);

    // Inputs
    QLineEdit *usernameInput = new QLineEdit();
    usernameInput->setPlaceholderText("Username");
    usernameInput->setStyleSheet(getInputStyle());
    layout->addWidget(usernameInput);

    QLineEdit *passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setStyleSheet(getInputStyle());
    layout->addWidget(passwordInput);

    // Login Button
    QPushButton *loginButton = new QPushButton("Sign In");
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setStyleSheet(getButtonStyle());
    QObject::connect(loginButton, &QPushButton::clicked, [=]() {
        // Bypass login logic - Pure UI
        rootStack->addWidget(createMainAppContent(loginWidget));
        rootStack->setCurrentIndex(1); // Switch to main app
    });
    layout->addWidget(loginButton);

    // Register Link
    QPushButton *registerLink = new QPushButton("Don't have an account? Create one");
    registerLink->setCursor(Qt::PointingHandCursor);
    registerLink->setStyleSheet("QPushButton { border: none; background: transparent; color: #555; font-size: 14px; margin-top: 10px; } QPushButton:hover { color: #3DDC84; text-decoration: underline; }");
    QObject::connect(registerLink, &QPushButton::clicked, [=]() {
        authStack->setCurrentIndex(1); // Switch to register page
    });
    layout->addWidget(registerLink);

    mainLayout->addWidget(card);
    return loginWidget;
}

static QWidget* createRegisterWidget(QStackedWidget* authStack) {
    QWidget *registerWidget = new QWidget();
    registerWidget->setStyleSheet("background-color: #f0f2f5;");

    QVBoxLayout *mainLayout = new QVBoxLayout(registerWidget);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Card Container
    QWidget *card = new QWidget();
    card->setFixedWidth(500); // Slightly wider for more fields
    card->setStyleSheet(
        "background-color: #ffffff;"
        "border-radius: 16px;"
        "border: 1px solid #e0e0e0;"
        );

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(15);

    // Title
    QLabel *title = new QLabel("Create Account");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 24px; font-weight: 800; color: #1a1a1a; margin-bottom: 10px;");
    layout->addWidget(title);

    // Fields
    QLineEdit *fullNameInput = new QLineEdit(); fullNameInput->setPlaceholderText("Full Name"); fullNameInput->setStyleSheet(getInputStyle());
    layout->addWidget(fullNameInput);

    QLineEdit *emailInput = new QLineEdit(); emailInput->setPlaceholderText("Email Address"); emailInput->setStyleSheet(getInputStyle());
    layout->addWidget(emailInput);

    QLineEdit *usernameInput = new QLineEdit(); usernameInput->setPlaceholderText("Username"); usernameInput->setStyleSheet(getInputStyle());
    layout->addWidget(usernameInput);

    QLineEdit *passwordInput = new QLineEdit(); passwordInput->setPlaceholderText("Password"); passwordInput->setEchoMode(QLineEdit::Password); passwordInput->setStyleSheet(getInputStyle());
    layout->addWidget(passwordInput);

    // Row for Role/Dept
    QHBoxLayout *rowLayout = new QHBoxLayout();
    QLineEdit *role = new QLineEdit(); role->setPlaceholderText("Job Role"); role->setStyleSheet(getInputStyle());
    QLineEdit *dept = new QLineEdit(); dept->setPlaceholderText("Department"); dept->setStyleSheet(getInputStyle());
    rowLayout->addWidget(role);
    rowLayout->addWidget(dept);
    layout->addLayout(rowLayout);

    // Register Button
    QPushButton *registerButton = new QPushButton("Create Account");
    registerButton->setCursor(Qt::PointingHandCursor);
    registerButton->setStyleSheet(getButtonStyle());
    QObject::connect(registerButton, &QPushButton::clicked, [=]() {
        // Bypass registration logic - Pure UI
        authStack->setCurrentIndex(0); // Go back to login page
    });
    layout->addWidget(registerButton);

    // Login Link
    QPushButton *loginLink = new QPushButton("Already have an account? Sign In");
    loginLink->setCursor(Qt::PointingHandCursor);
    loginLink->setStyleSheet("QPushButton { border: none; background: transparent; color: #555; font-size: 14px; margin-top: 5px; } QPushButton:hover { color: #3DDC84; text-decoration: underline; }");
    QObject::connect(loginLink, &QPushButton::clicked, [=]() {
        authStack->setCurrentIndex(0); // Switch to login page
    });
    layout->addWidget(loginLink);

    mainLayout->addWidget(card);
    return registerWidget;
}

// --- Main Application Helper ---
static QWidget* createMainAppContent(QWidget* parent) {
    QWidget *centralWidget = new QWidget();
    centralWidget->setStyleSheet("background-color: #f4f6f8;");

    QVBoxLayout *verticalRoot = new QVBoxLayout(centralWidget);
    verticalRoot->setContentsMargins(0, 0, 0, 0);
    verticalRoot->setSpacing(0);

    // Sidebar
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(280);
    sidebar->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #111111, stop:1 #1a1a1a);"
        "border-right: 1px solid #333;"
        );

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(20, 30, 20, 40);
    sidebarLayout->setSpacing(10);

    // LOGO
    QLabel *brand = new QLabel();
    QPixmap logoPixmap(":/logo.png");
    if(!logoPixmap.isNull()) {
        brand->setPixmap(logoPixmap.scaledToHeight(45, Qt::SmoothTransformation));
        brand->setAlignment(Qt::AlignCenter);
    } else {
        brand->setText("NEXUS ERP");
        brand->setStyleSheet("color: white; font-size: 24px; font-weight: 900; border: none; background: transparent;");
    }
    sidebarLayout->addWidget(brand);
    sidebarLayout->addSpacing(20);

    QString sidebarButtonStyle =
        "QPushButton {"
        "color: #999;"
        "background-color: transparent;"
        "border: none;"
        "border-radius: 8px;"
        "padding: 14px 20px;"
        "text-align: left;"
        "font-size: 14px;"
        "font-weight: 500;"
        "} "
        "QPushButton:hover {"
        "background-color: rgba(255, 255, 255, 0.05);"
        "color: #fff;"
        "} "
        "QPushButton:checked {"
        "background-color: #3DDC84;"
        "color: #ffffff;"
        "font-weight: 700;"
        "}";

    struct NavItem { QString title; QPushButton *btn; };
    QList<NavItem> navItems;

    auto addNav = [&](const QString &title) {
        QPushButton *btn = new QPushButton(title);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(sidebarButtonStyle);
        sidebarLayout->addWidget(btn);
        navItems.append({title, btn});
        return btn;
    };

    QPushButton *btnHome = addNav("Overview");
    sidebarLayout->addSpacing(10);
    QLabel *menuLabel = new QLabel("MODULES");
    menuLabel->setStyleSheet("color: #444; font-weight: bold; font-size: 10px; margin-top: 10px; margin-bottom: 5px; padding-left: 10px; border: none; background: transparent;");
    sidebarLayout->addWidget(menuLabel);

    QPushButton *btnPersonnel = addNav("Personnel Management");
    QPushButton *btnProduct = addNav("Product");
    QPushButton *btnInventory = addNav("Inventory Management");
    QPushButton *btnFinance = addNav("Financial Management");
    QPushButton *btnMaintenance = addNav("Maintenance");
    QPushButton *btnOrder = addNav("Order Management");

    sidebarLayout->addStretch();

    // User Profile
    QWidget *userWidget = new QWidget();
    userWidget->setStyleSheet("background-color: rgba(255,255,255,0.05); border-radius: 8px;");
    QHBoxLayout *userLayout = new QHBoxLayout(userWidget);
    userLayout->setContentsMargins(10, 10, 10, 10);
    QLabel *userAvatar = new QLabel("AD");
    userAvatar->setFixedSize(36, 36);
    userAvatar->setAlignment(Qt::AlignCenter);
    userAvatar->setStyleSheet("background-color: #3DDC84; color: white; font-weight: bold; border-radius: 18px; border: none;");
    QLabel *userName = new QLabel("Samik Admin");
    userName->setStyleSheet("color: white; font-weight: 600; font-size: 13px; border: none; background: transparent;");

    QPushButton *btnLogout = new QPushButton("Logout");
    btnLogout->setCursor(Qt::PointingHandCursor);
    btnLogout->setStyleSheet("QPushButton { color: #aaaaaa; border: 1px solid #444; border-radius: 4px; padding: 4px 8px; font-size: 11px; background: transparent; } QPushButton:hover { color: #fff; border-color: #666; background: rgba(255,255,255,0.1); }");

    // Connect Logout
    // 'parent' of createMainAppContent is 'loginWidget' in current flow,
    // but we need to identify the MainWindow or Stack to switch.
    // The 'parent' passed here IS the loginWidget, which is inside AuthStack.
    // We need to access the Root Stack.
    // However, createMainAppContent return value is added to RootStack.
    // Let's use QObject::parent() traversal or signal forwarding?
    // Easiest hack: Pass rootStack pointer or find it.
    // Better: We are inside a function.
    // The 'parent' arg was passed as 'loginWidget'.
    // Let's rely on finding top level window or just casting parent to get MainWindow if possible,
    // OR simply assume specific hierarchy.
    // Actually, 'parent' in createMainAppContent(this) call (old code) was MainWindow.
    // In new flow: createMainAppContent(loginWidget).
    // Let's just use a direct connection logic assuming we can find the stack.
    // The cleanest way without changing signature is to find the QStackedWidget parent of this widget.

    QObject::connect(btnLogout, &QPushButton::clicked, [centralWidget]() {
        // We need to find the Root Stack.
        // Hierarchy: centralWidget -> Root Stack -> MainWindow
        // Wait, centralWidget IS returned and ADDED to Root Stack.
        // So centralWidget->parent() should be the QStackedWidget (Root).
        QWidget *p = centralWidget->parentWidget();
        if (QStackedWidget *stack = qobject_cast<QStackedWidget*>(p)) {
            stack->setCurrentIndex(0); // Index 0 is Auth Stack

            // Optional: Remove this widget to reset state?
            // For now, just switching is enough, but auth flow might be weird if we re-login.
            // Ideally we should remove it so a new one is created on next login.
            stack->removeWidget(centralWidget);
            centralWidget->deleteLater();
        }
    });

    userLayout->addWidget(userAvatar);
    userLayout->addWidget(userName);
    userLayout->addStretch();
    userLayout->addWidget(btnLogout);
    sidebarLayout->addWidget(userWidget);

    // --- MAIN CONTENT AREA ---
    QStackedWidget *stackedWidget = new QStackedWidget();

    // 1. Dashboard Page
    QWidget *homePage = new QWidget();
    QVBoxLayout *homeLayout = new QVBoxLayout(homePage);
    homeLayout->setContentsMargins(50, 50, 50, 50);
    homeLayout->setSpacing(30);

    // Top Bar
    QWidget *topBar = new QWidget();
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(0,0,0,0);
    QLabel *welcome = new QLabel("Welcome back, Admin");
    welcome->setStyleSheet("font-size: 36px; font-weight: 800; color: #1a1a1a; letter-spacing: -1px; border: none;");
    QLabel *dateLabel = new QLabel(QDate::currentDate().toString("dddd, MMMM d"));
    dateLabel->setStyleSheet("font-size: 16px; color: #777; font-weight: 500; border: none;");
    topLayout->addWidget(welcome);
    topLayout->addStretch();
    topLayout->addWidget(dateLabel);
    homeLayout->addWidget(topBar);

    // Stats
    QWidget *statsContainer = new QWidget();
    QHBoxLayout *statsLayout = new QHBoxLayout(statsContainer);
    statsLayout->setContentsMargins(0,0,0,0);
    statsLayout->setSpacing(20);
    statsLayout->addWidget(createStatCard("Total Revenue", "$128,400", "+12% this month", "#3DDC84"));
    statsLayout->addWidget(createStatCard("Active Clients", "1,245", "+5 new today", "#3498db"));
    statsLayout->addWidget(createStatCard("Pending Orders", "18", "Requires attention", "#e74c3c"));
    statsLayout->addWidget(createStatCard("System Status", "99.9%", "All systems operational", "#f1c40f"));
    homeLayout->addWidget(statsContainer);

    // Bottom Section
    QWidget *bottomSection = new QWidget();
    QHBoxLayout *splitLayout = new QHBoxLayout(bottomSection);
    splitLayout->setContentsMargins(0,0,0,0);
    splitLayout->setSpacing(30);

    // Recent Activity Placeholder
    QWidget *activityWidget = new QWidget();
    activityWidget->setStyleSheet(getCardStyle());
    QVBoxLayout *actLayout = new QVBoxLayout(activityWidget);
    actLayout->setContentsMargins(25,25,25,25);
    QLabel *actTitle = new QLabel("Recent Activity");
    actTitle->setStyleSheet("font-size: 18px; font-weight: 700; color: #222; margin-bottom: 15px; border: none;");
    actLayout->addWidget(actTitle);
    actLayout->addWidget(new QLabel("• User 'Admin' logged in"));
    actLayout->addWidget(new QLabel("• New Order #1023 received"));
    actLayout->addStretch();

    splitLayout->addWidget(activityWidget, 2);
    homeLayout->addWidget(bottomSection, 1);

    // 2. Sub Pages
    QStackedWidget *stackPersonnel = nullptr;
    QStackedWidget *stackProduct = nullptr;
    QStackedWidget *stackInventory = nullptr;
    QStackedWidget *stackFinance = nullptr;
    QStackedWidget *stackMaintenance = nullptr;
    QStackedWidget *stackOrder = nullptr;

    // Index mapping:
    // 0: Overview
    // 1: Personnel
    // 2: Product
    // 3: Inventory
    // 4: Finance
    // 5: Maintenance
    // 6: Order

    stackedWidget->addWidget(homePage);                        // Index 0
    stackedWidget->addWidget(createPersonnelPage(stackPersonnel)); // Index 1
    stackedWidget->addWidget(createProductPage(stackProduct));     // Index 2
    stackedWidget->addWidget(createInventoryPage(stackInventory)); // Index 3
    stackedWidget->addWidget(createFinancePage(stackFinance));     // Index 4
    stackedWidget->addWidget(createMaintenancePage(stackMaintenance)); // Index 5
    stackedWidget->addWidget(createOrderPage(stackOrder));         // Index 6

    // Layout Assembly
    QWidget *contentArea = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(contentArea);
    contentLayout->setContentsMargins(0,0,0,0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(sidebar);
    contentLayout->addWidget(stackedWidget);
    verticalRoot->addWidget(contentArea);

    // Navigation Logic
    auto setActive = [=](int index, QPushButton* activeBtn) {
        stackedWidget->setCurrentIndex(index);
        for(auto& item : navItems) item.btn->setChecked(item.btn == activeBtn);
    };

    // Connect Buttons
    // Index mapping: 0=Home, 1=Personnel
    QObject::connect(btnHome, &QPushButton::clicked, parent, [=](){ setActive(0, btnHome); });
    QObject::connect(btnPersonnel, &QPushButton::clicked, parent, [=](){ setActive(1, btnPersonnel); });
    QObject::connect(btnProduct, &QPushButton::clicked, parent, [=](){ setActive(2, btnProduct); });
    QObject::connect(btnInventory, &QPushButton::clicked, parent, [=](){ setActive(3, btnInventory); });
    QObject::connect(btnFinance, &QPushButton::clicked, parent, [=](){ setActive(4, btnFinance); });
    QObject::connect(btnMaintenance, &QPushButton::clicked, parent, [=](){ setActive(5, btnMaintenance); });
    QObject::connect(btnOrder, &QPushButton::clicked, parent, [=](){ setActive(6, btnOrder); });

    // Default to Overview
    btnHome->click();
    return centralWidget;
}

// --- MainWindow Implementation ---

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), stackedWidget(new QStackedWidget(this))
{
    setCentralWidget(stackedWidget);
    resize(1280, 800); // Standard desktop size
    setWindowTitle("NEXUS ERP - Enterprise System");

    // Root Stack has 2 layers:
    // Index 0: Auth Flow (Login <-> Register)
    // Index 1: Main App (Overview, Modules, etc.) - Created on Login

    // --- Layer 0: Authentication Stack ---
    QStackedWidget *authStack = new QStackedWidget();

    // Create Pages
    QWidget *loginPage = createLoginWidget(authStack, stackedWidget);
    QWidget* registerPage = createRegisterWidget(authStack);

    authStack->addWidget(loginPage);    // Auth Index 0
    authStack->addWidget(registerPage); // Auth Index 1

    stackedWidget->addWidget(authStack); // Root Index 0

    // Main App (Root Index 1) is added dynamically in createLoginWidget upon success.
}

