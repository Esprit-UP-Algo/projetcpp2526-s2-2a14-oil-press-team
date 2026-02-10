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
#include <QDate>
#include <QDateTime>
#include <QScrollArea>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPainter>

// --- Custom Bar Chart Widget (No Dependencies) ---
// --- Custom Bar Chart Widget (No Dependencies) ---
class GenericBarChart : public QWidget {

public:
    struct BarData {
        QString label;
        double value;
        QColor color;
    };

    GenericBarChart(const QString &title, QWidget *parent = nullptr) : QWidget(parent), m_title(title) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMinimumHeight(300);
    }

    void addBar(const QString &label, double value, const QColor &color) {
        m_bars.append({label, value, color});
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Background
        painter.fillRect(rect(), Qt::white); // White background for clarity

        if (m_bars.isEmpty()) return;

        // --- Layout Constants ---
        int leftMargin = 80;  // Space for Y-axis labels
        int bottomMargin = 50; // Space for X-axis labels
        int topMargin = 60;   // Space for Title
        int rightMargin = 20;

        int chartWidth = width() - leftMargin - rightMargin;
        int chartHeight = height() - topMargin - bottomMargin;

        // --- Calculate Range ---
        double maxVal = 0;
        for(const auto &bar : m_bars) maxVal = qMax(maxVal, bar.value);
        if (maxVal == 0) maxVal = 1;
        // Round up to nice number for Y-axis
        double niceMax = maxVal * 1.1; 

        // --- Draw Y-Axis (Scales & Grid) ---
        int gridLines = 5;
        painter.setFont(QFont("Segoe UI", 9));
        QPen gridPen(QColor("#e0e0e0")); // Light grey for grid
        gridPen.setStyle(Qt::DashLine);
        
        for (int i = 0; i <= gridLines; ++i) {
            double value = (niceMax / gridLines) * i;
            int y = topMargin + chartHeight - (int)((value / niceMax) * chartHeight);

            // Grid Line
            painter.setPen(gridPen);
            painter.drawLine(leftMargin, y, width() - rightMargin, y);

            // Scale Label
            painter.setPen(QColor("#666666"));
            // Format number (k for thousands, M for millions if needed, or just standard)
            QString label = QString::number(value, 'f', 0);
            if (value >= 1000000) label = QString::number(value / 1000000.0, 'f', 1) + "M";
            else if (value >= 1000) label = QString::number(value / 1000.0, 'f', 0) + "k";
            
            painter.drawText(QRect(0, y - 10, leftMargin - 10, 20), Qt::AlignRight | Qt::AlignVCenter, label);
        }

        // --- Draw X-Axis Line ---
        painter.setPen(QPen(QColor("#333333"), 2));
        painter.drawLine(leftMargin, topMargin + chartHeight, width() - rightMargin, topMargin + chartHeight);

        // --- Draw Bars ---
        int count = m_bars.size();
        int availableSpace = chartWidth / count;
        int barWidth = qMin(availableSpace - 20, 80); // Max width 80px
        int spacing = (chartWidth - (count * barWidth)) / (count + 1);
        if (count == 1) spacing = (chartWidth - barWidth) / 2;

        for (int i = 0; i < count; ++i) {
            const auto &bar = m_bars[i];
            int barH = (int)((bar.value / niceMax) * chartHeight);
            
            // Distribute evenly
            int x = leftMargin + (i * availableSpace) + (availableSpace - barWidth) / 2; 
            int y = topMargin + chartHeight - barH;

            QRect barRect(x, y, barWidth, barH);

            // Draw Bar (Flat Color, No Gradient)
            painter.setBrush(bar.color);
            painter.setPen(Qt::NoPen);
            painter.drawRect(barRect); // Sharp corners for technical look, or rounded if preferred

            // Draw Value on Top
            painter.setPen(QColor("#000000"));
            painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
            
            // Fix: Use 'f' format to avoid scientific notation (e.g., 1.25e+06)
            // Precision 0 means no decimals (1250500)
            QString valText = QString::number(bar.value, 'f', 0);
            
            // Format large numbers with commas for readability (manual poor-man's locale)
            if (bar.value >= 1000) {
                 // Insert commas every 3 digits from right
                 int pos = valText.length() - 3;
                 while (pos > 0) {
                     valText.insert(pos, ",");
                     pos -= 3;
                 }
            }

            painter.drawText(QRect(x - 10, y - 25, barWidth + 20, 20), Qt::AlignCenter, valText);

            // Draw X-Axis Label
            painter.setPen(QColor("#333333"));
            painter.setFont(QFont("Segoe UI", 10)); // Regular font for labels
            QRect labelRect(x - 20, topMargin + chartHeight + 10, barWidth + 40, 40); // Allow wrapping
            painter.drawText(labelRect, Qt::AlignCenter | Qt::TextWordWrap, bar.label);
        }
        
        // --- Draw Title ---
        painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
        painter.setPen(QColor("#1a1a1a"));
        painter.drawText(QRect(0, 10, width(), 30), Qt::AlignCenter, m_title);
    }

private:
    QString m_title;
    QList<BarData> m_bars;
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

static QWidget* createStyledForm(const QString &title, const QList<QPair<QString, QString>> &fields, const QString &submitText, int spacing = 15, bool useScroll = true) {
    // 1. Create content widget
    QWidget *formContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(formContent);
    int rightMargin = useScroll ? 10 : 0;
    layout->setContentsMargins(0, 0, rightMargin, 0); 
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

    // 2. Create container
    QWidget *formContainer = new QWidget();
    formContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    formContainer->setStyleSheet(".QWidget { background-color: #ffffff; border-radius: 10px; border: 1px solid #eee; }");
    
    QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
    outerLayout->setContentsMargins(30, 30, 30, 30);

    if (useScroll) {
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
        scrollArea->setWidget(formContent);
        outerLayout->addWidget(scrollArea);
    } else {
        formContent->setAttribute(Qt::WA_TranslucentBackground);
        formContent->setStyleSheet("background: transparent;");
        outerLayout->addWidget(formContent);
    }

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
    if (showActions) {
        table->horizontalHeader()->setSectionResizeMode(finalHeaders.size() - 1, QHeaderView::ResizeToContents);
    }
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(60); // Increased height
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
            // Increased right margin significantly to prevent clipping
            actionLayout->setContentsMargins(2, 2, 30, 2); 
            actionLayout->setSpacing(20); // Increased spacing
            actionLayout->setAlignment(Qt::AlignCenter); // Ensure vertical centering
            
            QPushButton *btnModify = new QPushButton("Modify");
            btnModify->setCursor(Qt::PointingHandCursor);
            btnModify->setMinimumWidth(80);
            btnModify->setFixedHeight(28); // Reduced height
            btnModify->setStyleSheet(
                "QPushButton {"
                "background-color: #ffffff;"
                "color: #333333;"
                "border: 1px solid #cccccc;"
                "border-radius: 6px;"
                "padding: 0px 8px;"
                "font-size: 13px;" // Matched to tab font size
                "font-weight: 600;" // Matched to tab font weight
                "} "
                "QPushButton:hover { border-color: #aaaaaa; color: #000000; background-color: #f6f6f6; }"
                "QPushButton:pressed { background-color: #e6e6e6; }"
            );

            QPushButton *btnDelete = new QPushButton("Delete");
            btnDelete->setCursor(Qt::PointingHandCursor);
            btnDelete->setMinimumWidth(80);
            btnDelete->setFixedHeight(28); // Reduced height
            btnDelete->setStyleSheet(
                "QPushButton {"
                "background-color: #ffffff;"
                "color: #d32f2f;"
                "border: 1px solid #d32f2f;"
                "border-radius: 6px;"
                "padding: 0px 8px;"
                "font-size: 13px;" // Matched to tab font size
                "font-weight: 600;" // Matched to tab font weight
                "} "
                "QPushButton:hover { background-color: #ffebee; border-color: #b71c1c; color: #b71c1c; }"
                "QPushButton:pressed { background-color: #ffcdd2; }"
            );

            actionLayout->addWidget(btnModify);
            actionLayout->addWidget(btnDelete);
            actionLayout->addStretch();
            
            // Capture row data for the modify dialog
            QStringList rowData = data[i];
            QStringList colHeaders = headers;
            QObject::connect(btnModify, &QPushButton::clicked, [rowData, colHeaders, table, i]() {
                QDialog *dialog = new QDialog(table->window());
                dialog->setWindowTitle("Modify Item");
                dialog->setMinimumWidth(450);
                dialog->setStyleSheet(
                    "QDialog { background-color: #ffffff; }"
                    "QLabel { font-size: 14px; font-weight: 700; color: #333; }"
                    "QLineEdit { background-color: #fcfcfc; border: 1px solid #e0e0e0; border-radius: 8px; padding: 10px 14px; font-size: 14px; color: #333; min-height: 35px; }"
                    "QLineEdit:focus { border: 2px solid #3DDC84; background-color: #ffffff; }"
                );
                
                QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
                dialogLayout->setContentsMargins(30, 30, 30, 30);
                dialogLayout->setSpacing(15);
                
                QLabel *titleLabel = new QLabel("Modify Item");
                titleLabel->setStyleSheet("font-size: 22px; font-weight: 800; color: #1a1a1a; margin-bottom: 10px;");
                dialogLayout->addWidget(titleLabel);
                
                QList<QLineEdit*> inputs;
                for (int c = 0; c < rowData.size() && c < colHeaders.size(); ++c) {
                    QLabel *lbl = new QLabel(colHeaders[c] + ":");
                    QLineEdit *input = new QLineEdit(rowData[c]);
                    dialogLayout->addWidget(lbl);
                    dialogLayout->addWidget(input);
                    inputs.append(input);
                }
                
                dialogLayout->addSpacing(20);
                
                QHBoxLayout *btnLayout = new QHBoxLayout();
                btnLayout->setSpacing(15);
                
                QPushButton *cancelBtn = new QPushButton("Cancel");
                cancelBtn->setCursor(Qt::PointingHandCursor);
                cancelBtn->setFixedHeight(45);
                cancelBtn->setStyleSheet(
                    "QPushButton { background-color: #ffffff; color: #555; border: 1px solid #ddd; border-radius: 8px; padding: 12px 24px; font-size: 14px; font-weight: 600; } "
                    "QPushButton:hover { border-color: #bbb; color: #333; }"
                );
                
                QPushButton *saveBtn = new QPushButton("Save Changes");
                saveBtn->setCursor(Qt::PointingHandCursor);
                saveBtn->setFixedHeight(45);
                saveBtn->setStyleSheet(
                    "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3DDC84, stop:1 #2DB66F); color: #FFFFFF; border: none; border-radius: 8px; padding: 12px 24px; font-size: 14px; font-weight: 700; } "
                    "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4EED95, stop:1 #3DDC84); }"
                    "QPushButton:pressed { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2DB66F, stop:1 #228B5A); }"
                );
                
                btnLayout->addWidget(cancelBtn);
                btnLayout->addWidget(saveBtn);
                dialogLayout->addLayout(btnLayout);
                
                QObject::connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
                QObject::connect(saveBtn, &QPushButton::clicked, [dialog, inputs, table, i]() {
                    // Update the table with the new values
                    for (int c = 0; c < inputs.size(); ++c) {
                        table->setItem(i, c, new QTableWidgetItem(inputs[c]->text()));
                    }
                    dialog->accept();
                });
                
                dialog->exec();
                dialog->deleteLater();
            });
            
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
                {"2023-10-25", "Trattoria Luigi", "Meeting", "Discussed Winter Supply"},
                {"2023-10-24", "Oileria Bella", "Support", "Clarified acidity levels"},
                {"2023-10-22", "Organic Market", "Order", "Placed bulk order #992"},
                {"2023-10-20", "Gourmet Foods", "Call", "Quality check inquiry"}
            }, true)); // Actions enabled
        } else {
             // Client Statistics Chart
             GenericBarChart *chart = new GenericBarChart("New Client Acquisition (Q2 vs Q3)");
             chart->addBar("Q2 2023", 45, QColor(52, 152, 219)); // Blue
             chart->addBar("Q3 2023", 52, QColor(61, 220, 132)); // Green

             QWidget *chartContainer = new QWidget();
             chartContainer->setStyleSheet(getCardStyle());
             QVBoxLayout *containerLayout = new QVBoxLayout(chartContainer);
             containerLayout->setContentsMargins(20, 20, 20, 20);
             containerLayout->addWidget(chart);

             cLayout->addWidget(chartContainer);
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
             // Add Print PDF Button
             QPushButton *btnPrint = new QPushButton("PRINT PDF");
             btnPrint->setStyleSheet(getButtonStyle());
             btnPrint->setCursor(Qt::PointingHandCursor);
             btnPrint->setFixedWidth(150);
             cLayout->addWidget(btnPrint, 0, Qt::AlignRight);

             cLayout->addWidget(createStyledForm("Invoice Creation Form", {{"Customer Name:", "Enter customer name"}, {"Amount:", "0.00"}, {"Date:", "YYYY-MM-DD"}}, "Submit Invoice"));
        } else if (name == "View Transactions") {
             // Add Print PDF Button
             QPushButton *btnPrint = new QPushButton("PRINT PDF");
             btnPrint->setStyleSheet(getButtonStyle());
             btnPrint->setCursor(Qt::PointingHandCursor);
             btnPrint->setFixedWidth(150);
             cLayout->addWidget(btnPrint, 0, Qt::AlignRight);

             cLayout->addWidget(createStyledTable("Recent Transactions", {"ID", "Date", "Description", "Amount", "Status"}, {
                 {"TRX-1024", "2023-10-25", "Packaging Supplies", "$150.00", "Completed"},
                 {"TRX-1023", "2023-10-24", "Client Payment - Luigi", "$1,200.00", "Completed"},
                 {"TRX-1022", "2023-10-24", "Organic Certification", "$340.50", "Pending"}
             }, true));
        } else if (name == "Expense Tracking") {
             // Add Print PDF Button
             QPushButton *btnPrint = new QPushButton("PRINT PDF");
             btnPrint->setStyleSheet(getButtonStyle());
             btnPrint->setCursor(Qt::PointingHandCursor);
             btnPrint->setFixedWidth(150);
             cLayout->addWidget(btnPrint, 0, Qt::AlignRight);

             cLayout->addWidget(createStyledTable("Expense Log", {"Date", "Category", "Description", "Amount"}, {
                 {"2023-10-25", "Travel", "Flight to NY Conference", "$450.00"},
                 {"2023-10-24", "Meals", "Team Lunch", "$125.00"}
             }, true));
        } else {
             // Financial Chart Implementation
             // Add Print PDF Button
             QPushButton *btnPrint = new QPushButton("PRINT PDF");
             btnPrint->setStyleSheet(getButtonStyle());
             btnPrint->setCursor(Qt::PointingHandCursor);
             btnPrint->setFixedWidth(150);
             cLayout->addWidget(btnPrint, 0, Qt::AlignRight);

             GenericBarChart *chart = new GenericBarChart("Financial Summary (YTD)");
             chart->addBar("Revenue", 1250500, QColor(61, 220, 132)); // Brand Green
             chart->addBar("Expenses", 850000, QColor(231, 76, 60));  // Red
             
             QWidget *chartContainer = new QWidget();
             chartContainer->setStyleSheet(getCardStyle());
             QVBoxLayout *containerLayout = new QVBoxLayout(chartContainer);
             containerLayout->setContentsMargins(20, 20, 20, 20);
             containerLayout->addWidget(chart);

             cLayout->addWidget(chartContainer);
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
    layout->addWidget(new QLabel("Stock Management"));

    QWidget *actionBar = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setSpacing(12);

    outNestedStack = new QStackedWidget();
    QStringList tabNames = {"Add Stock Item", "Supplier Management", "Stock Reports"};
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
             cLayout->addWidget(createStyledForm("Add New Stock Item", {
                 {"Item Name:", "Extra Virgin 1L"}, 
                 {"SKU:", "OIL-EV-001"}, 
                 {"Quantity:", "0"}
             }, "Add Item", 15, false));
        } else if (name == "Supplier Management") {
             cLayout->addWidget(createStyledTable("Supplier Directory", {"Supplier Name", "Contact", "Phone", "Rating"}, {
                 {"Mediterranean Olives", "Maria Rossi", "(555) 123-4567", "5/5"},
                 {"Glass Bottle Co.", "Bob Smith", "(555) 987-6543", "4/5"}
             }, true));
        } else {
             cLayout->addWidget(createStyledTable("Stock Reports", {"Item Name", "SKU", "Current Qty", "Status"}, {
                 {"Empty Bottles 500ml", "BTL-500", "50", "CRITICAL"},
                 {"Labels - 'Gold'", "LBL-GLD", "120", "LOW"}
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

static QWidget* createMaintenancePage(QStackedWidget* &outNestedStack) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(25);
    layout->addWidget(new QLabel("Maintenance Management - Machines"));

    QWidget *actionBar = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
    actionLayout->setSpacing(12);

    outNestedStack = new QStackedWidget();
    // Updated tab names for Machine Management (English)
    QStringList tabNames = {"Add Machine", "Machine List", "Maintenance History", "Statistics"};
    QList<QPushButton*> tabButtons;

    for (const auto &name : tabNames) {
        QPushButton *btn = new QPushButton(name);
        btn->setCheckable(true);
        btn->setStyleSheet(getTabButtonStyle());
        actionLayout->addWidget(btn);
        tabButtons.append(btn);

        QWidget *content = new QWidget();
        QVBoxLayout *cLayout = new QVBoxLayout(content);
        
        if (name == "Add Machine") {
             // Form for adding a new machine with the 4 specific fields
             cLayout->addWidget(createStyledForm("New Machine", {
                 {"Machine ID:", "Ex: MAC-001"},
                 {"Machine Name:", "Ex: Olive Press Alpha"},
                 {"Machine Type:", "Ex: Press, Centrifuge, Filter..."},
                 {"Machine Status:", "Ex: Normal, Broken, Maintenance"}
             }, "Add Machine"));
        } else if (name == "Machine List") {
             // Table listing the machines with the 4 columns + Actions
             cLayout->addWidget(createStyledTable("Machine List", {"ID", "Name", "Type", "Status"}, {
                 {"MAC-001", "Press Alpha", "Press", "Normal"},
                 {"MAC-002", "Centrifuge Beta", "Centrifuge", "Broken"},
                 {"MAC-003", "Filter Gamma", "Filter", "Maintenance"}
             }, true));
        } else if (name == "Maintenance History") {
             // Keep historical logs
             cLayout->addWidget(createStyledTable("Intervention History", {"Date", "Machine", "Action", "Result"}, {
                 {"2023-09-01", "MAC-001", "Belt Replacement", "Success"}
             }, true));
        } else {
             // Maintenance Statistics Chart
             GenericBarChart *chart = new GenericBarChart("Machine Status Overview");
             chart->addBar("Operational", 15, QColor(46, 204, 113)); // Green
             chart->addBar("Broken", 2, QColor(231, 76, 60));        // Red
             chart->addBar("Maintenance", 3, QColor(241, 196, 15));  // Orange

             QWidget *chartContainer = new QWidget();
             chartContainer->setStyleSheet(getCardStyle());
             QVBoxLayout *containerLayout = new QVBoxLayout(chartContainer);
             containerLayout->setContentsMargins(20, 20, 20, 20);
             containerLayout->addWidget(chart);

             cLayout->addWidget(chartContainer);
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
    QStringList tabNames = {"Product List", "Add Item", "Remove Item"};
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
    QPixmap logoPixmap("C:/Users/Samik/OneDrive/Dokumente/untitled/logo.png");
    if(!logoPixmap.isNull()) {
        brand->setPixmap(logoPixmap.scaledToHeight(45, Qt::SmoothTransformation));
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
    QPushButton *btnInventory = addNav("Stock Management");
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
