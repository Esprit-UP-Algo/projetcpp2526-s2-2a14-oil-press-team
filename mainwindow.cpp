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
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QScrollArea>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QTextDocument>
#include <QPageSize>

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

static QWidget* createStyledForm(const QString &title, const QList<QPair<QString, QString>> &fields, const QString &submitText, int spacing = 15) {
    QWidget *formContainer = new QWidget();
    formContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    formContainer->setStyleSheet(".QWidget { background-color: #ffffff; border-radius: 10px; border: 1px solid #eee; }");
    
    QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
    outerLayout->setContentsMargins(30, 30, 30, 30);
    
    // --- Scroll Area Setup ---
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QWidget { background: transparent; }"
        "QScrollBar:vertical { border: none; background: #f0f0f0; width: 10px; margin: 0px 0px 0px 0px; border-radius: 5px; }"
        "QScrollBar::handle:vertical { background: #cdcdcd; min-height: 20px; border-radius: 5px; }"
        "QScrollBar::handle:vertical:hover { background: #3DDC84; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
    );

    QWidget *formContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(formContent);
    layout->setContentsMargins(0, 0, 10, 0); // Right padding for scrollbar
    layout->setSpacing(spacing); 

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
    
    scrollArea->setWidget(formContent);
    outerLayout->addWidget(scrollArea);

    return formContainer;
}

static QWidget* createStyledTable(const QString &title, const QStringList &headers, const QVector<QStringList> &data, bool showActions = false) {
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
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
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
            actionLayout->setContentsMargins(4, 4, 4, 4);
            actionLayout->setSpacing(8);
            
            
            QPushButton *btnDelete = new QPushButton("Delete");
            btnDelete->setCursor(Qt::PointingHandCursor);
            btnDelete->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border: none; border-radius: 4px; padding: 6px 10px; font-size: 11px; font-weight: bold; } QPushButton:hover { background-color: #c0392b; }");

            // actionLayout->addWidget(btnEdit); // Removed Edit button as requested
            actionLayout->addWidget(btnDelete);
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

static QWidget* createClientPage(QStackedWidget* &outNestedStack) {
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
            }, true)); // Actions enabled
        } else {
             // Container for statistics page to hold button + table
             QWidget *statsPageWidget = new QWidget();
             QVBoxLayout *statsPageLayout = new QVBoxLayout(statsPageWidget);
             statsPageLayout->setContentsMargins(0,0,0,0);
             statsPageLayout->setSpacing(10);

             // Add "PRINT PDF" button aligned to right (Inactive/Non-functional as requested)
             QPushButton *btnPrint = new QPushButton("PRINT PDF");
             btnPrint->setStyleSheet(getButtonStyle());
             // btnPrint->setCursor(Qt::PointingHandCursor); // Removed to signify inactivity if needed, but keeping style uniform
             btnPrint->setFixedWidth(150);
             // btnPrint->setEnabled(false); // Can uncomment if "inactif" meant disabled. For now, just non-functional.
             
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
             // Control Bar
             QWidget *controls = new QWidget();
             QHBoxLayout *ctrlLayout = new QHBoxLayout(controls);
             ctrlLayout->setContentsMargins(0,0,0,0);
             ctrlLayout->setSpacing(10);
             
             // Search
             
             QComboBox *searchType = new QComboBox();
             searchType->addItems({"All", "Type", "State"});
             searchType->setStyleSheet(getInputStyle());
             
             // Sort
             QLabel *sortLabel = new QLabel("Sort by:");
             sortLabel->setStyleSheet(getLabelStyle());
             QComboBox *sortCombo = new QComboBox();
             sortCombo->addItems({"Default", "State", "Hours"});
             sortCombo->setStyleSheet(getInputStyle());
             
             // PDF Button
             QPushButton *pdfBtn = new QPushButton("Export PDF");
             pdfBtn->setStyleSheet(getButtonStyle());
             pdfBtn->setFixedWidth(120);

             ctrlLayout->addWidget(searchType);
             ctrlLayout->addSpacing(20);
             ctrlLayout->addWidget(sortLabel);
             ctrlLayout->addWidget(sortCombo);
             ctrlLayout->addStretch();
             ctrlLayout->addWidget(pdfBtn);

             cLayout->addWidget(controls);

             // Table
             QStringList headers = {"ID", "Nom", "Type", "Etat", "Heures", "Date Maint", "Seuil"};
             QTableWidget *table = new QTableWidget();
             table->setColumnCount(headers.size());
             table->setHorizontalHeaderLabels(headers);
             table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
             table->verticalHeader()->setVisible(false);
             table->setAlternatingRowColors(true);
             table->setStyleSheet(
                "QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; gridline-color: transparent; border-radius: 8px; alternate-background-color: #f9fafb; }"
                "QHeaderView::section { background-color: #ffffff; padding: 12px; border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; color: #666; text-transform: uppercase; font-size: 12px; }"
                "QTableWidget::item { padding: 12px; border-bottom: 1px solid #f5f5f5; color: #333; }"
                "QTableWidget::item:selected { background-color: #e6f9ef; color: #1a1a1a; }"
             );

             // Dummy Data
             QVector<QStringList> data = {
                 {"EQ-001", "Presse A1", "Presse", "Normale", "1200", "2023-10-01", "2000"},
                 {"EQ-002", "Moteur X", "Moteur", "En Panne", "5400", "2023-09-15", "5000"},
                 {"EQ-003", "Filtre F1", "Filtre", "En Maintenance", "800", "2023-10-20", "1000"},
                 {"EQ-004", "Presse B2", "Presse", "Normale", "300", "2023-11-01", "2000"},
                 {"EQ-005", "Generateur", "Moteur", "Normale", "9500", "2023-08-10", "10000"}
             };

             table->setRowCount(data.size());
             for(int i=0; i<data.size(); ++i) {
                 for(int j=0; j<data[i].size(); ++j) {
                     QTableWidgetItem *item = new QTableWidgetItem(data[i][j]);
                     // For sorting numbers correctly (simple approach: pad with zeros or rely on Qt's default if it detects numbers, often mixed. 
                     // For 'Heures' (col 4), strictly it should be numerical. I'll just set data role)
                     if (j == 4 || j == 6) { // Heures or Seuil
                          item->setData(Qt::DisplayRole, data[i][j].toInt());
                     }
                     table->setItem(i, j, item);
                 }
             }
             
             cLayout->addWidget(table);

             // Logic (Lambdas)
             
             // Sort
             QObject::connect(sortCombo, &QComboBox::currentTextChanged, [=](const QString &text){
                 if (text == "State") table->sortItems(3, Qt::AscendingOrder);
                 else if (text == "Hours") table->sortItems(4, Qt::DescendingOrder);
                 else table->sortItems(0, Qt::AscendingOrder); // Default ID
             });

             // PDF Export
             QObject::connect(pdfBtn, &QPushButton::clicked, [=](){
                 QString fileName = QFileDialog::getSaveFileName(nullptr, "Export PDF", "", "PDF Files (*.pdf)");
                 if (fileName.isEmpty()) return;
                 if (QFileInfo(fileName).suffix().isEmpty()) fileName.append(".pdf");

                 QPrinter printer(QPrinter::PrinterResolution);
                 printer.setOutputFormat(QPrinter::PdfFormat);
                 printer.setPageSize(QPageSize(QPageSize::A4));
                 printer.setOutputFileName(fileName);

                 QTextDocument doc;
                 QString html = "<html><head><style>"
                                "table { border-collapse: collapse; width: 100%; }"
                                "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
                                "th { background-color: #f2f2f2; }"
                                "</style></head><body>"
                                "<h2>Equipment Status Report</h2>"
                                "<table><thead><tr>";
                 
                 for(int j=0; j<table->columnCount(); ++j)
                     html += "<th>" + headers[j] + "</th>";
                 
                 html += "</tr></thead><tbody>";

                 for(int i=0; i<table->rowCount(); ++i) {
                     if (table->isRowHidden(i)) continue; // Skip hidden rows (respect search)
                     html += "<tr>";
                     for(int j=0; j<table->columnCount(); ++j)
                         html += "<td>" + table->item(i, j)->text() + "</td>";
                     html += "</tr>";
                 }
                 html += "</tbody></table></body></html>";

                 doc.setHtml(html);
                 doc.setPageSize(printer.pageRect(QPrinter::DevicePixel).size()); 
                 doc.print(&printer);
                 
                 QMessageBox::information(nullptr, "Success", "PDF exported successfully!");
             });

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
             // Container for list page to hold button + table
             QWidget *listPageWidget = new QWidget();
             QVBoxLayout *listPageLayout = new QVBoxLayout(listPageWidget);
             listPageLayout->setContentsMargins(0,0,0,0);
             listPageLayout->setSpacing(10);

             // Add "PRINT PDF" button aligned to right
             QPushButton *btnPrint = new QPushButton("PRINT PDF");
             btnPrint->setStyleSheet(getButtonStyle());
             btnPrint->setCursor(Qt::PointingHandCursor);
             btnPrint->setFixedWidth(150);
             listPageLayout->addWidget(btnPrint, 0, Qt::AlignRight);

             // Create table
             QWidget *tableWidget = createStyledTable("Current Product Catalog", 
                {"ID Container", "Date Pressage", "Capacité", "Ref Testeur", "Qualité", "Viscosity", "Color", "Ref Press"}, {
                 {"CONT-001", "2023-10-25", "500L", "TEST-A1", "Premium", "0.85", "Golden", "PRESS-X1"},
                 {"CONT-002", "2023-10-26", "200L", "TEST-B2", "Standard", "0.90", "Yellow", "PRESS-X2"},
                 {"CONT-003", "2023-10-27", "1000L", "TEST-A1", "Premium", "0.84", "Golden", "PRESS-X1"}
             }, true);
             
             listPageLayout->addWidget(tableWidget);

             // Connect print button
             // PDF Export functionality removed as per request
             // The button remains visible but inactive

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
             }, "Add Product", 25));
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
             }, "Update Product", 25));
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
    QStringList tabNames = {"Employee Directory", "Add Employee", "Payroll", "Leave Requests"};
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
             cLayout->addWidget(createStyledTable("Staff Directory", {"ID", "Name", "Position", "Department", "Start Date"}, {
                 {"EMP-001", "John Doe", "Manager", "Sales", "2020-01-15"},
                 {"EMP-002", "Jane Smith", "Engineer", "Product", "2021-03-22"},
                 {"EMP-003", "Robert Brown", "Technician", "Maintenance", "2019-11-05"},
                 {"EMP-004", "Emily White", "Accountant", "Finance", "2022-06-01"}
             }, true));
        } else if (name == "Add Employee") {
             cLayout->addWidget(createStyledForm("New Employee Registration", {
                 {"Full Name:", "Enter full name"},
                 {"Position:", "Job Title"},
                 {"Department:", "Select Department"},
                 {"Email:", "company@email.com"},
                 {"Start Date:", "YYYY-MM-DD"}
             }, "Hire Employee"));
        } else if (name == "Payroll") {
             cLayout->addWidget(createStyledTable("Recent Payroll Runs", {"Period", "Total Payout", "Status", "Date Processed"}, {
                 {"October 2023", "$145,000", "Completed", "2023-10-31"},
                 {"September 2023", "$142,500", "Completed", "2023-09-30"}
             }, true));
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(1200, 800);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
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

    // LOGO INTEGRATION
    QLabel *brand = new QLabel();
    QPixmap logoPixmap(":/logo.png");
    if(!logoPixmap.isNull()) {
        brand->setPixmap(logoPixmap.scaledToHeight(54, Qt::SmoothTransformation));
        brand->setAlignment(Qt::AlignCenter);
    } else {
        brand->setText("NEXUS ERP"); 
        brand->setStyleSheet("color: white; font-size: 24px; font-weight: 900;");
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
        "box-shadow: 0 4px 6px rgba(61, 220, 132, 0.3);"
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
    menuLabel->setStyleSheet("color: #444; font-weight: bold; font-size: 10px; margin-top: 10px; margin-bottom: 5px; padding-left: 10px;");
    sidebarLayout->addWidget(menuLabel);

    QPushButton *btnClient = addNav("Order Management");
    QPushButton *btnFinance = addNav("Financial Management");
    QPushButton *btnInventory = addNav("Inventory Management");
    QPushButton *btnMaintenance = addNav("Maintenance");
    QPushButton *btnProduct = addNav("Product");
    QPushButton *btnPersonnel = addNav("Personnel Management"); // NEW BUTTON

    sidebarLayout->addStretch();
    sidebarLayout->addWidget(new QLabel("v1.3.0")); 

    stackedWidget = new QStackedWidget();

    // --- HOME DASHBOARD ---
    QWidget *homePage = new QWidget();
    QVBoxLayout *homeLayout = new QVBoxLayout(homePage);
    homeLayout->setContentsMargins(50, 50, 50, 50);
    homeLayout->setSpacing(30);

    QWidget *topBar = new QWidget();
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(0,0,0,0);
    QLabel *welcome = new QLabel("Welcome back, Admin");
    welcome->setStyleSheet("font-size: 36px; font-weight: 800; color: #1a1a1a; letter-spacing: -1px;");
    QLabel *dateLabel = new QLabel(QDate::currentDate().toString("dddd, MMMM d"));
    dateLabel->setStyleSheet("font-size: 16px; color: #777; font-weight: 500;");
    topLayout->addWidget(welcome);
    topLayout->addStretch();
    topLayout->addWidget(dateLabel);
    homeLayout->addWidget(topBar);

    QWidget *statsContainer = new QWidget();
    QHBoxLayout *statsLayout = new QHBoxLayout(statsContainer);
    statsLayout->setContentsMargins(0,0,0,0);
    statsLayout->setSpacing(20);
    statsLayout->addWidget(createStatCard("Total Revenue", "$128,400", "+12% this month", "#3DDC84"));
    statsLayout->addWidget(createStatCard("Active Clients", "1,245", "+5 new today", "#3498db"));
    statsLayout->addWidget(createStatCard("Pending Orders", "18", "Requires attention", "#e74c3c"));
    statsLayout->addWidget(createStatCard("System Status", "99.9%", "All systems operational", "#f1c40f"));
    homeLayout->addWidget(statsContainer);

    QWidget *bottomSection = new QWidget();
    QHBoxLayout *splitLayout = new QHBoxLayout(bottomSection);
    splitLayout->setContentsMargins(0,0,0,0);
    splitLayout->setSpacing(30);

    QWidget *quickActions = new QWidget();
    quickActions->setStyleSheet(getCardStyle());
    QVBoxLayout *qaLayout = new QVBoxLayout(quickActions);
    qaLayout->setContentsMargins(25,25,25,25);
    QLabel *qaTitle = new QLabel("Quick Actions");
    qaTitle->setStyleSheet("font-size: 18px; font-weight: 700; color: #222; margin-bottom: 15px;");
    qaLayout->addWidget(qaTitle);

    QString actionBtnStyle = "QPushButton { text-align: left; padding: 15px; border: 1px solid #eee; border-radius: 8px; background-color: #fafafa; font-weight: 600; color: #444; } QPushButton:hover { background-color: #f0f0f0; border-color: #ddd; }";
    
    QPushButton *btnQInvoice = new QPushButton("Draft New Invoice"); btnQInvoice->setStyleSheet(actionBtnStyle);
    QPushButton *btnQClient = new QPushButton("Register New Client"); btnQClient->setStyleSheet(actionBtnStyle);
    QPushButton *btnQStock = new QPushButton("Add Inventory Stock"); btnQStock->setStyleSheet(actionBtnStyle);
    
    qaLayout->addWidget(btnQInvoice);
    qaLayout->addSpacing(5);
    qaLayout->addWidget(btnQClient);
    qaLayout->addSpacing(5);
    qaLayout->addWidget(btnQStock);
    qaLayout->addStretch();

    QWidget *activityWidget = new QWidget();
    activityWidget->setStyleSheet(getCardStyle());
    QVBoxLayout *actLayout = new QVBoxLayout(activityWidget);
    actLayout->setContentsMargins(25,25,25,25);
    actLayout->addWidget(new QLabel("Recent Activity")); // Placeholder for brevity
    actLayout->addStretch();
    splitLayout->addWidget(quickActions, 1);
    splitLayout->addWidget(activityWidget, 2);
    homeLayout->addWidget(bottomSection, 1);
    
    stackedWidget->addWidget(homePage);

    // --- SUB PAGES & Internal Logic ---
    QStackedWidget *stackClient, *stackFinance, *stackInventory, *stackMaintenance, *stackProduct, *stackPersonnel;
    
    stackedWidget->addWidget(createClientPage(stackClient));
    stackedWidget->addWidget(createFinancePage(stackFinance));
    stackedWidget->addWidget(createInventoryPage(stackInventory));
    stackedWidget->addWidget(createMaintenancePage(stackMaintenance));
    stackedWidget->addWidget(createProductPage(stackProduct));
    stackedWidget->addWidget(createPersonnelPage(stackPersonnel)); // Add to stack

    // Finish Root Layout
    QWidget *contentArea = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(contentArea);
    contentLayout->setContentsMargins(0,0,0,0);
    contentLayout->addWidget(sidebar);
    contentLayout->addWidget(stackedWidget);
    verticalRoot->addWidget(contentArea);

    // Sidebar Navigation
    // CAPTURE BY VALUE FIX: [=] instead of [&]
    auto setActive = [=](int index, QPushButton* activeBtn) {
        stackedWidget->setCurrentIndex(index);
        for(auto& item : navItems) item.btn->setChecked(item.btn == activeBtn);
    };

    connect(btnHome, &QPushButton::clicked, this, [=](){ setActive(0, btnHome); });
    connect(btnClient, &QPushButton::clicked, this, [=](){ setActive(1, btnClient); });
    connect(btnFinance, &QPushButton::clicked, this, [=](){ setActive(2, btnFinance); });
    connect(btnInventory, &QPushButton::clicked, this, [=](){ setActive(3, btnInventory); });
    connect(btnMaintenance, &QPushButton::clicked, this, [=](){ setActive(4, btnMaintenance); });
    connect(btnProduct, &QPushButton::clicked, this, [=](){ setActive(5, btnProduct); });
    connect(btnPersonnel, &QPushButton::clicked, this, [=](){ setActive(6, btnPersonnel); }); // Link button

    // Quick Action Signal Connections
    connect(btnQClient, &QPushButton::clicked, this, [=](){ 
        setActive(1, btnClient); // Switch to Client Page
        stackClient->setCurrentIndex(0); // Add New Client tab
    });
    
    connect(btnQInvoice, &QPushButton::clicked, this, [=](){ 
        setActive(2, btnFinance); // Switch to Finance Page
        stackFinance->setCurrentIndex(0); // Create Invoice tab
    });

    connect(btnQStock, &QPushButton::clicked, this, [=](){ 
        setActive(3, btnInventory); // Switch to Inventory Page
        stackInventory->setCurrentIndex(0); // Add Stock Item tab
    });

    // Start at Home
    btnHome->click();
}
