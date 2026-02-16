#include "mainwindow.h"
#include <QWidget>
#include <QComboBox>
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
        // Spacing calculation removed as it was unused and causing warnings/errors

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
            
            // Connect Delete Button
            QObject::connect(btnDelete, &QPushButton::clicked, [table, btnDelete]() {
                // Robustly find the row by mapping the button's position to the table viewport
                QPoint btnPos = btnDelete->mapTo(table->viewport(), btnDelete->rect().center());
                int row = table->rowAt(btnPos.y());
                
                if (row >= 0) {
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::question(table->window(), "Confirm Deletion", 
                                                  "Are you sure you want to delete this item?",
                                                  QMessageBox::Yes|QMessageBox::No);
                    if (reply == QMessageBox::Yes) {
                        table->removeRow(row);
                    }
                }
            });
            
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
            cLayout->addWidget(createStyledTable("Order Interaction History", {"Date", "Client", "Action", "Notes"}, {
                {"2023-10-25", "Trattoria Luigi", "Meeting", "Discussed Winter Supply"},
                {"2023-10-24", "Oileria Bella", "Support", "Clarified acidity levels"},
                {"2023-10-22", "Organic Market", "Order", "Placed bulk order #992"},
                {"2023-10-20", "Gourmet Foods", "Call", "Quality check inquiry"}
            }, true)); // Actions enabled
        } else {
             // Client Statistics Chart
             GenericBarChart *chart = new GenericBarChart("New Order Acquisition (Q2 vs Q3)");
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
             // --- Custom Transactions View ---
             QWidget *transContainer = new QWidget();
             QVBoxLayout *transLayout = new QVBoxLayout(transContainer);
             transLayout->setContentsMargins(0,0,0,0);
             transLayout->setSpacing(10);

             // 1. Control Bar
             QWidget *controlBar = new QWidget();
             QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
             controlLayout->setContentsMargins(0, 0, 0, 0);

             QLineEdit *searchEdit = new QLineEdit();
             searchEdit->setPlaceholderText("Search...");
             searchEdit->setStyleSheet(getInputStyle());
             searchEdit->setFixedWidth(150); // Reduced width

             QComboBox *searchType = new QComboBox();
             searchType->addItems({"Status", "ID"});
             searchType->setStyleSheet(getInputStyle()); 
             searchType->setFixedWidth(80); // Reduced width

             // Removed Search Button

             QLabel *lblSort = new QLabel("Sort by:");
             lblSort->setStyleSheet(getLabelStyle());
             
             QComboBox *sortType = new QComboBox();
             sortType->addItems({"Amt High-Low", "Amt Low-High"}); // Shortened text
             sortType->setStyleSheet(getInputStyle());
             sortType->setFixedWidth(130); // Reduced width

             // Removed Sort Button

             QPushButton *btnPrint = new QPushButton("PRINT PDF");
             btnPrint->setStyleSheet(getButtonStyle());
             btnPrint->setCursor(Qt::PointingHandCursor);
             btnPrint->setFixedWidth(120);

             controlLayout->addWidget(searchEdit);
             controlLayout->addWidget(searchType);
             // controlLayout->addWidget(btnSearch); // Removed
             controlLayout->addSpacing(15); // Reduced spacing
             controlLayout->addWidget(lblSort);
             controlLayout->addWidget(sortType);
             // controlLayout->addWidget(btnSort); // Removed
             controlLayout->addStretch();
             controlLayout->addWidget(btnPrint);

             transLayout->addWidget(controlBar);

             // 2. Table
             QStringList headers = {"ID", "Date", "Description", "Amount", "Status"};
             QTableWidget *table = new QTableWidget();
             table->setColumnCount(headers.size());
             table->setHorizontalHeaderLabels(headers);
             table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
             table->verticalHeader()->setVisible(false);
             table->setAlternatingRowColors(true);
             table->setStyleSheet("QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; gridline-color: transparent; border-radius: 8px; alternate-background-color: #f9fafb; }"
                                  "QHeaderView::section { background-color: #ffffff; padding: 12px; border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; color: #666; }");

             // Data
             struct Transaction { QString id; QString date; QString desc; double amount; QString status; };
             QList<Transaction> transactions = {
                 {"TRX-1024", "2023-10-25", "Packaging Supplies", 150.00, "Completed"},
                 {"TRX-1023", "2023-10-24", "Client Payment - Luigi", 1200.00, "Completed"},
                 {"TRX-1022", "2023-10-24", "Organic Certification", 340.50, "Pending"},
                 {"TRX-1021", "2023-10-23", "Maintenance Parts", 850.00, "Completed"},
                 {"TRX-1020", "2023-10-22", "Utility Bill", 210.75, "Pending"}
             };

             auto populateTable = [table](const QList<Transaction>& data) {
                 table->setRowCount(0);
                 table->setRowCount(data.size());
                 for(int i=0; i<data.size(); ++i) {
                     const auto& t = data[i];
                     table->setItem(i, 0, new QTableWidgetItem(t.id));
                     table->setItem(i, 1, new QTableWidgetItem(t.date));
                     table->setItem(i, 2, new QTableWidgetItem(t.desc));
                     // Format Amount
                     QString amtStr = "$" + QString::number(t.amount, 'f', 2);
                     if(t.amount >= 1000) {
                         int pos = amtStr.length() - 6; 
                         while(pos > 1) { amtStr.insert(pos, ","); pos-=3; }
                     }
                     table->setItem(i, 3, new QTableWidgetItem(amtStr));
                     table->setItem(i, 4, new QTableWidgetItem(t.status));
                 }
             };

             populateTable(transactions);
             transLayout->addWidget(table);

             // 3. Logic
             // Define update function
             auto updateView = [=]() {
                 QString query = searchEdit->text().toLower();
                 QString sType = searchType->currentText();
                 QString sort = sortType->currentText();
                 
                 QList<Transaction> filtered;
                 for(const auto& t : transactions) {
                     bool match = false;
                     if (sType == "Status") match = t.status.toLower().contains(query);
                     else if (sType == "ID") match = t.id.toLower().contains(query);
                     
                     if (match) filtered.append(t);
                 }

                 // Sort
                 std::sort(filtered.begin(), filtered.end(), [sort](const Transaction& a, const Transaction& b){
                     if (sort == "Amt High-Low") return a.amount > b.amount;
                     else return a.amount < b.amount;
                 });
                 
                 populateTable(filtered);
             };

             // Initial Population
             updateView();

             // Connections - Auto Update
             QObject::connect(searchEdit, &QLineEdit::textChanged, updateView);
             QObject::connect(searchType, &QComboBox::currentTextChanged, updateView);
             QObject::connect(sortType, &QComboBox::currentTextChanged, updateView);

             // Print
             QObject::connect(btnPrint, &QPushButton::clicked, [=]() {
                 QMessageBox::information(table, "Print", "Generating Financial Report PDF...");
             });

             cLayout->addWidget(transContainer);
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
             // --- Stock Reports with Search and Actions ---
             QWidget *reportContainer = new QWidget();
             QVBoxLayout *reportLayout = new QVBoxLayout(reportContainer);
             reportLayout->setContentsMargins(0,0,0,0);
             reportLayout->setSpacing(10);

             // 1. Search Bar (Like Financial: Input + Type)
             QWidget *controlBar = new QWidget();
             QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
             controlLayout->setContentsMargins(0, 0, 0, 0);

             QLineEdit *searchEdit = new QLineEdit();
             searchEdit->setPlaceholderText("Search...");
             searchEdit->setStyleSheet(getInputStyle());
             searchEdit->setFixedWidth(150); // Matches Financial Module

             QComboBox *searchType = new QComboBox();
             searchType->addItems({"Item", "Status"});
             searchType->setStyleSheet(getInputStyle()); 
             searchType->setFixedWidth(80); // Matches Financial Module

             controlLayout->addWidget(searchEdit);
             controlLayout->addWidget(searchType);
             controlLayout->addStretch();
             reportLayout->addWidget(controlBar);

             // 2. Table with Actions (Modify/Delete) - Sorted by Quantity (Low -> High)
             struct StockItem { QString name; QString sku; int qty; QString status; };
             QList<StockItem> stockItems = {
                 {"Empty Bottles 500ml", "BTL-500", 50, "CRITICAL"},
                 {"Labels - 'Gold'", "LBL-GLD", 120, "LOW"},
                 {"Extra Virgin 1L", "OIL-EV-001", 450, "NORMAL"},
                 {"Caps - Black", "CAP-BLK", 2000, "NORMAL"}
             };

             // Sort by Quantity (Ascending: Low -> High)
             std::sort(stockItems.begin(), stockItems.end(), [](const StockItem& a, const StockItem& b){
                 return a.qty < b.qty;
             });

             // Convert to String List for createStyledTable
             QVector<QStringList> tableData;
             for(const auto& item : stockItems) {
                 tableData.append({item.name, item.sku, QString::number(item.qty), item.status});
             }

             QWidget *tableWidgetWrapper = createStyledTable("Stock Reports", 
                {"Item Name", "SKU", "Current Qty", "Status"}, tableData, true); // showActions = true

             reportLayout->addWidget(tableWidgetWrapper);
             cLayout->addWidget(reportContainer);

             // 3. Search Logic
             // We need to find the QTableWidget inside the wrapper to filter it
             QTableWidget *table = tableWidgetWrapper->findChild<QTableWidget*>();
             if (table) {
                 auto updateFilter = [table, searchEdit, searchType]() {
                     QString query = searchEdit->text().toLower();
                     QString type = searchType->currentText();
                     int colIndex = (type == "Item") ? 0 : 3; // Item Name matches col 0, Status matches col 3

                     for(int i = 0; i < table->rowCount(); ++i) {
                         bool match = false;
                         if (table->item(i, colIndex)) {
                             match = table->item(i, colIndex)->text().toLower().contains(query);
                         }
                         table->setRowHidden(i, !match);
                     }
                 };

                 QObject::connect(searchEdit, &QLineEdit::textChanged, updateFilter);
                 QObject::connect(searchType, &QComboBox::currentTextChanged, updateFilter);
             }

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
             // --- Enhanced Machine List ---
             QWidget *listContainer = new QWidget();
             QVBoxLayout *listLayout = new QVBoxLayout(listContainer);
             listLayout->setContentsMargins(0,0,0,0);
             listLayout->setSpacing(10);

             // 1. Control Bar
             QWidget *controlBar = new QWidget();
             QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
             controlLayout->setContentsMargins(0, 0, 0, 0);

             // Removed Search Bar and Button as requested
             // Kept Search Type ComboBox (though functionality is limited without input)
             QComboBox *searchType = new QComboBox();
             searchType->addItems({"Type", "Status"});
             searchType->setStyleSheet(getInputStyle()); 
             searchType->setFixedWidth(100);

             QLabel *lblSort = new QLabel("Sort by:");
             lblSort->setStyleSheet(getLabelStyle());
             
             QComboBox *sortType = new QComboBox();
             sortType->addItems({"Status", "Hours"});
             sortType->setStyleSheet(getInputStyle());
             sortType->setFixedWidth(100);

             // Removed Sort Button as requested

             QPushButton *btnPrint = new QPushButton("PRINT PDF");
             btnPrint->setStyleSheet(getButtonStyle());
             btnPrint->setCursor(Qt::PointingHandCursor);
             btnPrint->setFixedWidth(120);

             controlLayout->addWidget(searchType); // Kept as requested
             controlLayout->addSpacing(20);
             controlLayout->addWidget(lblSort);
             controlLayout->addWidget(sortType);
             controlLayout->addStretch();
             controlLayout->addWidget(btnPrint);

             listLayout->addWidget(controlBar);

             // 2. Table
             QStringList headers = {"ID", "Name", "Type", "Status", "Hours", "Actions"};
             QTableWidget *table = new QTableWidget();
             table->setColumnCount(headers.size());
             table->setHorizontalHeaderLabels(headers);
             table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
             table->horizontalHeader()->setSectionResizeMode(headers.size()-1, QHeaderView::ResizeToContents);
             table->verticalHeader()->setVisible(false);
             table->setAlternatingRowColors(true);
             table->setStyleSheet("QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; gridline-color: transparent; border-radius: 8px; alternate-background-color: #f9fafb; }"
                                  "QHeaderView::section { background-color: #ffffff; padding: 12px; border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; color: #666; }");

             // Data
             struct Machine { QString id; QString name; QString type; QString status; int hours; };
             QList<Machine> machines = {
                 {"MAC-001", "Press Alpha", "Press", "Normal", 1250},
                 {"MAC-002", "Centrifuge Beta", "Centrifuge", "Broken", 800},
                 {"MAC-003", "Filter Gamma", "Filter", "Maintenance", 3200},
                 {"MAC-004", "Press Delta", "Press", "Normal", 450},
                 {"MAC-005", "Bottler Epsilon", "Bottling", "Normal", 2100}
             };

             auto populateTable = [table, headers](const QList<Machine>& data) {
                 table->setRowCount(0);
                 table->setRowCount(data.size());
                 for(int i=0; i<data.size(); ++i) {
                     const auto& m = data[i];
                     table->setItem(i, 0, new QTableWidgetItem(m.id));
                     table->setItem(i, 1, new QTableWidgetItem(m.name));
                     table->setItem(i, 2, new QTableWidgetItem(m.type));
                     table->setItem(i, 3, new QTableWidgetItem(m.status));
                     // Hours (sortable number)
                     QTableWidgetItem *hItem = new QTableWidgetItem();
                     hItem->setData(Qt::DisplayRole, m.hours);
                     table->setItem(i, 4, hItem);

                     // Actions
                     QWidget *actionWidget = new QWidget();
                     QHBoxLayout *al = new QHBoxLayout(actionWidget);
                     al->setContentsMargins(2,2,2,2);
                     QPushButton *btnMod = new QPushButton("Edit");
                     QPushButton *btnDel = new QPushButton("Del");
                     btnMod->setStyleSheet("border: 1px solid #ccc; border-radius: 4px; padding: 2px 8px; background: #eee;");
                     btnDel->setStyleSheet("border: 1px solid #d32f2f; color: #d32f2f; border-radius: 4px; padding: 2px 8px; background: #fff;");
                     al->addWidget(btnMod);
                     al->addWidget(btnDel);
                     table->setCellWidget(i, 5, actionWidget);
                 }
             };

             populateTable(machines);
             listLayout->addWidget(table);

             // 3. Logic
             // Auto-Sort on ComboBox Change
             auto sortTable = [=]() {
                QString type = sortType->currentText();
                table->setSortingEnabled(false); 
                if (type == "Status") {
                    table->sortItems(3, Qt::AscendingOrder); // Column 3 = Status
                } else if (type == "Hours") {
                    table->sortItems(4, Qt::AscendingOrder); // Column 4 = Hours
                }
             };

             // Connect Sort Type
             QObject::connect(sortType, &QComboBox::currentTextChanged, sortTable);
            
             // Connect Search Type (for future use or as a secondary sort trigger if desired)
             QObject::connect(searchType, &QComboBox::currentTextChanged, [=](const QString &text) {
                 // For now, maybe just sort by that column type?
                 // Let's implement basic sort by column if user selects Type/Status in search combo
                 table->setSortingEnabled(false);
                 if (text == "Type") table->sortItems(2, Qt::AscendingOrder);
                 else if (text == "Status") table->sortItems(3, Qt::AscendingOrder);
             });

             // Print
             QObject::connect(btnPrint, &QPushButton::clicked, [=]() {
                 QMessageBox::information(table, "Print", "Geneating PDF report for Maintenance List...");
             });
             
             cLayout->addWidget(listContainer);
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
    QStringList tabNames = {"Product List", "Add Item"};
    QList<QPushButton*> tabButtons;

    // We need to capture the table widget to update it from the "Add Item" tab
    QTableWidget *productTable = nullptr;

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
             
             // Capture the table for updates
             productTable = tableWidget->findChild<QTableWidget*>();

             listPageLayout->addWidget(tableWidget);

             // Connect print button
             // PDF Export functionality removed as per request
             // The button remains visible but inactive

             cLayout->addWidget(listPageWidget); 
        } else if (name == "Add Item") {
             // Manual form creation to allow access to inputs
             QWidget *formContainer = new QWidget();
             QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
             formLayout->setSpacing(15);
             formLayout->setContentsMargins(0, 0, 10, 0); // Slight right margin for scrollbar if needed

             // Title
             QLabel *titleLabel = new QLabel("Add New Product");
             titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; margin-bottom: 25px; border: none;");
             formLayout->addWidget(titleLabel);

             QString labelStyle = getLabelStyle();
             QString inputStyle = getInputStyle();

             // Define fields (ID is auto-generated, so removed from input)
             struct FieldInput { QString label; QString placeholder; QLineEdit *widget; };
             QList<FieldInput> inputs = {
                 {"Date Pressage:", "YYYY-MM-DD", new QLineEdit()},
                 {"Capacité:", "e.g., 500L", new QLineEdit()},
                 {"Ref Testeur:", "Enter Tester Ref", new QLineEdit()},
                 {"Qualité:", "Enter Quality Grade", new QLineEdit()},
                 {"Viscosity:", "Enter Viscosity", new QLineEdit()},
                 {"Color:", "Enter Color", new QLineEdit()},
                 {"Ref Press:", "Enter Press Ref", new QLineEdit()}
             };

             for (auto &field : inputs) {
                 QLabel *lbl = new QLabel(field.label);
                 lbl->setStyleSheet(labelStyle);
                 field.widget->setStyleSheet(inputStyle);
                 field.widget->setPlaceholderText(field.placeholder);
                 
                 formLayout->addWidget(lbl);
                 formLayout->addWidget(field.widget);
             }

             formLayout->addSpacing(20);
             QPushButton *btnAdd = new QPushButton("Add Product");
             btnAdd->setStyleSheet(getButtonStyle());
             btnAdd->setCursor(Qt::PointingHandCursor);
             btnAdd->setFixedHeight(45);
             formLayout->addWidget(btnAdd);
             formLayout->addStretch();

             // Add Logic
             QObject::connect(btnAdd, &QPushButton::clicked, [inputs, productTable, outNestedStack, tabButtons]() {
                 if (!productTable) return;

                 // 1. Auto-generate ID (Simple Logic: "CONT-" + (RowCount + 1))
                 // Note: In a real app, this should check for existing IDs or use a database counter.
                 int nextIdNum = productTable->rowCount() + 1;
                 QString newId = QString("CONT-%1").arg(nextIdNum, 3, 10, QChar('0'));

                 // 2. Collect Data
                 int row = productTable->rowCount();
                 productTable->insertRow(row);
                 
                 // Column 0: ID
                 productTable->setItem(row, 0, new QTableWidgetItem(newId));

                 // Other Columns
                 for (int i = 0; i < inputs.size(); ++i) {
                     productTable->setItem(row, i + 1, new QTableWidgetItem(inputs[i].widget->text()));
                     inputs[i].widget->clear(); // Clear input after adding
                 }

                 // Add Actions (Modify/Delete) - We need to manually replicate the actions column setup here
                 // or ideally extract that logic. For now, let's replicate the basic button creation
                 // from createStyledTable to ensure consistency.
                 QWidget *actionWidget = new QWidget();
                 QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
                 actionLayout->setContentsMargins(2, 2, 30, 2); 
                 actionLayout->setSpacing(20);
                 actionLayout->setAlignment(Qt::AlignCenter);
                 
                 QPushButton *btnModify = new QPushButton("Modify");
                 btnModify->setCursor(Qt::PointingHandCursor);
                 btnModify->setMinimumWidth(80);
                 btnModify->setFixedHeight(28);
                 btnModify->setStyleSheet("QPushButton { background-color: #ffffff; color: #333333; border: 1px solid #cccccc; border-radius: 6px; padding: 0px 8px; font-size: 13px; font-weight: 600; } QPushButton:hover { border-color: #aaaaaa; color: #000000; background-color: #f6f6f6; } QPushButton:pressed { background-color: #e6e6e6; }");

                 QPushButton *btnDelete = new QPushButton("Delete");
                 btnDelete->setCursor(Qt::PointingHandCursor);
                 btnDelete->setMinimumWidth(80);
                 btnDelete->setFixedHeight(28);
                 btnDelete->setStyleSheet("QPushButton { background-color: #ffffff; color: #d32f2f; border: 1px solid #d32f2f; border-radius: 6px; padding: 0px 8px; font-size: 13px; font-weight: 600; } QPushButton:hover { background-color: #ffebee; border-color: #b71c1c; color: #b71c1c; } QPushButton:pressed { background-color: #ffcdd2; }");

                 actionLayout->addWidget(btnModify);
                 actionLayout->addWidget(btnDelete);
                 actionLayout->addStretch();
                 
                 // Connect Delete (Copying logic from createStyledTable fix)
                 QObject::connect(btnDelete, &QPushButton::clicked, [productTable, btnDelete]() {
                    QPoint btnPos = btnDelete->mapTo(productTable->viewport(), btnDelete->rect().center());
                    int r = productTable->rowAt(btnPos.y());
                    if (r >= 0) {
                        QMessageBox::StandardButton reply;
                        reply = QMessageBox::question(productTable->window(), "Confirm Deletion", "Are you sure you want to delete this item?", QMessageBox::Yes|QMessageBox::No);
                        if (reply == QMessageBox::Yes) productTable->removeRow(r);
                    }
                 });

                 productTable->setCellWidget(row, 8, actionWidget); // Column 8 is Actions (0-7 are data)

                 // 3. Switch to List View
                 if (outNestedStack) outNestedStack->setCurrentIndex(0);
                 if (!tabButtons.isEmpty()) tabButtons.first()->setChecked(true);
             });

             // Wrap in scroll area
             QScrollArea *scrolld = new QScrollArea();
             scrolld->setWidgetResizable(true);
             scrolld->setWidget(formContainer);
             scrolld->setFrameShape(QFrame::NoFrame);
             cLayout->addWidget(scrolld);
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
