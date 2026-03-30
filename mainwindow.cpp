#include "mainwindow.h"
#include "AuthWidgets.h"
#include "EyeSaverButton.h"
#include "article.h"
#include "commande.h"
#include "transaction.h"
#include "produit.h"
#include "personnel.h"
#include "machine.h"
#include "smtp.h"
#include <functional>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
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
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextStream>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLinearGradient>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPageSize>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPrinter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>
#include <algorithm>

Qt::Edges MainWindow::getEdge(const QPoint &pos) {
  int x = pos.x();
  int y = pos.y();
  int w = width();
  int h = height();
  int m = m_edgeMargin;

  Qt::Edges edges = Qt::Edges();

  if (x <= m)
    edges |= Qt::LeftEdge;
  if (x >= w - m)
    edges |= Qt::RightEdge;
  if (y <= m)
    edges |= Qt::TopEdge;
  if (y >= h - m)
    edges |= Qt::BottomEdge;

  return edges;
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    Qt::Edges edges = getEdge(event->pos());
    if (edges != 0) {
      if (windowHandle()) {
        windowHandle()->startSystemResize(edges);
      }
    } else if (event->pos().y() <= 35) { // Title Bar Height
      if (windowHandle()) {
        windowHandle()->startSystemMove();
      }
    }
    event->accept();
  }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
  Qt::Edges edges = getEdge(event->pos());

  if ((edges & Qt::LeftEdge && edges & Qt::TopEdge) ||
      (edges & Qt::RightEdge && edges & Qt::BottomEdge))
    setCursor(Qt::SizeFDiagCursor);
  else if ((edges & Qt::RightEdge && edges & Qt::TopEdge) ||
           (edges & Qt::LeftEdge && edges & Qt::BottomEdge))
    setCursor(Qt::SizeBDiagCursor);
  else if (edges & Qt::LeftEdge || edges & Qt::RightEdge)
    setCursor(Qt::SizeHorCursor);
  else if (edges & Qt::TopEdge || edges & Qt::BottomEdge)
    setCursor(Qt::SizeVerCursor);
  else
    setCursor(Qt::ArrowCursor);

  QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
  m_isResizing = false;
  m_isMoving = false;
  QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->pos().y() <= 35) {
    if (isMaximized())
      showNormal();
    else
      showMaximized();
  }
}

QWidget *MainWindow::createTitleBar() {
  QWidget *bar = new QWidget();
  bar->setFixedHeight(35);
  bar->setStyleSheet("background-color: #111; color: #999;");

  QHBoxLayout *layout = new QHBoxLayout(bar);
  layout->setContentsMargins(15, 0, 0, 0);
  layout->setSpacing(0);

  QLabel *title = new QLabel("oil press manager");
  title->setStyleSheet("font-weight: bold; font-size: 13px; color: #777;");
  layout->addWidget(title);
  layout->addStretch();

  // Buttons on the RIGHT - Opposite size (Rectangular 45x35)
  QPushButton *minBtn = new QPushButton("−");
  minBtn->setFixedSize(45, 35);
  minBtn->setCursor(Qt::PointingHandCursor);
  minBtn->setStyleSheet("QPushButton { background: transparent; border: none; "
                        "color: #999; font-size: 16px; } QPushButton:hover { "
                        "background-color: #333; color: white; }");

  QPushButton *maxBtn = new QPushButton("▢");
  maxBtn->setFixedSize(45, 35);
  maxBtn->setCursor(Qt::PointingHandCursor);
  maxBtn->setStyleSheet("QPushButton { background: transparent; border: none; "
                        "color: #999; font-size: 14px; } QPushButton:hover { "
                        "background-color: #333; color: white; }");

  QPushButton *closeBtn = new QPushButton("✕");
  closeBtn->setFixedSize(45, 35);
  closeBtn->setCursor(Qt::PointingHandCursor);
  closeBtn->setStyleSheet(
      "QPushButton { background: transparent; border: none; color: #999; "
      "font-size: 16px; } QPushButton:hover { background-color: #e74c3c; "
      "color: white; }");

  layout->addWidget(minBtn);
  layout->addWidget(maxBtn);
  layout->addWidget(closeBtn);

  connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
  connect(maxBtn, &QPushButton::clicked, this, [this]() {
    if (isMaximized())
      showNormal();
    else
      showMaximized();
  });
  connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

  return bar;
}

// --- Custom Bar Chart Widget (No Dependencies) ---
// --- Custom Bar Chart Widget (No Dependencies) ---
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
      // Format number (k for thousands, M for millions if needed, or just
      // standard)
      QString label = QString::number(value, 'f', 0);
      if (value >= 1000000)
        label = QString::number(value / 1000000.0, 'f', 1) + "M";
      else if (value >= 1000)
        label = QString::number(value / 1000.0, 'f', 0) + "k";

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

      // Distribute evenly
      int x =
          leftMargin + (i * availableSpace) + (availableSpace - barWidth) / 2;
      int y = topMargin + chartHeight - barH;

      QRect barRect(x, y, barWidth, barH);

      // Draw Bar (Flat Color, No Gradient)
      painter.setBrush(bar.color);
      painter.setPen(Qt::NoPen);
      painter.drawRect(
          barRect); // Sharp corners for technical look, or rounded if preferred

      // Draw Value on Top
      painter.setPen(QColor("#000000"));
      painter.setFont(QFont("Segoe UI", 10, QFont::Bold));

      // Fix: Use 'f' format to avoid scientific notation (e.g., 1.25e+06)
      // Precision 0 means no decimals (1250500)
      QString valText = QString::number(bar.value, 'f', 0);

      // Format large numbers with commas for readability (manual poor-man's
      // locale)
      if (bar.value >= 1000) {
        // Insert commas every 3 digits from right
        int pos = valText.length() - 3;
        while (pos > 0) {
          valText.insert(pos, ",");
          pos -= 3;
        }
      }

      painter.drawText(QRect(x - 10, y - 25, barWidth + 20, 20),
                       Qt::AlignCenter, valText);

      // Draw X-Axis Label
      painter.setPen(QColor("#333333"));
      painter.setFont(QFont("Segoe UI", 10)); // Regular font for labels
      QRect labelRect(x - 20, topMargin + chartHeight + 10, barWidth + 40,
                      40); // Allow wrapping
      painter.drawText(labelRect, Qt::AlignCenter | Qt::TextWordWrap,
                       bar.label);
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
  return "font-size: 14px; font-weight: 700; color: #333; margin-top: 10px; "
         "margin-bottom: 6px;";
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

  QTableWidget *table = new QTableWidget();
  table->setColumnCount(finalHeaders.size());
  table->setHorizontalHeaderLabels(finalHeaders);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  if (showActions) {
    table->horizontalHeader()->setSectionResizeMode(
        finalHeaders.size() - 1, QHeaderView::ResizeToContents);
  }
  table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft |
                                                 Qt::AlignVCenter);
  table->verticalHeader()->setVisible(false);

  // Auto-hide ID columns
  for (int i = 0; i < finalHeaders.size(); ++i) {
    QString header = finalHeaders[i].toUpper();
    if (header == "ID" || header == "ID CONTAINER") {
      table->setColumnHidden(i, true);
    }
  }
  table->verticalHeader()->setDefaultSectionSize(60); // Increased height
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setAlternatingRowColors(true);
  table->setStyleSheet(
      "QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; "
      "gridline-color: transparent; border-radius: 8px; "
      "alternate-background-color: #f9fafb; }"
      "QHeaderView::section { background-color: #ffffff; padding: 12px; "
      "border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; "
      "color: #666; text-transform: uppercase; font-size: 12px; }"
      "QTableWidget::item { padding: 12px; border-bottom: 1px solid #f5f5f5; "
      "color: #333; }"
      "QTableWidget::item:selected { background-color: #e6f9ef; color: "
      "#1a1a1a; }");

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
      actionLayout->setSpacing(20);                // Increased spacing
      actionLayout->setAlignment(Qt::AlignCenter); // Ensure vertical centering

      QPushButton *btnModify = new QPushButton("Edit");
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
          "font-size: 13px;"  // Matched to tab font size
          "font-weight: 600;" // Matched to tab font weight
          "} "
          "QPushButton:hover { border-color: #aaaaaa; color: #000000; "
          "background-color: #f6f6f6; }"
          "QPushButton:pressed { background-color: #e6e6e6; }");

      QPushButton *btnDelete = new QPushButton("Remove");
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
          "font-size: 13px;"  // Matched to tab font size
          "font-weight: 600;" // Matched to tab font weight
          "} "
          "QPushButton:hover { background-color: #ffebee; border-color: "
          "#b71c1c; color: #b71c1c; }"
          "QPushButton:pressed { background-color: #ffcdd2; }");

      actionLayout->addWidget(btnModify);
      actionLayout->addWidget(btnDelete);

      if (showViewAction) {
        QPushButton *btnAccept = new QPushButton("Accept");
        btnAccept->setCursor(Qt::PointingHandCursor);
        btnAccept->setMinimumWidth(60);
        btnAccept->setFixedHeight(28);
        btnAccept->setStyleSheet(
            "QPushButton {"
            "background-color: #ffffff;"
            "color: #3DDC84;"
            "border: 1px solid #3DDC84;"
            "border-radius: 6px;"
            "padding: 0px 8px;"
            "font-size: 13px;"
            "font-weight: 600;"
            "} "
            "QPushButton:hover { background-color: #e6f9ef; color: #34c772; "
            "border-color: #34c772; }");
        actionLayout->addWidget(btnAccept);
        QObject::connect(btnAccept, &QPushButton::clicked, [=]() {
          QMessageBox::information(nullptr, "Leave Request",
                                   "Request for " + data[i][0] +
                                       " has been accepted.");
        });
      }

      if (showQrAction) {
        QPushButton *btnQr = new QPushButton("QR Code");
        btnQr->setCursor(Qt::PointingHandCursor);
        btnQr->setStyleSheet(
            "QPushButton { background-color: #9b59b6; color: white; border: "
            "none; border-radius: 6px; padding: 6px 12px; font-size: 13px; "
            "font-weight: 600; } QPushButton:hover { background-color: "
            "#8e44ad; }");

        // Capture row data for QR
        QString name = data[i].size() > 1 ? data[i][1] : "Employee";
        QObject::connect(btnQr, &QPushButton::clicked, [=]() {
          QMessageBox::information(nullptr, "QR Code Generated",
                                   "QR Code generated for: " + name +
                                       "\n(Sent to printer)");
        });
        actionLayout->addWidget(btnQr);
      }

      actionLayout->addStretch();

      // Connect Delete Button
      QObject::connect(btnDelete, &QPushButton::clicked, [table, btnDelete]() {
        // Robustly find the row by mapping the button's position to the table
        // viewport
        QPoint btnPos =
            btnDelete->mapTo(table->viewport(), btnDelete->rect().center());
        int row = table->rowAt(btnPos.y());

        if (row >= 0) {
          QMessageBox::StandardButton reply;
          reply = QMessageBox::question(
              table->window(), "Confirm Deletion",
              "Are you sure you want to delete this item?",
              QMessageBox::Yes | QMessageBox::No);
          if (reply == QMessageBox::Yes) {
            table->removeRow(row);
          }
        }
      });

      // Capture row data for the modify dialog
      QStringList rowData = data[i];
      QStringList colHeaders = headers;
      QObject::connect(
          btnModify, &QPushButton::clicked, [rowData, colHeaders, table, i]() {
            QDialog *dialog = new QDialog(table->window());
            dialog->setWindowTitle("Modify Item");
            dialog->setMinimumWidth(450);
            dialog->setStyleSheet(
                "QDialog { background-color: #ffffff; }"
                "QLabel { font-size: 14px; font-weight: 700; color: #333; }"
                "QLineEdit { background-color: #fcfcfc; border: 1px solid "
                "#e0e0e0; border-radius: 8px; padding: 10px 14px; font-size: "
                "14px; color: #333; min-height: 35px; }"
                "QLineEdit:focus { border: 2px solid #3DDC84; "
                "background-color: #ffffff; }");

            QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
            dialogLayout->setContentsMargins(30, 30, 30, 30);
            dialogLayout->setSpacing(15);

            QLabel *titleLabel = new QLabel("Modify Item");
            titleLabel->setStyleSheet("font-size: 22px; font-weight: 800; "
                                      "color: #1a1a1a; margin-bottom: 10px;");
            dialogLayout->addWidget(titleLabel);

            QList<QLineEdit *> inputs;
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
                "QPushButton { background-color: #ffffff; color: #555; border: "
                "1px solid #ddd; border-radius: 8px; padding: 12px 24px; "
                "font-size: 14px; font-weight: 600; } "
                "QPushButton:hover { border-color: #bbb; color: #333; }");

            QPushButton *saveBtn = new QPushButton("Save Changes");
            saveBtn->setCursor(Qt::PointingHandCursor);
            saveBtn->setFixedHeight(45);
            saveBtn->setStyleSheet(
                "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, "
                "y2:1, stop:0 #3DDC84, stop:1 #2DB66F); color: #FFFFFF; "
                "border: none; border-radius: 8px; padding: 12px 24px; "
                "font-size: 14px; font-weight: 700; } "
                "QPushButton:hover { background: qlineargradient(x1:0, y1:0, "
                "x2:0, y2:1, stop:0 #4EED95, stop:1 #3DDC84); }"
                "QPushButton:pressed { background: qlineargradient(x1:0, y1:0, "
                "x2:0, y2:1, stop:0 #2DB66F, stop:1 #228B5A); }");

            btnLayout->addWidget(cancelBtn);
            btnLayout->addWidget(saveBtn);
            dialogLayout->addLayout(btnLayout);

            QObject::connect(cancelBtn, &QPushButton::clicked, dialog,
                             &QDialog::reject);
            QObject::connect(
                saveBtn, &QPushButton::clicked, [dialog, inputs, table, i]() {
                  // Update the table with the new values
                  for (int c = 0; c < inputs.size(); ++c) {
                    table->setItem(i, c,
                                   new QTableWidgetItem(inputs[c]->text()));
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

static void setupTabNavigation(const QList<QPushButton *> &buttons,
                               QStackedWidget *stack) {
  for (int i = 0; i < buttons.size(); ++i) {
    QPushButton *btn = buttons[i];
    QObject::connect(btn, &QPushButton::clicked, [=]() {
      stack->setCurrentIndex(i);
      for (auto *b : buttons)
        b->setChecked(b == btn);
    });
  }
}

static QWidget *createClientPage(QStackedWidget *&outNestedStack) {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(40, 40, 40, 40);
  layout->setSpacing(25);

  layout->addWidget(new QLabel("Order Management"));

  QWidget *actionBar = new QWidget();
  QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
  actionLayout->setSpacing(12);

  outNestedStack = new QStackedWidget();
  QStringList tabNames = {"New Order", "Edit Order", "Order Hub", "Analytics"};
  QList<QPushButton *> tabButtons;

  // Helper functions used internally to build repetitive UI
  auto addField = [](QVBoxLayout* layout, const QString& labelStyle, const QString& inputStyle, const QString& label, const QString& placeholder, QWidget*& outWidget, bool isDate = false) {
    QLabel *lbl = new QLabel(label);
    lbl->setStyleSheet(labelStyle);
    layout->addWidget(lbl);
    if (isDate) {
        QDateEdit *dateEdit = new QDateEdit(QDate::currentDate());
        dateEdit->setDisplayFormat("yyyy-MM-dd");
        dateEdit->setCalendarPopup(true);
        dateEdit->setStyleSheet(inputStyle);
        outWidget = dateEdit;
    } else {
        QLineEdit *inp = new QLineEdit();
        inp->setStyleSheet(inputStyle);
        inp->setPlaceholderText(placeholder);
        outWidget = inp;
    }
    layout->addWidget(outWidget);
  };

  for (const auto &name : tabNames) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(getTabButtonStyle());
    actionLayout->addWidget(btn);
    tabButtons.append(btn);

    QWidget *content = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout(content);

    if (name == "New Order") {
      QWidget *formContainer = new QWidget();
      formContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      formContainer->setStyleSheet(".QWidget { background-color: #ffffff; border-radius: 10px; border: 1px solid #eee; }");
      QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
      outerLayout->setContentsMargins(30, 30, 30, 30);
      
      QWidget *formContent = new QWidget();
      QVBoxLayout *formLayout = new QVBoxLayout(formContent);
      formLayout->setContentsMargins(0, 0, 10, 0);
      formLayout->setSpacing(15);

      QLabel *titleLabel = new QLabel("New Order Registration");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; margin-bottom: 25px; border: none;");
      formLayout->addWidget(titleLabel);

      QString labelStyle = getLabelStyle();
      QString inputStyle = getInputStyle();

      QWidget *wId, *wDate, *wClient, *wDelivery;
      addField(formLayout, labelStyle, inputStyle, "ID:", "Enter order ID", wId, false);
      addField(formLayout, labelStyle, inputStyle, "Order Date:", "", wDate, true);
      addField(formLayout, labelStyle, inputStyle, "Client's Name:", "Enter client's name", wClient, false);
      addField(formLayout, labelStyle, inputStyle, "Delivery Date:", "", wDelivery, true);

      QLabel *stateLbl = new QLabel("State:");
      stateLbl->setStyleSheet(labelStyle);
      formLayout->addWidget(stateLbl);

      QWidget *radioWidget = new QWidget();
      QHBoxLayout *radioLayout = new QHBoxLayout(radioWidget);
      radioLayout->setContentsMargins(0, 0, 0, 0);
      QRadioButton *pendingRadio = new QRadioButton("Pending");
      QRadioButton *completedRadio = new QRadioButton("Completed");
      pendingRadio->setChecked(true);
      pendingRadio->setStyleSheet("QRadioButton { font-size: 14px; color: #333; }");
      completedRadio->setStyleSheet("QRadioButton { font-size: 14px; color: #333; }");
      radioLayout->addWidget(pendingRadio);
      radioLayout->addWidget(completedRadio);
      radioLayout->addStretch();
      formLayout->addWidget(radioWidget);

      formLayout->addSpacing(20);
      QPushButton *btnSubmit = new QPushButton("Register Order");
      btnSubmit->setStyleSheet(getButtonStyle());
      btnSubmit->setCursor(Qt::PointingHandCursor);
      btnSubmit->setFixedHeight(45);
      formLayout->addWidget(btnSubmit);
      formLayout->addStretch();

      QScrollArea *scrollArea = new QScrollArea();
      scrollArea->setWidgetResizable(true);
      scrollArea->setFrameShape(QFrame::NoFrame);
      scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; } QWidget { background: transparent; }");
      scrollArea->setWidget(formContent);
      outerLayout->addWidget(scrollArea);

      cLayout->addWidget(formContainer);

      // Connect Button
      QObject::connect(btnSubmit, &QPushButton::clicked, [=]() {
          int id = static_cast<QLineEdit*>(wId)->text().toInt();
          QDate date = static_cast<QDateEdit*>(wDate)->date();
          QString client = static_cast<QLineEdit*>(wClient)->text();
          QDate delivery = static_cast<QDateEdit*>(wDelivery)->date();
          QString state = pendingRadio->isChecked() ? "Pending" : "Completed";

          Commande c(id, date, state, client, delivery);
          if (c.ajouter()) {
              QMessageBox::information(nullptr, "Success", "Order added successfully!");
          } else {
              QMessageBox::critical(nullptr, "Error", "Failed to add order.");
          }
      });

    } else if (name == "Edit Order") {
      QWidget *searchBar = new QWidget();
      searchBar->setStyleSheet(getCardStyle());
      QHBoxLayout *sLayout = new QHBoxLayout(searchBar);
      QLineEdit *searchInp = new QLineEdit();
      searchInp->setStyleSheet("border: none; font-size: 14px;");
      searchInp->setPlaceholderText("Search Order ID...");
      QPushButton *sBtn = new QPushButton("Search (Not used yet)");
      sBtn->setStyleSheet(getButtonStyle());
      sBtn->setFixedWidth(150);
      sLayout->addWidget(searchInp);
      sLayout->addWidget(sBtn);
      cLayout->addWidget(searchBar);
      cLayout->addSpacing(15);
      
      // Inline edit form
      QWidget *formContainer = new QWidget();
      formContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      formContainer->setStyleSheet(".QWidget { background-color: #ffffff; border-radius: 10px; border: 1px solid #eee; }");
      QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
      outerLayout->setContentsMargins(30, 30, 30, 30);
      
      QWidget *formContent = new QWidget();
      QVBoxLayout *formLayout = new QVBoxLayout(formContent);
      formLayout->setContentsMargins(0, 0, 10, 0);
      formLayout->setSpacing(15);

      QLabel *titleLabel = new QLabel("Edit Order Details");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; margin-bottom: 25px; border: none;");
      formLayout->addWidget(titleLabel);

      QString labelStyle = getLabelStyle();
      QString inputStyle = getInputStyle();

      QWidget *wId, *wDate, *wClient, *wDelivery;
      addField(formLayout, labelStyle, inputStyle, "ID:", "Enter order ID", wId, false);
      static_cast<QLineEdit*>(wId)->setReadOnly(true); // Don't let users edit ID in this static form directly without searching
      addField(formLayout, labelStyle, inputStyle, "Order Date:", "", wDate, true);
      addField(formLayout, labelStyle, inputStyle, "Client's Name:", "Enter client's name", wClient, false);
      addField(formLayout, labelStyle, inputStyle, "Delivery Date:", "", wDelivery, true);

      QLabel *stateLbl = new QLabel("State:");
      stateLbl->setStyleSheet(labelStyle);
      formLayout->addWidget(stateLbl);

      QWidget *radioWidget = new QWidget();
      QHBoxLayout *radioLayout = new QHBoxLayout(radioWidget);
      radioLayout->setContentsMargins(0, 0, 0, 0);
      QRadioButton *pendingRadio = new QRadioButton("Pending");
      QRadioButton *completedRadio = new QRadioButton("Completed");
      pendingRadio->setChecked(true);
      pendingRadio->setStyleSheet("QRadioButton { font-size: 14px; color: #333; }");
      completedRadio->setStyleSheet("QRadioButton { font-size: 14px; color: #333; }");
      radioLayout->addWidget(pendingRadio);
      radioLayout->addWidget(completedRadio);
      radioLayout->addStretch();
      formLayout->addWidget(radioWidget);

      formLayout->addSpacing(20);
      QPushButton *btnSubmit = new QPushButton("Update Order");
      btnSubmit->setStyleSheet(getButtonStyle());
      btnSubmit->setCursor(Qt::PointingHandCursor);
      btnSubmit->setFixedHeight(45);
      formLayout->addWidget(btnSubmit);
      formLayout->addStretch();

      QScrollArea *scrollArea = new QScrollArea();
      scrollArea->setWidgetResizable(true);
      scrollArea->setFrameShape(QFrame::NoFrame);
      scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; } QWidget { background: transparent; }");
      scrollArea->setWidget(formContent);
      outerLayout->addWidget(scrollArea);

      cLayout->addWidget(formContainer);

      // We will implement searching and full update logic later.
      QObject::connect(btnSubmit, &QPushButton::clicked, [=]() {
          int id = static_cast<QLineEdit*>(wId)->text().toInt();
          QDate date = static_cast<QDateEdit*>(wDate)->date();
          QString client = static_cast<QLineEdit*>(wClient)->text();
          QDate delivery = static_cast<QDateEdit*>(wDelivery)->date();
          QString state = pendingRadio->isChecked() ? "Pending" : "Completed";

          Commande c(id, date, state, client, delivery);
          if (c.modifier()) {
              QMessageBox::information(nullptr, "Success", "Order updated successfully!");
          } else {
              QMessageBox::critical(nullptr, "Error", "Failed to update order. Ensure ID is correct.");
          }
      });
    } else if (name == "Order Hub") {
      // --- Order History with Search & Sort ---

      // Container
      QWidget *historyContainer = new QWidget();
      QVBoxLayout *histLayout = new QVBoxLayout(historyContainer);
      histLayout->setContentsMargins(0, 0, 0, 0);
      histLayout->setSpacing(10);

      // 1. Controls (Search + Sort)
      QWidget *controlBar = new QWidget();
      QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
      controlLayout->setContentsMargins(0, 0, 0, 0);

      // Search
      QLineEdit *searchEdit = new QLineEdit();
      searchEdit->setPlaceholderText("Search Orders...");
      searchEdit->setStyleSheet(getInputStyle());
      searchEdit->setMinimumWidth(200);

      // Sort
      QLabel *lblSort = new QLabel("Sort by:");
      lblSort->setStyleSheet(getLabelStyle());
      lblSort->setSizePolicy(QSizePolicy::Fixed,
                             QSizePolicy::Fixed); // scanning fix

      QComboBox *sortCombo = new QComboBox();
      sortCombo->addItems({"Date (Newest)", "Date (Oldest)",
                           "Amount (High-Low)", "Amount (Low-High)"});
      sortCombo->setStyleSheet(getInputStyle());
      sortCombo->setFixedWidth(180);

      sortCombo->setFixedWidth(180);

      // Print PDF Button
      QPushButton *btnPrint = new QPushButton("Print PDF");
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setFixedWidth(120);

      // Refresh Button
      QPushButton *btnRefresh = new QPushButton("Refresh");
      btnRefresh->setCursor(Qt::PointingHandCursor);
      btnRefresh->setStyleSheet(getButtonStyle());
      btnRefresh->setFixedWidth(100);

      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(15);
      controlLayout->addWidget(lblSort);
      controlLayout->addWidget(sortCombo);
      controlLayout->addStretch();
      controlLayout->addWidget(btnRefresh);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnPrint);

      histLayout->addWidget(controlBar);

      // 2. Table
      QStringList headers = {"ID", "Date", "State", "Client", "Delivery", "Actions"};
      QTableWidget *table = new QTableWidget();
      table->setColumnCount(headers.size());
      table->setHorizontalHeaderLabels(headers);
      table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      table->horizontalHeader()->setSectionResizeMode(
          headers.size() - 1, QHeaderView::ResizeToContents);
      table->verticalHeader()->setVisible(false);
      table->verticalHeader()->setDefaultSectionSize(60);
      table->setAlternatingRowColors(true);
      table->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: "
          "#ffffff; gridline-color: transparent; border-radius: 8px; "
          "alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; "
          "border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; "
          "color: #666; }"
          "QTableWidget::item { padding: 12px; border-bottom: 1px solid "
          "#f5f5f5; color: #333; }");

      histLayout->addWidget(table);

      // Connect Print (Moved here to capture 'table' correctly)
      QObject::connect(btnPrint, &QPushButton::clicked, [table]() {
        QString fileName = QFileDialog::getSaveFileName(
            table->window(), "Export Order History", QString(),
            "PDF Files (*.pdf)");
        if (fileName.isEmpty())
          return;
        if (QFileInfo(fileName).suffix().isEmpty())
          fileName.append(".pdf");

        QPrinter printer(QPrinter::PrinterResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setPageSize(
            QPageSize(QPageSize::A4)); // Updated for Qt6 compatibility
        printer.setOutputFileName(fileName);

        QTextDocument doc;
        QString html = "<h1 style='text-align:center; color:#333;'>Order "
                       "History Report</h1>";
        html += "<h3 style='text-align:center; color:#666;'>" +
                QDate::currentDate().toString("dd MMMM yyyy") + "</h3><br>";
        html +=
            "<table border='1' cellspacing='0' cellpadding='6' width='100%' "
            "style='border-collapse:collapse; border-color:#ccc;'>";

        // Headers
        html += "<thead style='background-color:#f2f2f2;'><tr>";
        for (int c = 0; c < table->columnCount() - 1;
             ++c) { // Skip 'Actions' column
          html += "<th style='padding:8px;'>" +
                  table->horizontalHeaderItem(c)->text() + "</th>";
        }
        html += "</tr></thead>";

        // Body
        html += "<tbody>";
        for (int r = 0; r < table->rowCount(); ++r) {
          html += "<tr>";
          for (int c = 0; c < table->columnCount() - 1; ++c) {
            QTableWidgetItem *item = table->item(r, c);
            html += "<td style='padding:8px;'>" + (item ? item->text() : "") +
                    "</td>";
          }
          html += "</tr>";
        }
        html += "</tbody></table>";

        doc.setHtml(html);
        doc.setPageSize(printer.pageRect(QPrinter::Point).size());
        doc.print(&printer);

        QMessageBox::information(table->window(), "Success",
                                 "PDF Exported Successfully!");
      });

      // 3. Data & Logic
      auto updateTable = [table, searchEdit, sortCombo]() {
        Commande c;
        QSqlQueryModel *model = c.afficher();
        if (!model) return;

        QString query = searchEdit->text().toLower();
        // Sorting might be complex to implement purely in C++ with QSqlQueryModel if we don't use QSortFilterProxyModel,
        // For now, we will just filter visually or fetch sorted from DB. This is a simple visual filter.

        table->setRowCount(0);
        int rowIdx = 0;
        for (int i = 0; i < model->rowCount(); ++i) {
          QString id = model->record(i).value("ID_COMMANDE").toString();
          QString date = model->record(i).value("DATE_COMMANDE").toDate().toString("yyyy-MM-dd");
          QString etat = model->record(i).value("ETAT_COMMANDE").toString();
          QString client = model->record(i).value("NOM_CLIENT").toString();
          QString livraison = model->record(i).value("DATE_LIVRAISON").toDate().toString("yyyy-MM-dd");

          bool match = client.toLower().contains(query) || id.contains(query) || etat.toLower().contains(query);
          if (!match && !query.isEmpty()) continue;

          table->insertRow(rowIdx);
          table->setItem(rowIdx, 0, new QTableWidgetItem(id));
          table->setItem(rowIdx, 1, new QTableWidgetItem(date));
          table->setItem(rowIdx, 2, new QTableWidgetItem(etat));
          table->setItem(rowIdx, 3, new QTableWidgetItem(client));
          table->setItem(rowIdx, 4, new QTableWidgetItem(livraison));

          // Action Column
          QWidget *actionWidget = new QWidget();
          QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
          actionLayout->setContentsMargins(5, 2, 5, 2);
          actionLayout->setSpacing(5);

          QPushButton *btnModify = new QPushButton("Edit");
          btnModify->setCursor(Qt::PointingHandCursor);
          btnModify->setMinimumWidth(80);
          btnModify->setFixedHeight(28);
          btnModify->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#cccccc; border-radius: 6px; padding: 0px 8px; font-weight: "
              "normal; font-size: 13px; color: #0066cc; } QPushButton:hover { "
              "border-color: #aaaaaa; color: #004c99; background-color: "
              "#f6f6f6; }");

          QPushButton *btnDelete = new QPushButton("Remove");
          btnDelete->setCursor(Qt::PointingHandCursor);
          btnDelete->setMinimumWidth(80);
          btnDelete->setFixedHeight(28);
          btnDelete->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#d32f2f; color: #d32f2f; border-radius: 6px; padding: 0px 8px; "
              "font-weight: 600; font-size: 13px; } QPushButton:hover { "
              "background-color: #ffebee; border-color: #b71c1c; color: "
              "#b71c1c; }");

          actionLayout->addWidget(btnModify);
          actionLayout->addWidget(btnDelete);
          table->setCellWidget(rowIdx, 5, actionWidget);

          int orderId = id.toInt();

          // Connect Modify
          QObject::connect(btnModify, &QPushButton::clicked, [table, orderId, id, date, etat, client, livraison]() {
                QDialog dlg(table->window());
                dlg.setWindowTitle("Edit Order Details");
                dlg.setModal(true);
                dlg.setMinimumWidth(450);
                dlg.setStyleSheet("QDialog { background-color: #ffffff; border-radius: 12px; }");

                QVBoxLayout *mainV = new QVBoxLayout(&dlg);
                mainV->setContentsMargins(30, 30, 30, 30);
                mainV->setSpacing(20);

                QLabel *title = new QLabel("Update Order Information");
                title->setStyleSheet("font-size: 20px; font-weight: 700; color: #1a1a1a;");
                mainV->addWidget(title);

                QFormLayout *form = new QFormLayout();
                form->setSpacing(15);
                form->setLabelAlignment(Qt::AlignLeft);
                
                // Fields
                QDateEdit *dateEdit = new QDateEdit(QDate::fromString(date, "yyyy-MM-dd"));
                dateEdit->setDisplayFormat("yyyy-MM-dd");
                dateEdit->setCalendarPopup(true);
                QLineEdit *clientEdit = new QLineEdit(client);
                QDateEdit *livraisonEdit = new QDateEdit(QDate::fromString(livraison, "yyyy-MM-dd"));
                livraisonEdit->setDisplayFormat("yyyy-MM-dd");
                livraisonEdit->setCalendarPopup(true);

                auto styleField = [&](QWidget *w) {
                  w->setStyleSheet(
                      "QWidget { background-color: #f9fafb; border: 1px "
                      "solid #eaeaea; border-radius: 8px; padding: 10px; "
                      "font-size: 14px; color: #333; } QWidget:focus { "
                      "border-color: #3DDC84; background-color: #ffffff; }");
                  w->setFixedHeight(40);
                };

                styleField(dateEdit);
                styleField(clientEdit);
                styleField(livraisonEdit);

                QWidget *radioWidget = new QWidget();
                radioWidget->setStyleSheet("background: transparent; border: none;");
                QHBoxLayout *radioLayout = new QHBoxLayout(radioWidget);
                radioLayout->setContentsMargins(0, 0, 0, 0);
                QRadioButton *pendingRadio = new QRadioButton("Pending");
                QRadioButton *completedRadio = new QRadioButton("Completed");
                pendingRadio->setStyleSheet("QRadioButton { font-size: 14px; color: #333; background: transparent; border: none; }");
                completedRadio->setStyleSheet("QRadioButton { font-size: 14px; color: #333; background: transparent; border: none; }");
                if (etat == "Completed") {
                     completedRadio->setChecked(true);
                } else {
                     pendingRadio->setChecked(true);
                }
                radioLayout->addWidget(pendingRadio);
                radioLayout->addWidget(completedRadio);
                radioLayout->addStretch();

                auto addRow = [&](const QString &label, QWidget *w) {
                  QLabel *l = new QLabel(label);
                  l->setStyleSheet("font-weight: 600; color: #444; font-size: 13px; background: transparent; border: none;");
                  form->addRow(l, w);
                };

                addRow("Order Date:", dateEdit);
                addRow("Client Name:", clientEdit);
                addRow("Delivery Date:", livraisonEdit);
                addRow("State:", radioWidget);

                mainV->addLayout(form);
                mainV->addSpacing(10);

                QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
                bbox->setStyleSheet(
                    "QDialogButtonBox { background: transparent; border: none; }"
                    "QPushButton { padding: 10px 24px; border-radius: 8px; "
                    "font-weight: 700; font-size: 14px; min-width: 100px; }"
                    "QPushButton[text='Save'] { background-color: #3DDC84; "
                    "color: white; border: none; }"
                    "QPushButton[text='Save']:hover { background-color: "
                    "#34c772; }"
                    "QPushButton[text='Cancel'] { background-color: #ffffff; "
                    "color: #666; border: 1px solid #ddd; }"
                    "QPushButton[text='Cancel']:hover { background-color: "
                    "#f9fafb; }");
                mainV->addWidget(bbox);

                QObject::connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
                QObject::connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

                if (dlg.exec() == QDialog::Accepted) {
                    QString newState = pendingRadio->isChecked() ? "Pending" : "Completed";
                    Commande c(orderId, dateEdit->date(), newState, clientEdit->text(), livraisonEdit->date());
                    if (c.modifier()) {
                        QMessageBox::information(table->window(), "Success", "Order updated successfully!");
                        // Ideally we'd refresh the table here. In the full application, we should trigger the outer updateTable() to re-query the DB.
                    } else {
                         QMessageBox::critical(table->window(), "Error", "Failed to update order.");
                    }
                }
          });

          // Connect Delete
          QObject::connect(btnDelete, &QPushButton::clicked, [table, btnDelete, orderId]() {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(
                    table->window(), "Delete Order",
                    "Are you sure you want to delete this order?",
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                  Commande c;
                  if (c.supprimer(orderId)) {
                      QMessageBox::information(table->window(), "Deleted", "Order deleted successfully.");
                      // We ideally should refresh the whole table here.
                      QPoint btnPos = btnDelete->mapTo(table->viewport(), btnDelete->rect().center());
                      int row = table->rowAt(btnPos.y());
                      if (row >= 0) table->removeRow(row);
                  } else {
                      QMessageBox::critical(table->window(), "Error", "Failed to delete order.");
                  }
                }
              });

          rowIdx++;
        }
        delete model;
      };

      // Connect
      QObject::connect(searchEdit, &QLineEdit::textChanged, updateTable);
      QObject::connect(sortCombo, &QComboBox::currentTextChanged, updateTable);
      QObject::connect(btnRefresh, &QPushButton::clicked, updateTable);

      // Init
      updateTable();

      cLayout->addWidget(historyContainer);
    } else {
      // Client Statistics Chart
      GenericBarChart *chart =
          new GenericBarChart("New Order Acquisition (Q2 vs Q3)");
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

static QWidget *createFinancePage(QStackedWidget *&outNestedStack) {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(40, 40, 40, 40);
  layout->setSpacing(25);
  layout->addWidget(new QLabel("Financial Management"));

  QWidget *actionBar = new QWidget();
  QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
  actionLayout->setSpacing(12);

  outNestedStack = new QStackedWidget();
  QStringList tabNames = {"New Invoice", "Transaction Hub", "Expense Tracking",
                          "Analytics"};
  QList<QPushButton *> tabButtons;

  // Shared table pointer so "New Invoice" can refresh "Transaction Hub"
  QTableWidget *transTable = nullptr;

  for (const auto &name : tabNames) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(getTabButtonStyle());
    actionLayout->addWidget(btn);
    tabButtons.append(btn);

    QWidget *content = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout(content);

    if (name == "New Invoice") {
      // ========== AJOUTER (CREATE) ==========

      // Solid white card container — no transparency that bleeds dark bg
      QWidget *formContainer = new QWidget();
      formContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      formContainer->setStyleSheet(
          "QWidget {"
          "  background-color: #ffffff;"
          "  border-radius: 12px;"
          "}");

      QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
      outerLayout->setContentsMargins(40, 35, 40, 35);
      outerLayout->setSpacing(0);

      // Title
      QLabel *titleLabel = new QLabel("New Transaction");
      titleLabel->setStyleSheet(
          "font-size: 22px; font-weight: 700; color: #1a1a1a; margin-bottom: 18px;");
      outerLayout->addWidget(titleLabel);

      // Field style — explicitly covers QLineEdit, QDateEdit AND QComboBox
      QString fieldStyle =
          "QLineEdit, QDateEdit, QComboBox {"
          "  background-color: #f8f9fb;"
          "  border: 1.5px solid #d0d5dd;"
          "  border-radius: 8px;"
          "  padding: 10px 14px;"
          "  font-size: 14px;"
          "  color: #1a1a1a;"
          "  min-height: 42px;"
          "}"
          "QLineEdit:focus, QDateEdit:focus, QComboBox:focus {"
          "  border: 2px solid #3DDC84;"
          "  background-color: #ffffff;"
          "}"
          "QComboBox::drop-down { border: none; width: 28px; }"
          "QComboBox QAbstractItemView {"
          "  background-color: #ffffff; color: #1a1a1a;"
          "  selection-background-color: #e8fdf2;"
          "  border: 1px solid #d0d5dd;"
          "  outline: none;"
          "}"
          "QDateEdit::drop-down { border: none; width: 28px; }";

      QString lblStyle =
          "font-size: 13px; font-weight: 600; color: #374151;"
          "margin-top: 14px; margin-bottom: 5px;";

      // Helper to add a label + widget pair
      auto addField = [&](const QString &label, QWidget *w) {
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet(lblStyle);
        w->setStyleSheet(fieldStyle);
        outerLayout->addWidget(lbl);
        outerLayout->addWidget(w);
      };

      // --- Fields ---
      QLineEdit *inputMontant = new QLineEdit();
      inputMontant->setPlaceholderText("e.g. 1500.00");
      addField("Amount (TND):", inputMontant);

      QDateEdit *inputDate = new QDateEdit(QDate::currentDate());
      inputDate->setCalendarPopup(true);
      inputDate->setDisplayFormat("yyyy-MM-dd");
      addField("Transaction Date:", inputDate);

      QComboBox *inputType = new QComboBox();
      inputType->addItems({"Revenue", "Expense", "Refund", "Transfer"});
      addField("Transaction Type:", inputType);

      QComboBox *inputMode = new QComboBox();
      inputMode->addItems({"Cash", "Card", "Bank Transfer", "Check"});
      addField("Payment Mode:", inputMode);

      QLineEdit *inputDesc = new QLineEdit();
      inputDesc->setPlaceholderText("Enter description...");
      addField("Description:", inputDesc);

      QLineEdit *inputCommande = new QLineEdit();
      inputCommande->setPlaceholderText("0");
      addField("Order ID:", inputCommande);

      // --- Submit Button ---
      outerLayout->addSpacing(22);
      QPushButton *btnSubmit = new QPushButton("Submit Transaction");
      btnSubmit->setStyleSheet(
          "QPushButton {"
          "  background-color: #3DDC84;"
          "  color: #ffffff;"
          "  border: none;"
          "  border-radius: 8px;"
          "  font-size: 15px;"
          "  font-weight: 700;"
          "  min-height: 48px;"
          "  padding: 0px 28px;"
          "}"
          "QPushButton:hover { background-color: #34c772; }"
          "QPushButton:pressed { background-color: #2aad60; }");
      btnSubmit->setCursor(Qt::PointingHandCursor);
      outerLayout->addWidget(btnSubmit);
      outerLayout->addStretch();

      cLayout->addWidget(formContainer);

      // --- Connect Submit ---
      QObject::connect(
          btnSubmit, &QPushButton::clicked,
          [inputMontant, inputDate, inputType, inputMode, inputDesc,
           inputCommande, &transTable]() {
            if (inputMontant->text().trimmed().isEmpty()) {
              QMessageBox::warning(nullptr, "Validation Error", "Amount is required.");
              return;
            }
            bool ok = false;
            double montant = inputMontant->text().toDouble(&ok);
            if (!ok) {
              QMessageBox::warning(nullptr, "Validation Error",
                                   "Amount must be a valid number.");
              return;
            }
            Transaction t;
            t.setMontant(montant);
            t.setDateTransaction(inputDate->date());
            t.setTypeTransaction(inputType->currentText());
            t.setModePaiement(inputMode->currentText());
            t.setDescription(inputDesc->text().trimmed());
            t.setIdCommande(inputCommande->text().toInt());

            if (t.ajouter()) {
              QMessageBox::information(nullptr, "Success",
                                       "Transaction added successfully!");
              inputMontant->clear();
              inputDate->setDate(QDate::currentDate());
              inputType->setCurrentIndex(0);
              inputMode->setCurrentIndex(0);
              inputDesc->clear();
              inputCommande->clear();
            } else {
              QMessageBox::critical(
                  nullptr, "Error",
                  "Failed to add transaction.\n\nDB Error: " + t.getLastError());
            }
          });

    } else if (name == "Transaction Hub") {
      // ========== AFFICHER (READ) + MODIFIER (UPDATE) + SUPPRIMER (DELETE) ==========
      QWidget *transContainer = new QWidget();
      QVBoxLayout *transLayout = new QVBoxLayout(transContainer);
      transLayout->setContentsMargins(0, 0, 0, 0);
      transLayout->setSpacing(10);

      // 1. Control Bar
      QWidget *controlBar = new QWidget();
      QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
      controlLayout->setContentsMargins(0, 0, 0, 0);

      QLineEdit *searchEdit = new QLineEdit();
      searchEdit->setPlaceholderText("Search description / type...");
      searchEdit->setStyleSheet(getInputStyle());
      searchEdit->setFixedWidth(200);

      QLabel *lblSort = new QLabel("Sort by:");
      lblSort->setStyleSheet(getLabelStyle());

      QComboBox *sortType = new QComboBox();
      sortType->addItems({"Amt High-Low", "Amt Low-High"});
      sortType->setStyleSheet(getInputStyle());
      sortType->setFixedWidth(130);

      QPushButton *btnRefresh = new QPushButton("Refresh");
      btnRefresh->setStyleSheet(getButtonStyle());
      btnRefresh->setCursor(Qt::PointingHandCursor);
      btnRefresh->setFixedWidth(100);

      QPushButton *btnPrint = new QPushButton("PRINT PDF");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedWidth(120);

      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(15);
      controlLayout->addWidget(lblSort);
      controlLayout->addWidget(sortType);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnRefresh);
      controlLayout->addStretch();
      controlLayout->addWidget(btnPrint);

      transLayout->addWidget(controlBar);

      // 2. Table
      transTable = new QTableWidget();
      QStringList headers = {"ID", "Amount", "Date", "Type", "Payment Mode",
                             "Description", "Order ID", "Actions"};
      transTable->setColumnCount(headers.size());
      transTable->setHorizontalHeaderLabels(headers);
      transTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      transTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      transTable->horizontalHeader()->setSectionResizeMode(headers.size() - 1, QHeaderView::Fixed);
      transTable->setColumnWidth(headers.size() - 1, 220);
      transTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      transTable->verticalHeader()->setVisible(false);
      transTable->setColumnHidden(0, true); // Hide ID column
      transTable->verticalHeader()->setDefaultSectionSize(60);
      transTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      transTable->setSelectionBehavior(QAbstractItemView::SelectRows);
      transTable->setAlternatingRowColors(true);
      transTable->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: "
          "#ffffff; gridline-color: transparent; border-radius: 8px; "
          "alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; "
          "border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; "
          "color: #666; text-transform: uppercase; font-size: 12px; }"
          "QTableWidget::item { padding: 12px; border-bottom: 1px solid "
          "#f5f5f5; color: #333; }"
          "QTableWidget::item:selected { background-color: #e6f9ef; color: "
          "#1a1a1a; }");

      transLayout->addWidget(transTable);

      // 3. Lambda to load/refresh data from the database
      auto refreshTable = [transTable, searchEdit, sortType]() {
        Transaction tr;
        QSqlQueryModel *model = tr.afficher();

        // Read all into a local list for filtering/sorting
        struct Row {
          int id; double montant; QString date; QString type;
          QString mode; QString desc; int commande;
        };
        QList<Row> allRows;
        for (int i = 0; i < model->rowCount(); ++i) {
          Row r;
          r.id = model->data(model->index(i, 0)).toInt();
          r.montant = model->data(model->index(i, 1)).toDouble();
          r.date = model->data(model->index(i, 2)).toString();
          r.type = model->data(model->index(i, 3)).toString();
          r.mode = model->data(model->index(i, 4)).toString();
          r.desc = model->data(model->index(i, 5)).toString();
          r.commande = model->data(model->index(i, 6)).toInt();
          allRows.append(r);
        }
        delete model;

        // Filter
        QString query = searchEdit->text().toLower();
        QList<Row> filtered;
        for (const auto &r : allRows) {
          bool match = r.desc.toLower().contains(query) ||
                       r.type.toLower().contains(query) ||
                       r.mode.toLower().contains(query);
          if (match) filtered.append(r);
        }

        // Sort
        QString sort = sortType->currentText();
        std::sort(filtered.begin(), filtered.end(),
                  [sort](const Row &a, const Row &b) {
                    if (sort == "Amt High-Low") return a.montant > b.montant;
                    else return a.montant < b.montant;
                  });

        // Populate table
        transTable->setRowCount(0);
        transTable->setRowCount(filtered.size());
        for (int i = 0; i < filtered.size(); ++i) {
          const auto &r = filtered[i];
          transTable->setItem(i, 0, new QTableWidgetItem(QString::number(r.id)));
          transTable->setItem(i, 1, new QTableWidgetItem(QString::number(r.montant, 'f', 2)));
          transTable->setItem(i, 2, new QTableWidgetItem(r.date));
          transTable->setItem(i, 3, new QTableWidgetItem(r.type));
          transTable->setItem(i, 4, new QTableWidgetItem(r.mode));
          transTable->setItem(i, 5, new QTableWidgetItem(r.desc));
          transTable->setItem(i, 6, new QTableWidgetItem(QString::number(r.commande)));

          // --- Action Buttons (Edit + Delete) ---
          QWidget *actionWidget = new QWidget();
          QHBoxLayout *actionBtnLayout = new QHBoxLayout(actionWidget);
          actionBtnLayout->setContentsMargins(10, 0, 10, 0);
          actionBtnLayout->setSpacing(15);
          actionBtnLayout->setAlignment(Qt::AlignCenter);

          QPushButton *btnModify = new QPushButton("Edit");
          btnModify->setCursor(Qt::PointingHandCursor);
          btnModify->setMinimumWidth(80);
          btnModify->setFixedHeight(28);
          btnModify->setStyleSheet(
              "QPushButton {"
              "background-color: #ffffff;"
              "color: #333333;"
              "border: 1px solid #cccccc;"
              "border-radius: 6px;"
              "padding: 0px 8px;"
              "font-size: 13px;"
              "font-weight: 600;"
              "} "
              "QPushButton:hover { border-color: #aaaaaa; color: #000000; "
              "background-color: #f6f6f6; }"
              "QPushButton:pressed { background-color: #e6e6e6; }");

          QPushButton *btnDelete = new QPushButton("Remove");
          btnDelete->setCursor(Qt::PointingHandCursor);
          btnDelete->setMinimumWidth(80);
          btnDelete->setFixedHeight(28);
          btnDelete->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#d32f2f; color: #d32f2f; border-radius: 6px; padding: 0px 8px; "
              "font-weight: 600; font-size: 13px; } QPushButton:hover { "
              "background-color: #ffebee; border-color: #b71c1c; color: "
              "#b71c1c; }");

          actionBtnLayout->addWidget(btnModify);
          actionBtnLayout->addWidget(btnDelete);
          transTable->setCellWidget(i, 7, actionWidget);

          // --- Connect Edit ---
          int rowId = r.id;
          QObject::connect(
              btnModify, &QPushButton::clicked,
              [transTable, btnModify, rowId, searchEdit, sortType]() {
                // Find current row dynamically
                QPoint btnPos = btnModify->mapTo(transTable->viewport(),
                                                  btnModify->rect().center());
                int row = transTable->rowAt(btnPos.y());
                if (row < 0) return;

                QDialog dlg(transTable->window());
                dlg.setWindowTitle("Edit Transaction");
                dlg.setModal(true);
                dlg.setMinimumWidth(450);
                dlg.setStyleSheet("QDialog { background-color: #ffffff; "
                                  "border-radius: 12px; }");

                QVBoxLayout *mainV = new QVBoxLayout(&dlg);
                mainV->setContentsMargins(30, 30, 30, 30);
                mainV->setSpacing(20);

                QLabel *title = new QLabel("Update Transaction");
                title->setStyleSheet(
                    "font-size: 20px; font-weight: 700; color: #1a1a1a;");
                mainV->addWidget(title);

                QFormLayout *form = new QFormLayout();
                form->setSpacing(15);
                form->setLabelAlignment(Qt::AlignLeft);

                auto styleField = [&](QWidget *w) {
                  w->setStyleSheet(
                      "QLineEdit, QDateEdit, QComboBox { background-color: #f9fafb; border: 1px "
                      "solid #eaeaea; border-radius: 8px; padding: 10px; "
                      "font-size: 14px; color: #333; } "
                      "QLineEdit:focus, QDateEdit:focus, QComboBox:focus { "
                      "border-color: #3DDC84; background-color: #ffffff; }");
                  w->setFixedHeight(40);
                };

                QLineEdit *editMontant = new QLineEdit(transTable->item(row, 1)->text());
                styleField(editMontant);

                QDateEdit *editDate = new QDateEdit(
                    QDate::fromString(transTable->item(row, 2)->text().left(10), "yyyy-MM-dd"));
                editDate->setCalendarPopup(true);
                editDate->setDisplayFormat("yyyy-MM-dd");
                styleField(editDate);

                QComboBox *editType = new QComboBox();
                editType->addItems({"Revenue", "Expense", "Refund", "Transfer"});
                editType->setCurrentText(transTable->item(row, 3)->text());
                styleField(editType);

                QComboBox *editMode = new QComboBox();
                editMode->addItems({"Cash", "Card", "Bank Transfer", "Check"});
                editMode->setCurrentText(transTable->item(row, 4)->text());
                styleField(editMode);

                QLineEdit *editDesc = new QLineEdit(transTable->item(row, 5)->text());
                styleField(editDesc);

                QLineEdit *editCommande = new QLineEdit(transTable->item(row, 6)->text());
                styleField(editCommande);

                auto addRow = [&](const QString &label, QWidget *w) {
                  QLabel *l = new QLabel(label);
                  l->setStyleSheet(
                      "font-weight: 600; color: #444; font-size: 13px;");
                  form->addRow(l, w);
                };

                addRow("Amount:", editMontant);
                addRow("Date:", editDate);
                addRow("Type:", editType);
                addRow("Payment Mode:", editMode);
                addRow("Description:", editDesc);
                addRow("Order ID:", editCommande);

                mainV->addLayout(form);
                mainV->addSpacing(10);

                QDialogButtonBox *bbox = new QDialogButtonBox(
                    QDialogButtonBox::Save | QDialogButtonBox::Cancel);
                bbox->setStyleSheet(
                    "QPushButton { padding: 10px 24px; border-radius: 8px; "
                    "font-weight: 700; font-size: 14px; min-width: 100px; }"
                    "QPushButton[text='Save'] { background-color: #3DDC84; "
                    "color: white; border: none; }"
                    "QPushButton[text='Save']:hover { background-color: "
                    "#34c772; }"
                    "QPushButton[text='Cancel'] { background-color: #ffffff; "
                    "color: #666; border: 1px solid #ddd; }"
                    "QPushButton[text='Cancel']:hover { background-color: "
                    "#f9fafb; }");
                mainV->addWidget(bbox);

                QObject::connect(bbox, &QDialogButtonBox::accepted, &dlg,
                                 &QDialog::accept);
                QObject::connect(bbox, &QDialogButtonBox::rejected, &dlg,
                                 &QDialog::reject);

                if (dlg.exec() == QDialog::Accepted) {
                  Transaction t;
                  t.setIdTransaction(rowId);
                  t.setMontant(editMontant->text().toDouble());
                  t.setDateTransaction(editDate->date());
                  t.setTypeTransaction(editType->currentText());
                  t.setModePaiement(editMode->currentText());
                  t.setDescription(editDesc->text().trimmed());
                  t.setIdCommande(editCommande->text().toInt());

                  if (t.modifier()) {
                    QMessageBox::information(
                        transTable->window(), "Success",
                        "Transaction updated successfully.");
                    // Trigger refresh via searchEdit signal
                    emit searchEdit->textChanged(searchEdit->text());
                  } else {
                    QMessageBox::critical(
                        transTable->window(), "Error",
                        "Failed to update transaction.\n\nDB Error: " +
                            t.getLastError());
                  }
                }
              });

          // --- Connect Delete ---
          QObject::connect(
              btnDelete, &QPushButton::clicked,
              [transTable, btnDelete, rowId, searchEdit]() {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(
                    transTable->window(), "Delete Transaction",
                    "Are you sure you want to delete this transaction?",
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                  Transaction t;
                  if (t.supprimer(rowId)) {
                    QMessageBox::information(transTable->window(), "Deleted",
                                             "Transaction deleted successfully.");
                    // Trigger refresh
                    emit searchEdit->textChanged(searchEdit->text());
                  } else {
                    QMessageBox::critical(
                        transTable->window(), "Error",
                        "Failed to delete transaction.\n\nDB Error: " +
                            t.getLastError());
                  }
                }
              });
        }
      };

      // Initial load
      refreshTable();

      // Connections - Auto Update on search/sort
      QObject::connect(searchEdit, &QLineEdit::textChanged, refreshTable);
      QObject::connect(sortType, &QComboBox::currentTextChanged, refreshTable);
      QObject::connect(btnRefresh, &QPushButton::clicked, refreshTable);

      // Print PDF
      QObject::connect(btnPrint, &QPushButton::clicked, [transTable]() {
        QString fileName = QFileDialog::getSaveFileName(
            transTable->window(), "Export PDF", "transactions_report.pdf",
            "PDF Files (*.pdf)");
        if (fileName.isEmpty()) return;

        QPrinter printer(QPrinter::PrinterResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        printer.setPageSize(QPageSize(QPageSize::A4));

        QString html = "<h2 style='color:#1a1a1a;'>Transaction Report</h2>"
                       "<table style='width:100%; border-collapse:collapse;'>"
                       "<tr style='background:#f0f0f0;'>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Amount</th>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Date</th>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Type</th>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Payment</th>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Description</th>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Order ID</th>"
                       "</tr>";

        for (int r = 0; r < transTable->rowCount(); ++r) {
          html += "<tr>";
          for (int c = 1; c <= 6; ++c) { // Skip hidden ID col
            QString val = transTable->item(r, c) ? transTable->item(r, c)->text() : "";
            html += "<td style='padding:8px; border:1px solid #ddd;'>" + val + "</td>";
          }
          html += "</tr>";
        }
        html += "</table>";

        QTextDocument doc;
        doc.setHtml(html);
        doc.setPageSize(printer.pageRect(QPrinter::Point).size());
        doc.print(&printer);

        QMessageBox::information(transTable->window(), "Success",
                                 "PDF Exported Successfully!");
      });

      cLayout->addWidget(transContainer);

    } else if (name == "Expense Tracking") {
      // ========== Filtered view — Expenses only ==========
      QPushButton *btnPrint = new QPushButton("PRINT PDF");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedWidth(150);
      cLayout->addWidget(btnPrint, 0, Qt::AlignRight);

      QTableWidget *expTable = new QTableWidget();
      QStringList headers = {"Date", "Type", "Description", "Amount", "Payment Mode"};
      expTable->setColumnCount(headers.size());
      expTable->setHorizontalHeaderLabels(headers);
      expTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      expTable->verticalHeader()->setVisible(false);
      expTable->setAlternatingRowColors(true);
      expTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      expTable->setSelectionBehavior(QAbstractItemView::SelectRows);
      expTable->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: "
          "#ffffff; gridline-color: transparent; border-radius: 8px; "
          "alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; "
          "border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; "
          "color: #666; }");

      // Load expense data from DB
      auto loadExpenses = [expTable]() {
        QSqlQuery query;
        query.prepare("SELECT DATE_TRANSACTION, TYPE_TRANSACTION, DESCRIPTION, "
                      "MONTANT, MODE_PAIEMENT FROM FINANCE "
                      "WHERE UPPER(TYPE_TRANSACTION) = 'EXPENSE' "
                      "ORDER BY DATE_TRANSACTION DESC");
        query.exec();

        expTable->setRowCount(0);
        int row = 0;
        while (query.next()) {
          expTable->insertRow(row);
          expTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
          expTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
          expTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
          expTable->setItem(row, 3, new QTableWidgetItem(
              QString::number(query.value(3).toDouble(), 'f', 2)));
          expTable->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
          row++;
        }
      };

      loadExpenses();
      cLayout->addWidget(expTable);

      // Print PDF
      QObject::connect(btnPrint, &QPushButton::clicked, [expTable]() {
        QString fileName = QFileDialog::getSaveFileName(
            expTable->window(), "Export PDF", "expense_report.pdf",
            "PDF Files (*.pdf)");
        if (fileName.isEmpty()) return;

        QPrinter printer(QPrinter::PrinterResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        printer.setPageSize(QPageSize(QPageSize::A4));

        QString html = "<h2>Expense Report</h2>"
                       "<table style='width:100%; border-collapse:collapse;'>"
                       "<tr style='background:#f0f0f0;'>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Date</th>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Type</th>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Description</th>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Amount</th>"
                       "<th style='padding:8px; border:1px solid #ddd;'>Payment</th></tr>";
        for (int r = 0; r < expTable->rowCount(); ++r) {
          html += "<tr>";
          for (int c = 0; c < expTable->columnCount(); ++c) {
            QString val = expTable->item(r, c) ? expTable->item(r, c)->text() : "";
            html += "<td style='padding:8px; border:1px solid #ddd;'>" + val + "</td>";
          }
          html += "</tr>";
        }
        html += "</table>";
        QTextDocument doc;
        doc.setHtml(html);
        doc.setPageSize(printer.pageRect(QPrinter::Point).size());
        doc.print(&printer);
        QMessageBox::information(expTable->window(), "Success",
                                 "PDF Exported Successfully!");
      });

    } else if (name == "Analytics") {
      // ========== Analytics — Aggregated from DB ==========
      QPushButton *btnPrint = new QPushButton("PRINT REPORT");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedWidth(150);
      cLayout->addWidget(btnPrint, 0, Qt::AlignRight);

      GenericBarChart *chart = new GenericBarChart("Revenue vs Expenses");

      // Query aggregated data from DB
      QSqlQuery q;
      double totalRevenue = 0, totalExpense = 0;

      q.prepare("SELECT NVL(SUM(MONTANT),0) FROM FINANCE WHERE UPPER(TYPE_TRANSACTION) = 'REVENUE'");
      if (q.exec() && q.next()) totalRevenue = q.value(0).toDouble();

      q.prepare("SELECT NVL(SUM(MONTANT),0) FROM FINANCE WHERE UPPER(TYPE_TRANSACTION) = 'EXPENSE'");
      if (q.exec() && q.next()) totalExpense = q.value(0).toDouble();

      chart->addBar("Revenue", totalRevenue, QColor(61, 220, 132));
      chart->addBar("Expenses", totalExpense, QColor(231, 76, 60));

      QWidget *chartContainer = new QWidget();
      chartContainer->setStyleSheet(getCardStyle());
      QVBoxLayout *containerLayout = new QVBoxLayout(chartContainer);
      containerLayout->setContentsMargins(30, 30, 30, 30);
      containerLayout->addWidget(chart);

      // Summary cards
      QWidget *summaryBar = new QWidget();
      QHBoxLayout *summaryLayout = new QHBoxLayout(summaryBar);
      summaryLayout->setSpacing(20);

      double net = totalRevenue - totalExpense;
      QString netColor = net >= 0 ? "#3DDC84" : "#d32f2f";

      summaryLayout->addWidget(
          createStatCard("Total Revenue",
                         QString::number(totalRevenue, 'f', 2),
                         "From DB", "#3DDC84"));
      summaryLayout->addWidget(
          createStatCard("Total Expenses",
                         QString::number(totalExpense, 'f', 2),
                         "From DB", "#e74c3c"));
      summaryLayout->addWidget(
          createStatCard("Net Profit",
                         QString::number(net, 'f', 2),
                         net >= 0 ? "Positive" : "Negative", netColor));

      cLayout->addWidget(summaryBar);
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

static QWidget *createInventoryPage(QStackedWidget *&outNestedStack) {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(40, 40, 40, 40);
  layout->setSpacing(25);
  layout->addWidget(new QLabel("Stock Management"));

  QWidget *actionBar = new QWidget();
  QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
  actionLayout->setSpacing(12);

  outNestedStack = new QStackedWidget();
  QStringList tabNames = {"Add Stock", "Supplier Hub", "Stock Reports",
                          "Analytics"};
  QList<QPushButton *> tabButtons;

  // We need a shared pointer to the table so "Add Stock" can refresh "Stock
  // Reports"
  QTableWidget *stockTable = nullptr;

  for (const auto &name : tabNames) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(getTabButtonStyle());
    actionLayout->addWidget(btn);
    tabButtons.append(btn);

    QWidget *content = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout(content);

    if (name == "Add Stock") {
      // ========== AJOUTER (CREATE) ==========
      QWidget *formContainer = new QWidget();
      formContainer->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Expanding);
      formContainer->setStyleSheet(
          ".QWidget { background-color: #ffffff; border-radius: 10px; border: "
          "1px solid #eee; }");

      QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
      outerLayout->setContentsMargins(30, 30, 30, 30);

      QWidget *formContent = new QWidget();
      formContent->setAttribute(Qt::WA_TranslucentBackground);
      formContent->setStyleSheet("background: transparent;");
      QVBoxLayout *formLayout = new QVBoxLayout(formContent);
      formLayout->setContentsMargins(0, 0, 0, 0);
      formLayout->setSpacing(15);

      QLabel *titleLabel = new QLabel("Add New Stock Item");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: "
                                "#1a1a1a; margin-bottom: 25px; border: none;");
      formLayout->addWidget(titleLabel);

      QString labelStyle = getLabelStyle();
      QString inputStyle = getInputStyle();

      // NOM
      QLabel *lblNom = new QLabel("Item Name:");
      lblNom->setStyleSheet(labelStyle);
      QLineEdit *inputNom = new QLineEdit();
      inputNom->setStyleSheet(inputStyle);
      inputNom->setPlaceholderText("Extra Virgin 1L");
      formLayout->addWidget(lblNom);
      formLayout->addWidget(inputNom);

      // QUANTITE
      QLabel *lblQty = new QLabel("Quantity:");
      lblQty->setStyleSheet(labelStyle);
      QLineEdit *inputQty = new QLineEdit();
      inputQty->setStyleSheet(inputStyle);
      inputQty->setPlaceholderText("0");
      formLayout->addWidget(lblQty);
      formLayout->addWidget(inputQty);

      // UNITE
      QLabel *lblUnt = new QLabel("Unit:");
      lblUnt->setStyleSheet(labelStyle);
      QLineEdit *inputUnt = new QLineEdit();
      inputUnt->setStyleSheet(inputStyle);
      inputUnt->setPlaceholderText("Liters / Kg / Units");
      formLayout->addWidget(lblUnt);
      formLayout->addWidget(inputUnt);

      // SEUIL MINIMAL
      QLabel *lblSeuil = new QLabel("Min Threshold:");
      lblSeuil->setStyleSheet(labelStyle);
      QLineEdit *inputSeuil = new QLineEdit();
      inputSeuil->setStyleSheet(inputStyle);
      inputSeuil->setPlaceholderText("0");
      formLayout->addWidget(lblSeuil);
      formLayout->addWidget(inputSeuil);

      // Submit Button
      formLayout->addSpacing(20);
      QPushButton *btnSubmit = new QPushButton("Add Item");
      btnSubmit->setStyleSheet(getButtonStyle());
      btnSubmit->setCursor(Qt::PointingHandCursor);
      btnSubmit->setFixedHeight(45);
      formLayout->addWidget(btnSubmit);

      // Connect Add Button -> Article::ajouter()
      QObject::connect(
          btnSubmit, &QPushButton::clicked, [inputNom, inputQty, inputUnt, inputSeuil]() {
            // Input validation
            if (inputNom->text().trimmed().isEmpty()) {
              QMessageBox::warning(nullptr, "Validation Error",
                                   "Item Name is required.");
              return;
            }

            Article a;
            a.setNom(inputNom->text().trimmed());
            a.setQuantite(inputQty->text().toInt());
            a.setUnite(inputUnt->text().trimmed());
            a.setSeuilMinimal(inputSeuil->text().toInt());

            if (a.ajouter()) {
              QMessageBox::information(nullptr, "Success",
                                       "Item added successfully!");
              // Clear the form
              inputNom->clear();
              inputQty->clear();
              inputUnt->clear();
              inputSeuil->clear();
            } else {
              QMessageBox::critical(
                  nullptr, "Error",
                  "Failed to add item to the database.\n\nDB Error: " +
                      a.getLastError());
            }
          });

      formLayout->addStretch();
      outerLayout->addWidget(formContent);
      cLayout->addWidget(formContainer);

    } else if (name == "Supplier Hub") {
      cLayout->addWidget(createStyledTable(
          "Supplier Directory", {"Supplier Name", "Contact", "Phone", "Rating"},
          {{"Mediterranean Olives", "Maria Rossi", "(555) 123-4567", "5/5"},
           {"Glass Bottle Co.", "Bob Smith", "(555) 987-6543", "4/5"}},
          true));

    } else if (name == "Stock Reports") {
      // ========== AFFICHER (READ) + MODIFIER (UPDATE) + SUPPRIMER (DELETE)
      // ==========
      QWidget *reportContainer = new QWidget();
      QVBoxLayout *reportLayout = new QVBoxLayout(reportContainer);
      reportLayout->setContentsMargins(0, 0, 0, 0);
      reportLayout->setSpacing(10);

      // 1. Control Bar: Search + Refresh
      QWidget *controlBar = new QWidget();
      QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
      controlLayout->setContentsMargins(0, 0, 0, 0);

      QLineEdit *searchEdit = new QLineEdit();
      searchEdit->setPlaceholderText("Search items...");
      searchEdit->setStyleSheet(getInputStyle());
      searchEdit->setFixedWidth(150);

      QLabel *lblSort = new QLabel("Sort by:");
      lblSort->setStyleSheet(getLabelStyle());
      QComboBox *sortCombo = new QComboBox();
      sortCombo->addItems(
          {"None", "Quantity (High-Low)", "Quantity (Low-High)"});
      sortCombo->setCurrentText("None");
      sortCombo->setStyleSheet(getInputStyle());
      sortCombo->setFixedWidth(160);

      QPushButton *btnRefresh = new QPushButton("Refresh");
      btnRefresh->setStyleSheet(getButtonStyle());
      btnRefresh->setCursor(Qt::PointingHandCursor);
      btnRefresh->setFixedWidth(100);

      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(lblSort);
      controlLayout->addWidget(sortCombo);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnRefresh);
      controlLayout->addStretch();
      reportLayout->addWidget(controlBar);

      // 2. Table
      stockTable = new QTableWidget();
      QStringList headers = {"ID", "Item Name", "Current Qty", "Unit", "Min Threshold",
                             "Actions"};
      stockTable->setColumnCount(headers.size());
      stockTable->setHorizontalHeaderLabels(headers);
      stockTable->horizontalHeader()->setSectionResizeMode(
          QHeaderView::Stretch);
      stockTable->horizontalHeader()->setSectionResizeMode(
          0, QHeaderView::ResizeToContents); // ID column
      stockTable->horizontalHeader()->setSectionResizeMode(headers.size() - 1,
                                                           QHeaderView::Fixed);
      stockTable->setColumnWidth(headers.size() - 1,
                                 220); // Give enough space for buttons
      stockTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft |
                                                          Qt::AlignVCenter);
      stockTable->verticalHeader()->setVisible(false);
      stockTable->setColumnHidden(0, true); // Hide ID column
      stockTable->verticalHeader()->setDefaultSectionSize(60);
      stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
      stockTable->setAlternatingRowColors(true);
      stockTable->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: "
          "#ffffff; gridline-color: transparent; border-radius: 8px; "
          "alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; "
          "border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; "
          "color: #666; text-transform: uppercase; font-size: 12px; }"
          "QTableWidget::item { padding: 12px; border-bottom: 1px solid "
          "#f5f5f5; color: #333; }"
          "QTableWidget::item:selected { background-color: #e6f9ef; color: "
          "#1a1a1a; }");

      reportLayout->addWidget(stockTable);

      // 3. Lambda to load/refresh data from the database
      auto refreshTable = [stockTable]() {
        Article a;
        QSqlQueryModel *model = a.afficher();

        stockTable->setRowCount(0); // Clear existing rows

        int rowCount = model->rowCount();
        stockTable->setRowCount(rowCount);

        for (int i = 0; i < rowCount; ++i) {
          // Columns: ID(0), NOM(1), QUANTITE(2), SEUIL(3)
          QString id = model->data(model->index(i, 0)).toString();
          QString nom = model->data(model->index(i, 1)).toString();
          QString qty = model->data(model->index(i, 2)).toString();
          QString unt = model->data(model->index(i, 3)).toString();
          QString seuil = model->data(model->index(i, 4)).toString();

          stockTable->setItem(i, 0, new QTableWidgetItem(id));
          stockTable->setItem(i, 1, new QTableWidgetItem(nom));
          stockTable->setItem(i, 2, new QTableWidgetItem(qty));
          stockTable->setItem(i, 3, new QTableWidgetItem(unt));
          stockTable->setItem(i, 4, new QTableWidgetItem(seuil));

          // --- Action Buttons (Modify + Delete) ---
          QWidget *actionWidget = new QWidget();
          QHBoxLayout *actionBtnLayout = new QHBoxLayout(actionWidget);
          actionBtnLayout->setContentsMargins(10, 0, 10, 0);
          actionBtnLayout->setSpacing(15);
          actionBtnLayout->setAlignment(Qt::AlignCenter);

          // Remove the first stretch to keep buttons more predictable if the
          // column is wide actionBtnLayout->addStretch();

          QPushButton *btnModify = new QPushButton("Edit");
          btnModify->setCursor(Qt::PointingHandCursor);
          btnModify->setMinimumWidth(80);
          btnModify->setFixedHeight(28);
          btnModify->setStyleSheet(
              "QPushButton {"
              "background-color: #ffffff;"
              "color: #333333;"
              "border: 1px solid #cccccc;"
              "border-radius: 6px;"
              "padding: 0px 8px;"
              "font-size: 13px;"
              "font-weight: 600;"
              "} "
              "QPushButton:hover { border-color: #aaaaaa; color: #000000; "
              "background-color: #f6f6f6; }"
              "QPushButton:pressed { background-color: #e6e6e6; }");

          QPushButton *btnDelete = new QPushButton("Remove");
          btnDelete->setCursor(Qt::PointingHandCursor);
          btnDelete->setMinimumWidth(80);
          btnDelete->setFixedHeight(28);
          btnDelete->setStyleSheet(
              "QPushButton {"
              "background-color: #ffffff;"
              "color: #d32f2f;"
              "border: 1px solid #d32f2f;"
              "border-radius: 6px;"
              "padding: 0px 8px;"
              "font-size: 13px;"
              "font-weight: 600;"
              "} "
              "QPushButton:hover { background-color: #ffebee; border-color: "
              "#b71c1c; color: #b71c1c; }"
              "QPushButton:pressed { background-color: #ffcdd2; }");

          actionBtnLayout->addWidget(btnModify);
          actionBtnLayout->addWidget(btnDelete);
          // Keep only the end stretch or no stretch if Aligned Center

          // --- SUPPRIMER (DELETE) ---
          int itemId = id.toInt();
          QObject::connect(
              btnDelete, &QPushButton::clicked,
              [stockTable, itemId, btnDelete]() {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(
                    stockTable->window(), "Confirm Deletion",
                    "Are you sure you want to delete this item?",
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                  Article a;
                  if (a.supprimer(itemId)) {
                    // Find and remove the row
                    QPoint btnPos = btnDelete->mapTo(
                        stockTable->viewport(), btnDelete->rect().center());
                    int row = stockTable->rowAt(btnPos.y());
                    if (row >= 0) {
                      stockTable->removeRow(row);
                    }
                    QMessageBox::information(nullptr, "Success",
                                             "Item deleted successfully!");
                  } else {
                    QMessageBox::critical(
                        nullptr, "Error",
                        "Failed to delete item from database.");
                  }
                }
              });

          // --- MODIFIER (UPDATE) ---
          QObject::connect(
              btnModify, &QPushButton::clicked,
              [stockTable, itemId, nom, qty, unt, seuil]() {
                QDialog *dialog = new QDialog(stockTable->window());
                dialog->setWindowTitle("Modify Item");
                dialog->setMinimumWidth(450);
                dialog->setStyleSheet(
                    "QDialog { background-color: #ffffff; }"
                    "QLabel { font-size: 14px; font-weight: 700; color: #333; }"
                    "QLineEdit { background-color: #fcfcfc; border: 1px solid "
                    "#e0e0e0; border-radius: 8px; padding: 10px 14px; "
                    "font-size: 14px; color: #333; min-height: 35px; }"
                    "QLineEdit:focus { border: 2px solid #3DDC84; "
                    "background-color: #ffffff; }");

                QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
                dialogLayout->setContentsMargins(30, 30, 30, 30);
                dialogLayout->setSpacing(15);

                QLabel *dlgTitle = new QLabel("Modify Item");
                dlgTitle->setStyleSheet("font-size: 22px; font-weight: 800; "
                                        "color: #1a1a1a; margin-bottom: 10px;");
                dialogLayout->addWidget(dlgTitle);

                QLabel *lblNom = new QLabel("Item Name:");
                QLineEdit *editNom = new QLineEdit(nom);
                dialogLayout->addWidget(lblNom);
                dialogLayout->addWidget(editNom);

                QLabel *lblQty = new QLabel("Quantity:");
                QLineEdit *editQty = new QLineEdit(qty);
                dialogLayout->addWidget(lblQty);
                dialogLayout->addWidget(editQty);

                QLabel *lblUnt = new QLabel("Unit:");
                QLineEdit *editUnt = new QLineEdit(unt);
                dialogLayout->addWidget(lblUnt);
                dialogLayout->addWidget(editUnt);

                QLabel *lblSeuil = new QLabel("Min Threshold:");
                QLineEdit *editSeuil = new QLineEdit(seuil);
                dialogLayout->addWidget(lblSeuil);
                dialogLayout->addWidget(editSeuil);

                dialogLayout->addSpacing(20);

                QHBoxLayout *btnLayout = new QHBoxLayout();
                btnLayout->setSpacing(15);

                QPushButton *cancelBtn = new QPushButton("Cancel");
                cancelBtn->setCursor(Qt::PointingHandCursor);
                cancelBtn->setFixedHeight(45);
                cancelBtn->setStyleSheet(
                    "QPushButton { background-color: #ffffff; color: #555; "
                    "border: 1px solid #ddd; border-radius: 8px; padding: 12px "
                    "24px; font-size: 14px; font-weight: 600; } "
                    "QPushButton:hover { border-color: #bbb; color: #333; }");

                QPushButton *saveBtn = new QPushButton("Save Changes");
                saveBtn->setCursor(Qt::PointingHandCursor);
                saveBtn->setFixedHeight(45);
                saveBtn->setStyleSheet(
                    "QPushButton { background: qlineargradient(x1:0, y1:0, "
                    "x2:0, y2:1, stop:0 #3DDC84, stop:1 #2DB66F); color: "
                    "#FFFFFF; border: none; border-radius: 8px; padding: 12px "
                    "24px; font-size: 14px; font-weight: 700; } "
                    "QPushButton:hover { background: qlineargradient(x1:0, "
                    "y1:0, x2:0, y2:1, stop:0 #4EED95, stop:1 #3DDC84); }"
                    "QPushButton:pressed { background: qlineargradient(x1:0, "
                    "y1:0, x2:0, y2:1, stop:0 #2DB66F, stop:1 #228B5A); }");

                btnLayout->addWidget(cancelBtn);
                btnLayout->addWidget(saveBtn);
                dialogLayout->addLayout(btnLayout);

                QObject::connect(cancelBtn, &QPushButton::clicked, dialog,
                                 &QDialog::reject);
                QObject::connect(
                    saveBtn, &QPushButton::clicked,
                    [dialog, editNom, editQty, editUnt, editSeuil, itemId,
                     stockTable]() {
                      Article a;
                      a.setId(itemId);
                      a.setNom(editNom->text().trimmed());
                      a.setQuantite(editQty->text().toInt());
                      a.setUnite(editUnt->text().trimmed());
                      a.setSeuilMinimal(editSeuil->text().toInt());

                      if (a.modifier()) {
                        QMessageBox::information(nullptr, "Success",
                                                 "Item updated successfully!");
                        dialog->accept();
                        for (int r = 0; r < stockTable->rowCount(); ++r) {
                          if (stockTable->item(r, 0) &&
                              stockTable->item(r, 0)->text().toInt() ==
                                  itemId) {
                            stockTable->item(r, 1)->setText(
                                editNom->text().trimmed());
                            stockTable->item(r, 2)->setText(editQty->text());
                            stockTable->item(r, 3)->setText(editUnt->text());
                            stockTable->item(r, 4)->setText(editSeuil->text());
                            break;
                          }
                        }
                      } else {
                        QMessageBox::critical(
                            nullptr, "Error",
                            "Failed to update item in database.");
                      }
                    });

                dialog->exec();
                dialog->deleteLater();
              });

          stockTable->setCellWidget(i, 5, actionWidget);
        }

        delete model;
      };

      // Initial load
      refreshTable();

      // (Template Only) Trigger refresh on sort change - Removed
      // QObject::connect(sortCombo, &QComboBox::currentTextChanged,
      //                  [refreshTable]() { refreshTable(); });

      // Refresh button
      QObject::connect(btnRefresh, &QPushButton::clicked, refreshTable);

      // 4. Search Logic
      auto updateFilter = [stockTable, searchEdit]() {
        QString query = searchEdit->text().toLower();
        int colIndex = 1; // NOM is col 1

        for (int i = 0; i < stockTable->rowCount(); ++i) {
          bool match = false;
          if (stockTable->item(i, colIndex)) {
            match =
                stockTable->item(i, colIndex)->text().toLower().contains(query);
          }
          stockTable->setRowHidden(i, !match);
        }
      };

      QObject::connect(searchEdit, &QLineEdit::textChanged, updateFilter);
      // QObject::connect(searchType, &QComboBox::currentTextChanged,
      // updateFilter); // Removed searchType as Status is removed

      cLayout->addWidget(reportContainer);

    } else {
      // Stock Statistics Chart
      GenericBarChart *chart = new GenericBarChart("Stock Inventory Levels");
      chart->addBar("Empty Bottles 500ml", 50, QColor(231, 76, 60));
      chart->addBar("Labels - 'Gold'", 120, QColor(243, 156, 18));
      chart->addBar("Extra Virgin 1L", 450, QColor(61, 220, 132));
      chart->addBar("Caps - Black", 2000, QColor(61, 220, 132));

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
  QStringList tabNames = {"Add Asset", "Asset Hub", "Service History", "Analytics"};
  QList<QPushButton *> tabButtons;

  // Shared pointers for cross-lambda access
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
      
      QTableWidgetItem *hItem = new QTableWidgetItem();
      hItem->setData(Qt::DisplayRole, model->data(model->index(i, 4)).toInt());
      table->setItem(i, 4, hItem);

      QTableWidgetItem *sItem = new QTableWidgetItem();
      sItem->setData(Qt::DisplayRole, model->data(model->index(i, 5)).toInt());
      table->setItem(i, 5, sItem);

      // --- Actions ---
      QWidget *actionWidget = new QWidget();
      QHBoxLayout *al = new QHBoxLayout(actionWidget);
      al->setContentsMargins(5, 2, 5, 2);
      al->setSpacing(8);

      QPushButton *btnMod = new QPushButton("Edit");
      btnMod->setCursor(Qt::PointingHandCursor);
      btnMod->setMinimumWidth(80);
      btnMod->setFixedHeight(28);
      btnMod->setStyleSheet(
          "QPushButton { background-color: #ffffff; border: 1px solid #cccccc; border-radius: 6px; padding: 0px 8px; font-weight: 600; font-size: 13px; color: #333333; } "
          "QPushButton:hover { border-color: #aaaaaa; color: #000000; background-color: #f6f6f6; }");

      QPushButton *btnDel = new QPushButton("Remove");
      btnDel->setCursor(Qt::PointingHandCursor);
      btnDel->setMinimumWidth(80);
      btnDel->setFixedHeight(28);
      btnDel->setStyleSheet(
          "QPushButton { background-color: #ffffff; border: 1px solid #d32f2f; color: #d32f2f; border-radius: 6px; padding: 0px 8px; font-weight: 600; font-size: 13px; } "
          "QPushButton:hover { background-color: #ffebee; border-color: #b71c1c; color: #b71c1c; }");

      al->addWidget(btnMod);
      al->addWidget(btnDel);
      table->setCellWidget(i, 6, actionWidget);

      // Connect Edit
      QObject::connect(btnMod, &QPushButton::clicked, [table, mid, refreshMachineTable]() {
          // Find row index
          int row = -1;
          for(int r=0; r<table->rowCount(); ++r) {
              if(table->item(r, 0)->text().toInt() == mid) {
                  row = r;
                  break;
              }
          }
          if(row == -1) return;

          QString currentName = table->item(row, 1)->text();
          QString currentType = table->item(row, 2)->text();
          QString currentStatus = table->item(row, 3)->text();
          int currentHours = table->item(row, 4)->data(Qt::DisplayRole).toInt();
          int currentSeuil = table->item(row, 5)->data(Qt::DisplayRole).toInt();

          QDialog dlg(table->window());
          dlg.setWindowTitle("Modify Machine");
          dlg.setMinimumWidth(400);
          dlg.setStyleSheet("QDialog { background-color: #ffffff; border-radius: 12px; }");

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

          QLineEdit *edName = addField("Name:", currentName);
          QLineEdit *edType = addField("Type:", currentType);
          QLineEdit *edStatus = addField("Status:", currentStatus);
          QLineEdit *edHours = addField("Hours:", QString::number(currentHours));
          edHours->setValidator(new QIntValidator(0, 9999999, edHours));
          QLineEdit *edSeuil = addField("Threshold:", QString::number(currentSeuil));
          edSeuil->setValidator(new QIntValidator(0, 9999999, edSeuil));

          QPushButton *btnSave = new QPushButton("Save Changes");
          btnSave->setStyleSheet("QPushButton { background-color: #3DDC84; color: white; border: none; border-radius: 8px; padding: 12px; font-weight: 700; }");
          mainV->addWidget(btnSave);

          QObject::connect(btnSave, &QPushButton::clicked, [=, &dlg]() {
              if (edName->text().trimmed().isEmpty() || edType->text().trimmed().isEmpty() || edHours->text().isEmpty() || edSeuil->text().isEmpty()) {
                  QMessageBox::warning(&dlg, "Erreur de Saisie", "Veuillez remplir tous les champs obligatoires (Nom, Type, Heures, Seuil).");
                  return;
              }
              Machine updateObj(mid, edName->text(), edType->text(), edStatus->text(), edHours->text().toInt(), edSeuil->text().toInt());
              if (updateObj.modifier()) {
                  dlg.accept();
                  (*refreshMachineTable)();
              } else {
                  QMessageBox::critical(&dlg, "Error", "Update failed: " + updateObj.getLastError());
              }
          });
          dlg.exec();
      });

      // Connect Delete
      QObject::connect(btnDel, &QPushButton::clicked, [table, mid, refreshMachineTable]() {
          if (QMessageBox::warning(table->window(), "Confirm Delete", "Are you sure you want to remove this machine?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
              Machine deleteObj;
              if (deleteObj.supprimer(mid)) {
                  (*refreshMachineTable)();
              } else {
                  QMessageBox::critical(table->window(), "Error", "Delete failed: " + deleteObj.getLastError());
              }
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
      formLayout->setSpacing(15);

      QLabel *titleLabel = new QLabel("New Machine");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; margin-bottom: 25px; border: none;");
      titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      titleLabel->setMinimumHeight(40);
      formLayout->addWidget(titleLabel);

      QLineEdit *nameInput = new QLineEdit();
      QLineEdit *typeInput = new QLineEdit();
      QLineEdit *statusInput = new QLineEdit();
      QLineEdit *hoursInput = new QLineEdit("0");
      hoursInput->setValidator(new QIntValidator(0, 9999999, hoursInput));
      QLineEdit *seuilInput = new QLineEdit("100");
      seuilInput->setValidator(new QIntValidator(0, 9999999, seuilInput));

      auto addInput = [&](const QString &txt, QLineEdit *le) {
          QLabel *l = new QLabel(txt);
          l->setStyleSheet(getLabelStyle());
          l->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
          l->setMinimumHeight(30);
          le->setStyleSheet(getInputStyle());
          le->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
          le->setMinimumHeight(40);
          formLayout->addWidget(l);
          formLayout->addWidget(le);
      };

      addInput("Machine Name:", nameInput);
      addInput("Machine Type:", typeInput);
      addInput("Machine Status:", statusInput);
      addInput("Operating Hours:", hoursInput);
      addInput("Maintenance Threshold:", seuilInput);

      formLayout->addSpacing(20);
      QPushButton *btnAdd = new QPushButton("Add Machine");
      btnAdd->setStyleSheet(getButtonStyle());
      btnAdd->setFixedHeight(45);
      formLayout->addWidget(btnAdd);
      formLayout->addStretch();

      cLayout->addWidget(formContainer);

      QObject::connect(btnAdd, &QPushButton::clicked, [=]() {
          if (nameInput->text().trimmed().isEmpty() || typeInput->text().trimmed().isEmpty()) {
              QMessageBox::warning(nullptr, "Erreur de Saisie", "Les champs 'Nom' et 'Type' sont obligatoires.");
              return;
          }
          if (hoursInput->text().isEmpty() || seuilInput->text().isEmpty()) {
              QMessageBox::warning(nullptr, "Erreur de Saisie", "Les heures et le seuil ne peuvent pas être vides.");
              return;
          }

          Machine newM(0, nameInput->text(), typeInput->text(), statusInput->text().isEmpty() ? "Normal" : statusInput->text(), hoursInput->text().toInt(), seuilInput->text().toInt());
          if (newM.ajouter()) {
              QMessageBox::information(nullptr, "Success", "Machine added with persistence!");
              nameInput->clear(); typeInput->clear(); statusInput->clear(); hoursInput->setText("0"); seuilInput->setText("100");
              
              if (refreshMachineTable) {
                  (*refreshMachineTable)();
              }
              
              if (outNestedStack && tabButtons.size() > 1) {
                  outNestedStack->setCurrentIndex(1);
                  tabButtons[1]->setChecked(true);
              }
          } else {
              QMessageBox::critical(nullptr, "Error", "Database insertion failed: " + newM.getLastError());
          }
      });

    } else if (name == "Asset Hub") {
      QWidget *controlBar = new QWidget();
      QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
      
      QLineEdit *searchEdit = new QLineEdit();
      searchEdit->setPlaceholderText("Search...");
      searchEdit->setStyleSheet(getInputStyle());
      searchEdit->setFixedWidth(120);

      QComboBox *searchType = new QComboBox();
      searchType->addItems({"Name", "Type"});
      searchType->setStyleSheet(getInputStyle());
      searchType->setFixedWidth(90);

      QPushButton *btnRefresh = new QPushButton("Refresh");
      btnRefresh->setStyleSheet(getButtonStyle());
      btnRefresh->setCursor(Qt::PointingHandCursor);
      btnRefresh->setFixedWidth(100);

      QPushButton *btnPrint = new QPushButton("PRINT PDF");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setFixedWidth(120);

      QComboBox *sortType = new QComboBox();
      sortType->addItems({"All", "Status", "Hours"});
      sortType->setStyleSheet(getInputStyle());
      sortType->setFixedWidth(110);

      QPushButton *btnAlert = new QPushButton("Check Alerts");
      btnAlert->setStyleSheet("QPushButton { background-color: #f39c12; color: white; border: none; border-radius: 8px; font-weight: 700; font-size: 13px; padding: 10px 18px; } QPushButton:hover { background-color: #e67e22; }");
      btnAlert->setCursor(Qt::PointingHandCursor);
      btnAlert->setFixedWidth(120);

      controlLayout->addWidget(searchType);
      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(new QLabel("Sort by:"));
      controlLayout->addWidget(sortType);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnRefresh);
      controlLayout->addStretch();
      controlLayout->addWidget(btnAlert);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnPrint);

      QStringList headers = {"ID", "Name", "Type", "Status", "Hours", "Threshold", "Actions"};
      QTableWidget *table = new QTableWidget();
      *machineTablePtr = table;
      table->setColumnCount(headers.size());
      table->setHorizontalHeaderLabels(headers);
      table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      table->horizontalHeader()->setSectionResizeMode(headers.size() - 1, QHeaderView::ResizeToContents);
      table->setColumnHidden(0, true);
      table->verticalHeader()->setVisible(false);
      table->setAlternatingRowColors(true);
      table->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; gridline-color: transparent; border-radius: 8px; alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; color: #666; }");

      cLayout->addWidget(controlBar);
      cLayout->addWidget(table);

      QObject::connect(sortType, &QComboBox::currentTextChanged, [table](const QString &text) {
          if (text == "Status") table->sortItems(3, Qt::AscendingOrder);
          else if (text == "Hours") table->sortItems(4, Qt::AscendingOrder);
          // For "All" we could ideally sort by ID if needed, but keeping it simple
          else if (text == "All") table->sortItems(0, Qt::AscendingOrder);
      });

      auto updateFilter = [table, searchEdit, searchType]() {
          QString lowerQuery = searchEdit->text().toLower();
          int col = (searchType->currentText() == "Type") ? 2 : 1; // 1 = Name, 2 = Type
          
          for (int i = 0; i < table->rowCount(); ++i) {
              bool match = false;
              if (table->item(i, col)) {
                  match = table->item(i, col)->text().toLower().contains(lowerQuery);
              }
              table->setRowHidden(i, !match);
          }
      };

      QObject::connect(searchEdit, &QLineEdit::textChanged, updateFilter);
      QObject::connect(searchType, &QComboBox::currentTextChanged, updateFilter);

      QObject::connect(btnRefresh, &QPushButton::clicked, [refreshMachineTable]() {
          (*refreshMachineTable)();
      });

      QObject::connect(btnAlert, &QPushButton::clicked, [table]() {
          if (!table) return;
          QString alertMsg;
          bool hasAlerts = false;
          for (int i = 0; i < table->rowCount(); ++i) {
             int id = table->item(i, 0)->text().toInt();
             QString nom = table->item(i, 1)->text();
             int heures = table->item(i, 4)->data(Qt::DisplayRole).toInt();
             int seuil = table->item(i, 5)->data(Qt::DisplayRole).toInt();
             if (heures >= seuil && seuil > 0) {
                alertMsg += "Machine " + QString::number(id) + " (" + nom + ") a atteint/dépassé son seuil (" + QString::number(heures) + "/" + QString::number(seuil) + "h).\n";
                hasAlerts = true;
                for(int c=0; c<table->columnCount() - 1; ++c) { // Exclude actions col
                   if(table->item(i, c)) table->item(i, c)->setBackground(QColor(255, 200, 200)); // Red tint
                }
             } else {
                for(int c=0; c<table->columnCount() - 1; ++c) {
                   if(table->item(i, c)) table->item(i, c)->setBackground(QBrush()); // Reset
                }
             }
          }
          if (hasAlerts) {
              QMessageBox::warning(table->window(), "Alertes de Maintenance", alertMsg);
              // Sending email
              Smtp* smtp = new Smtp("nour.benrhoumakok@gmail.com", "toualqtctfvdcnlp", "smtp.gmail.com", 465);
              QObject::connect(smtp, &Smtp::status, [table](const QString &status) {
                  qDebug() << "Email Status:" << status;
              });
              smtp->sendMail("nour.benrhoumakok@gmail.com", "nour.benrhoumakok@gmail.com", "Alerte de Maintenance Critique", "Les machines suivantes nécessitent une maintenance immédiate :\n\n" + alertMsg);
          } else {
              QMessageBox::information(table->window(), "Maintenance", "Toutes les machines sont en dessous de leur seuil de maintenance.");
          }
      });

      QObject::connect(btnPrint, &QPushButton::clicked, [table]() {
          if (!table) return;
          
          QString strStream;
          QTextStream out(&strStream);

          const int rowCount = table->rowCount();
          const int columnCount = table->columnCount() - 1; // Exclude Actions col

          out <<  "<html>\n"
              "<head>\n"
              "<meta Content=\"Text/html; charset=utf-8\">\n"
              <<  QString("<title>%1</title>\n").arg("Liste des Machines")
              <<  "</head>\n"
              "<body bgcolor=#ffffff link=#5000A0>\n"
              "<h1 style=\"text-align: center; color: #2c3e50; font-family: Arial, sans-serif;\">Rapport de Maintenance - Machines</h1>\n"
              "<p style=\"text-align: center; color: #7f8c8d;\">Généré le: " + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm") + "</p>\n"
              <<  "<table border=1 cellspacing=0 cellpadding=8 width=\"100%\" style=\"border-collapse: collapse; font-family: Arial, sans-serif;\">\n";

          // headers
          out << "<thead><tr bgcolor=#f0f0f0 style=\"color: #333;\">";
          for (int column = 0; column < columnCount; column++)
              if (!table->isColumnHidden(column))
                  out << QString("<th style=\"border: 1px solid #ddd;\">%1</th>").arg(table->horizontalHeaderItem(column)->text());
          out << "</tr></thead>\n<tbody>\n";

          // data
          for (int row = 0; row < rowCount; row++) {
              out << "<tr>";
              for (int column = 0; column < columnCount; column++) {
                  if (!table->isColumnHidden(column)) {
                      QString data;
                      if(table->item(row, column)) {
                          data = table->item(row, column)->text();
                          if(data.isEmpty()) data = table->item(row, column)->data(Qt::DisplayRole).toString();
                      }
                      out << QString("<td style=\"border: 1px solid #ddd; text-align: center;\">%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
                  }
              }
              out << "</tr>\n";
          }
          out <<  "</tbody></table>\n"
              "</body>\n"
              "</html>\n";

          QTextDocument document;
          document.setHtml(strStream);

          QString defaultName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "_Rapport_Machines.pdf";
          QString fileName = QFileDialog::getSaveFileName(table->window(), "Exporter en PDF", defaultName, "PDF Files (*.pdf)");
          
          if (fileName.isEmpty()) return;

          QPrinter printer(QPrinter::PrinterResolution);
          printer.setOutputFormat(QPrinter::PdfFormat);
          printer.setOutputFileName(fileName);
          printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
          
          document.print(&printer);
          QMessageBox::information(table->window(), "Succès", "Le PDF a été généré avec succès !\nEmplacement: " + fileName);
      });

      (*refreshMachineTable)();

    } else if (name == "Service History") {
      cLayout->addWidget(createStyledTable("Intervention History", {"Date", "Machine", "Action", "Result"}, {{"2024-02-27", "MAC-001", "Routine Check", "Success"}}, true));
    } else if (name == "Analytics") {
      GenericBarChart *chart = new GenericBarChart("Machine Status Overview");
      chart->addBar("Operational", 15, QColor(46, 204, 113));
      chart->addBar("Broken", 2, QColor(231, 76, 60));
      chart->addBar("Maintenance", 3, QColor(241, 196, 15));
      cLayout->addWidget(chart);
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

static QWidget *createProductPage(QStackedWidget *&outNestedStack) {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(40, 40, 40, 40);
  layout->setSpacing(25);
  layout->addWidget(new QLabel("Product Management"));

  QWidget *actionBar = new QWidget();
  QHBoxLayout *actionLayout = new QHBoxLayout(actionBar);
  actionLayout->setSpacing(12);

  outNestedStack = new QStackedWidget();
  QStringList tabNames = {"Product Hub", "Add Product", "Analytics"};
  QList<QPushButton *> tabButtons;

  // Use a shared pointer to hold the table pointer so the lambda can access it after initialization
  auto productTablePtr = std::make_shared<QTableWidget *>(nullptr);

  auto refreshProductTable = std::make_shared<std::function<void()>>();
  *refreshProductTable = [productTablePtr, refreshProductTable]() {
    QTableWidget *productTable = *productTablePtr;
    if (!productTable) return;
    Produit p;
    QSqlQueryModel *model = p.afficher();
    
    // Check for query errors
    if (model->lastError().isValid()) {
        qDebug() << "Refresh Table Error:" << model->lastError().text();
    }

    productTable->setRowCount(0);
    int rows = model->rowCount();
    productTable->setRowCount(rows);

    for (int i = 0; i < rows; ++i) {
      int pid = model->data(model->index(i, 0)).toInt();
      productTable->setItem(i, 0, new QTableWidgetItem(QString::number(pid)));
      productTable->setItem(i, 1, new QTableWidgetItem(model->data(model->index(i, 1)).toDate().toString("yyyy-MM-dd")));
      productTable->setItem(i, 2, new QTableWidgetItem(QString::number(model->data(model->index(i, 2)).toInt())));
      productTable->setItem(i, 3, new QTableWidgetItem(model->data(model->index(i, 3)).toString()));
      productTable->setItem(i, 4, new QTableWidgetItem(model->data(model->index(i, 4)).toString()));
      productTable->setItem(i, 5, new QTableWidgetItem(model->data(model->index(i, 5)).toString()));
      productTable->setItem(i, 6, new QTableWidgetItem(model->data(model->index(i, 6)).toString()));
      productTable->setItem(i, 7, new QTableWidgetItem(QString::number(model->data(model->index(i, 7)).toInt())));
      productTable->setItem(i, 8, new QTableWidgetItem(model->data(model->index(i, 8)).toString()));

      // Action Buttons
      QWidget *actionWidget = new QWidget();
      QHBoxLayout *al = new QHBoxLayout(actionWidget);
      al->setContentsMargins(10, 0, 10, 0);
      al->setSpacing(15);
      al->setAlignment(Qt::AlignCenter);

      QPushButton *btnEdit = new QPushButton("Edit");
      btnEdit->setCursor(Qt::PointingHandCursor);
      btnEdit->setMinimumWidth(80);
      btnEdit->setFixedHeight(28);
      btnEdit->setStyleSheet("QPushButton { background-color: #ffffff; color: #333333; border: 1px solid #cccccc; "
                             "border-radius: 6px; padding: 0px 8px; font-size: 13px; font-weight: 600; } "
                             "QPushButton:hover { border-color: #aaaaaa; color: #000000; background-color: #f6f6f6; }");

      QPushButton *btnDelete = new QPushButton("Remove");
      btnDelete->setCursor(Qt::PointingHandCursor);
      btnDelete->setMinimumWidth(80);
      btnDelete->setFixedHeight(28);
      btnDelete->setStyleSheet("QPushButton { background-color: #ffffff; border: 1px solid #d32f2f; color: #d32f2f; "
                               "border-radius: 6px; padding: 0px 8px; font-weight: 600; font-size: 13px; } "
                               "QPushButton:hover { background-color: #ffebee; border-color: #b71c1c; color: #b71c1c; }");

      al->addWidget(btnEdit);
      al->addWidget(btnDelete);
      productTable->setCellWidget(i, 9, actionWidget);

      // Connect Delete
      QObject::connect(btnDelete, &QPushButton::clicked, [productTablePtr, pid, refreshProductTable]() {
        QTableWidget *productTable = *productTablePtr;
        if (!productTable) return;
        if (QMessageBox::question(productTable->window(), "Delete Product", "Are you sure?") == QMessageBox::Yes) {
          Produit p;
          if (p.supprimer(pid)) {
            (*refreshProductTable)();
          } else {
            QMessageBox::critical(productTable->window(), "Error", "Failed to delete: " + p.getLastError());
          }
        }
      });

      // Connect Edit
      QObject::connect(btnEdit, &QPushButton::clicked, [productTablePtr, i, pid, refreshProductTable]() {
          QTableWidget *productTable = *productTablePtr;
          if (!productTable) return;
          QDialog dlg(productTable->window());
          dlg.setWindowTitle("Edit Product");
          dlg.setMinimumWidth(450);
          dlg.setStyleSheet("QDialog { background-color: #ffffff; }");
          QVBoxLayout *mainV = new QVBoxLayout(&dlg);
          mainV->setContentsMargins(30, 30, 30, 30);
          mainV->setSpacing(15);

          auto addField = [&](const QString &lbl, const QString &val) {
              mainV->addWidget(new QLabel(lbl));
              QLineEdit *le = new QLineEdit(val);
              le->setStyleSheet("QLineEdit { background-color: #f9fafb; border: 1px solid #eaeaea; "
                                "border-radius: 8px; padding: 10px; font-size: 14px; }");
              mainV->addWidget(le);
              return le;
          };

          // Removing eIdC and eRef as requested
          QLineEdit *eDate = addField("Date Pressage:", productTable->item(i, 1)->text());
          QLineEdit *eQnt = addField("Quantité:", productTable->item(i, 2)->text());
          QLineEdit *eRef = addField("Ref:", productTable->item(i, 3)->text());
          QLineEdit *eVisc = addField("Viscosité:", productTable->item(i, 4)->text());
          QLineEdit *eCol = addField("Couleur:", productTable->item(i, 5)->text());
          QLineEdit *eTst = addField("Test:", productTable->item(i, 6)->text());
          QLineEdit *eCap = addField("Capacité:", productTable->item(i, 7)->text());
          QLineEdit *eIdM = addField("ID Machine:", productTable->item(i, 8)->text());

          QPushButton *btnSave = new QPushButton("Save Changes");
          btnSave->setStyleSheet("QPushButton { background-color: #3DDC84; color: white; border: none; "
                                 "border-radius: 8px; padding: 12px; font-weight: 700; }");
          mainV->addWidget(btnSave);

          QObject::connect(btnSave, &QPushButton::clicked, [&dlg, eDate, eQnt, eRef, eVisc, eCol, eTst, eCap, eIdM, pid, refreshProductTable]() {
              Produit p;
              p.setIdContenair(pid);
              p.setDatePress(QDate::fromString(eDate->text(), "yyyy-MM-dd"));
              p.setQuantite(eQnt->text().toInt());
              p.setRef(eRef->text());
              p.setViscosite(eVisc->text());
              p.setCouleur(eCol->text());
              p.setTest(eTst->text());
              p.setCapacite(eCap->text().toInt());
              p.setIdMachine(eIdM->text().toInt());

              if (p.modifier()) {
                  dlg.accept();
                  (*refreshProductTable)();
              } else {
                  QMessageBox::critical(&dlg, "Error", p.getLastError());
              }
          });
          dlg.exec();
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

    if (name == "Product Hub") {
      // Container for list page to hold button + table
      QWidget *listPageWidget = new QWidget();
      QVBoxLayout *listPageLayout = new QVBoxLayout(listPageWidget);
      listPageLayout->setContentsMargins(0, 0, 0, 0);
      listPageLayout->setSpacing(10);

      // 1. Top Buttons (Refresh & Print)
      QWidget *topButtonsWidget = new QWidget();
      QHBoxLayout *topButtonsLayout = new QHBoxLayout(topButtonsWidget);
      topButtonsLayout->setContentsMargins(0, 0, 0, 0);
      topButtonsLayout->setSpacing(10);
      topButtonsLayout->addStretch();

      QPushButton *btnRefresh = new QPushButton("REFRESH");
      btnRefresh->setStyleSheet("QPushButton { background-color: #f8f9fa; border: 1px solid #dee2e6; color: #495057; "
                               "border-radius: 8px; padding: 10px 18px; font-weight: 700; font-size: 13px; } "
                               "QPushButton:hover { background-color: #e9ecef; border-color: #adb5bd; }");
      btnRefresh->setCursor(Qt::PointingHandCursor);
      btnRefresh->setFixedWidth(120);
      topButtonsLayout->addWidget(btnRefresh);

      QPushButton *btnPrint = new QPushButton("PRINT PDF");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedWidth(150);
      topButtonsLayout->addWidget(btnPrint);

      listPageLayout->addWidget(topButtonsWidget);

      // 2. Table
      *productTablePtr = new QTableWidget();
      QTableWidget *productTable = *productTablePtr;
      QStringList headers = {"ID Contenair", "Date Press", "Quantité",
                             "Ref", "Viscosité", "Couleur", "Test", "Capacité", "ID Machine", "Actions"};
      productTable->setColumnCount(headers.size());
      productTable->setHorizontalHeaderLabels(headers);
      productTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      productTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      productTable->horizontalHeader()->setSectionResizeMode(headers.size() - 1, QHeaderView::Fixed);
      productTable->setColumnWidth(headers.size() - 1, 220);
      productTable->verticalHeader()->setVisible(false);
      // productTable->setColumnHidden(0, true);
      // productTable->setColumnHidden(1, true); 
      // productTable->setColumnHidden(4, true); 
      productTable->verticalHeader()->setDefaultSectionSize(60);
      productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
      productTable->setAlternatingRowColors(true);
      productTable->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; "
          "gridline-color: transparent; border-radius: 8px; alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; border: none; "
          "border-bottom: 2px solid #f0f0f0; font-weight: 700; color: #666; "
          "text-transform: uppercase; font-size: 12px; }"
          "QTableWidget::item { padding: 12px; border-bottom: 1px solid #f5f5f5; color: #333; }"
          "QTableWidget::item:selected { background-color: #e6f9ef; color: #1a1a1a; }");

      listPageLayout->addWidget(productTable);

      (*refreshProductTable)();
      cLayout->addWidget(listPageWidget);

      // Connect Buttons
      QObject::connect(btnRefresh, &QPushButton::clicked, [refreshProductTable]() {
          (*refreshProductTable)();
      });

      QObject::connect(btnPrint, &QPushButton::clicked, [productTablePtr]() {
          QTableWidget *productTable = *productTablePtr;
          if (!productTable) return;
          QMessageBox::information(productTable->window(), "Print", "PDF Export started...");
      });

    } else if (name == "Add Product") {
      QWidget *formContainer = new QWidget();
      QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
      formLayout->setSpacing(15);
      formLayout->setContentsMargins(0, 0, 10, 0);

      QLabel *titleLabel = new QLabel("Add New Product");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a; margin-bottom: 25px;");
      formLayout->addWidget(titleLabel);

      auto createField = [&](const QString &lbl, const QString &ph) {
          formLayout->addWidget(new QLabel(lbl));
          QLineEdit *le = new QLineEdit();
          le->setPlaceholderText(ph);
          le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #e0e0e0; "
                            "border-radius: 8px; padding: 10px 14px; font-size: 14px; min-height: 45px; }");
          formLayout->addWidget(le);
          return le;
      };

      // Removed iIdC and iRef
      QLineEdit *iDate = createField("Date Pressage:", "YYYY-MM-DD");
      iDate->setText(QDate::currentDate().toString("yyyy-MM-dd"));
      QLineEdit *iQnt = createField("Quantité:", "500");
      QLineEdit *iRef = createField("Ref:", "PRD-001");
      QLineEdit *iVisc = createField("Viscosité:", "0.85");
      QLineEdit *iCol = createField("Couleur:", "Golden");
      QLineEdit *iTst = createField("Test:", "Compliant");
      QLineEdit *iCap = createField("Capacité:", "1000");
      QLineEdit *iIdM = createField("ID Machine:", "101");

      formLayout->addSpacing(20);
      QPushButton *btnAdd = new QPushButton("Add Product");
      btnAdd->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3DDC84, stop:1 #2DB66F); "
                            "color: white; border: none; border-radius: 8px; padding: 12px; font-weight: 700; }");
      btnAdd->setCursor(Qt::PointingHandCursor);
      formLayout->addWidget(btnAdd);
      formLayout->addStretch();

        QObject::connect(btnAdd, &QPushButton::clicked, [=]() {
          Produit p;
          p.setDatePress(QDate::fromString(iDate->text(), "yyyy-MM-dd"));
          p.setQuantite(iQnt->text().toInt());
          p.setRef(iRef->text());
          p.setViscosite(iVisc->text());
          p.setCouleur(iCol->text());
          p.setTest(iTst->text());
          p.setCapacite(iCap->text().toInt());
          p.setIdMachine(iIdM->text().toInt());

          if (p.ajouter()) {
              QMessageBox::information(nullptr, "Success", "Product added successfully!");
              iQnt->clear(); iVisc->clear(); iCol->clear(); iTst->clear(); iIdM->clear();
              
              // Switch to "Product Hub" tab first
              if (outNestedStack) outNestedStack->setCurrentIndex(0);
              if (!tabButtons.isEmpty()) tabButtons.first()->setChecked(true);
              
              // Refresh the table
              if (refreshProductTable) (*refreshProductTable)();
          } else {
              QMessageBox::critical(nullptr, "Error", p.getLastError());
          }
      });

      cLayout->addWidget(formContainer);
    } else if (name == "Analytics") {
      GenericBarChart *chart = new GenericBarChart("Product Production Overview");
      chart->addBar("Extra Virgin 1L", 450, QColor(61, 220, 132));
      chart->addBar("Premium Blend 500ml", 120, QColor(52, 152, 219));
      chart->addBar("Economy 2L", 80, QColor(241, 196, 15));
      cLayout->addWidget(chart);
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
                          "Request Hub", "Analytics", "PDF Printing"};
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
      searchEdit->setPlaceholderText("Search Employee...");
      searchEdit->setStyleSheet(getInputStyle());
      searchEdit->setFixedWidth(200);

      QPushButton *btnRefresh = new QPushButton("Refresh");
      btnRefresh->setStyleSheet(getButtonStyle());
      btnRefresh->setCursor(Qt::PointingHandCursor);
      btnRefresh->setFixedWidth(100);

      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnRefresh);
      controlLayout->addStretch();
      listPageLayout->addWidget(controlBar);

      // Table
      personnelTable = new QTableWidget();
      QStringList headers = {"CIN", "Name", "Salary", "Address", "Phone", "Exp", "Grade", "Role", "Actions"};
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
          "QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; "
          "gridline-color: transparent; border-radius: 8px; "
          "alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; "
          "border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; "
          "color: #666; text-transform: uppercase; font-size: 12px; }"
          "QTableWidget::item { padding: 12px; border-bottom: 1px solid #f5f5f5; "
          "color: #333; }"
          "QTableWidget::item:selected { background-color: #e6f9ef; color: #1a1a1a; }");

      listPageLayout->addWidget(personnelTable);
      cLayout->addWidget(listPageWidget);

      auto refreshTable = [personnelTable]() {
          Personnel p;
          QSqlQueryModel *model = p.afficher();
          personnelTable->setRowCount(0);
          int rowCount = model->rowCount();
          personnelTable->setRowCount(rowCount);

          for (int i = 0; i < rowCount; ++i) {
              for (int j = 0; j < 8; ++j) {
                  personnelTable->setItem(i, j, new QTableWidgetItem(model->data(model->index(i, j)).toString()));
              }

              // Action Buttons
              QWidget *actionWidget = new QWidget();
              QHBoxLayout *actionBtnLayout = new QHBoxLayout(actionWidget);
              actionBtnLayout->setContentsMargins(10, 0, 10, 0);
              actionBtnLayout->setSpacing(10);
              actionBtnLayout->setAlignment(Qt::AlignCenter);

              QPushButton *btnModify = new QPushButton("Edit");
              btnModify->setCursor(Qt::PointingHandCursor);
              btnModify->setMinimumWidth(70);
              btnModify->setFixedHeight(28);
              btnModify->setStyleSheet("QPushButton { background-color: #ffffff; color: #333; border: 1px solid #ccc; border-radius: 6px; font-size: 13px; font-weight: 600; } QPushButton:hover { background-color: #f6f6f6; }");

              QPushButton *btnDelete = new QPushButton("Remove");
              btnDelete->setCursor(Qt::PointingHandCursor);
              btnDelete->setMinimumWidth(70);
              btnDelete->setFixedHeight(28);
              btnDelete->setStyleSheet("QPushButton { background-color: #ffffff; color: #d32f2f; border: 1px solid #d32f2f; border-radius: 6px; font-size: 13px; font-weight: 600; } QPushButton:hover { background-color: #ffebee; }");

              actionBtnLayout->addWidget(btnModify);
              actionBtnLayout->addWidget(btnDelete);

              QObject::connect(btnDelete, &QPushButton::clicked, [personnelTable, i]() {
                  int cin = personnelTable->item(i, 0)->text().toInt();
                  if (QMessageBox::question(nullptr, "Confirm Deletion", "Are you sure you want to delete this employee?") == QMessageBox::Yes) {
                      Personnel p;
                      if (p.supprimer(cin)) {
                         QMessageBox::information(nullptr, "Deleted", "Employee removed successfully.");
                         // Refresh via button or signal if needed
                      }
                  }
              });

              QObject::connect(btnModify, &QPushButton::clicked, [personnelTable, i]() {
                  QDialog *dialog = new QDialog();
                  dialog->setWindowTitle("Modify Personnel");
                  dialog->setMinimumWidth(400);
                  QVBoxLayout *dLayout = new QVBoxLayout(dialog);
                  
                  QList<QLineEdit*> inputs;
                  QStringList fieldNames = {"CIN", "Name", "Salary", "Address", "Phone", "Experience", "Grade", "Role"};
                  for (int c = 0; c < 8; ++c) {
                      dLayout->addWidget(new QLabel(fieldNames[c]));
                      QLineEdit *le = new QLineEdit(personnelTable->item(i, c)->text());
                      if (c == 0) le->setEnabled(false); // CIN is PK
                      dLayout->addWidget(le);
                      inputs.append(le);
                  }

                  QPushButton *btnSave = new QPushButton("Save Changes");
                  dLayout->addWidget(btnSave);
                  QObject::connect(btnSave, &QPushButton::clicked, [=]() {
                      Personnel p;
                      p.setCin(inputs[0]->text().toInt());
                      p.setNom(inputs[1]->text());
                      p.setSalaire(inputs[2]->text().toDouble());
                      p.setAdresse(inputs[3]->text());
                      p.setTel(inputs[4]->text());
                      p.setExperience(inputs[5]->text().toInt());
                      p.setGrade(inputs[6]->text());
                      p.setRole(inputs[7]->text());

                      if (p.modifier()) {
                          QMessageBox::information(nullptr, "Success", "Record updated.");
                          dialog->accept();
                      } else {
                          QMessageBox::critical(nullptr, "Error", p.getLastError());
                      }
                  });
                  dialog->exec();
              });

              personnelTable->setCellWidget(i, 8, actionWidget);
          }
      };

      QObject::connect(btnRefresh, &QPushButton::clicked, refreshTable);
      refreshTable(); // Initial load

      // Search Logic
      QObject::connect(searchEdit, &QLineEdit::textChanged, [personnelTable](const QString &text) {
          QString query = text.toLower();
          for (int i = 0; i < personnelTable->rowCount(); ++i) {
              bool match = false;
              if (personnelTable->item(i, 1))
                  match = personnelTable->item(i, 1)->text().toLower().contains(query);
              personnelTable->setRowHidden(i, !match);
          }
      });

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

      QLineEdit *cinInput = new QLineEdit(); cinInput->setPlaceholderText("CIN Number");
      QLineEdit *nameInput = new QLineEdit(); nameInput->setPlaceholderText("Full Name");
      QLineEdit *salInput = new QLineEdit(); salInput->setPlaceholderText("Salary");
      QLineEdit *addrInput = new QLineEdit(); addrInput->setPlaceholderText("Address");
      QLineEdit *telInput = new QLineEdit(); telInput->setPlaceholderText("Phone Number");
      QLineEdit *expInput = new QLineEdit(); expInput->setPlaceholderText("Years of Exp");
      QComboBox *gradeInput = new QComboBox();
      gradeInput->addItems({"Junior", "Senior", "Lead", "Principal", "Chief"});
      QComboBox *roleInput = new QComboBox();
      roleInput->addItems({"Stock Management", "Product Management", "Maintenance Management", "Personnel Management", "Order Management", "Financial Management"});

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

      QPushButton *btnHire = new QPushButton("Hire Employee");
      btnHire->setStyleSheet(getButtonStyle());
      btnHire->setFixedHeight(45);
      formLayout->addWidget(btnHire);
      formLayout->addStretch();

      QObject::connect(btnHire, &QPushButton::clicked, [=]() {
          if (cinInput->text().isEmpty() || nameInput->text().isEmpty()) {
              QMessageBox::warning(nullptr, "Error", "CIN and Name are required!");
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

          if (p.ajouter()) {
              QMessageBox::information(nullptr, "Success", "Employee added successfully.");
              cinInput->clear(); nameInput->clear(); salInput->clear(); addrInput->clear();
              telInput->clear(); expInput->clear();
              outNestedStack->setCurrentIndex(0);
          } else {
              QMessageBox::critical(nullptr, "Error", p.getLastError());
          }
      });

      cLayout->addWidget(formContainer);

    } else if (name == "Payroll") {
      cLayout->addWidget(createStyledTable(
          "Recent Payroll Runs",
          {"Period", "Total Payout", "Status", "Date Processed"},
          {{"October 2023", "$145,000", "Completed", "2023-10-31"},
           {"September 2023", "$142,500", "Completed", "2023-09-30"}},
          true));
    } else if (name == "Analytics") {
      QWidget *statsContainer = new QWidget();
      QVBoxLayout *statsLayout = new QVBoxLayout(statsContainer);
      statsLayout->setSpacing(30);

      GenericBarChart *chart1 = new GenericBarChart("Department Headcount");
      chart1->addBar("Sales", 12, QColor(52, 152, 219));
      chart1->addBar("Product", 8, QColor(46, 204, 113));
      chart1->addBar("Maintenance", 5, QColor(241, 196, 15));
      chart1->addBar("Finance", 4, QColor(155, 89, 182));

      QWidget *c1Wrapper = new QWidget();
      c1Wrapper->setStyleSheet(getCardStyle());
      QVBoxLayout *v1 = new QVBoxLayout(c1Wrapper);
      v1->addWidget(chart1);
      statsLayout->addWidget(c1Wrapper);

      GenericBarChart *chart2 = new GenericBarChart("Absence Days (YTD)");
      chart2->addBar("John Doe", 2, QColor(231, 76, 60));
      chart2->addBar("Jane Smith", 5, QColor(231, 76, 60));
      chart2->addBar("Robert Brown", 0, QColor(46, 204, 113));
      chart2->addBar("Emily White", 3, QColor(243, 156, 18));

      QWidget *c2Wrapper = new QWidget();
      c2Wrapper->setStyleSheet(getCardStyle());
      QVBoxLayout *v2 = new QVBoxLayout(c2Wrapper);
      v2->addWidget(chart2);
      statsLayout->addWidget(c2Wrapper);

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

      vbox->addWidget(new QLabel("Employee Name:"));
      QLineEdit *eName = new QLineEdit();
      eName->setStyleSheet(inputStyle);
      vbox->addWidget(eName);
      vbox->addWidget(new QLabel("Start Date:"));
      QLineEdit *sDate = new QLineEdit();
      sDate->setStyleSheet(inputStyle);
      vbox->addWidget(sDate);
      vbox->addWidget(new QLabel("End Date:"));
      QLineEdit *eDate = new QLineEdit();
      eDate->setStyleSheet(inputStyle);
      vbox->addWidget(eDate);
      vbox->addWidget(new QLabel("Reason:"));
      QLineEdit *reason = new QLineEdit();
      reason->setStyleSheet(inputStyle);
      vbox->addWidget(reason);

      vbox->addSpacing(30);

      QPushButton *btnPrint = new QPushButton("Print Request to PDF");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedHeight(45);
      vbox->addWidget(btnPrint);
      vbox->addStretch();

      cLayout->addWidget(formContainer);

      QObject::connect(btnPrint, &QPushButton::clicked, [=]() {
        QMessageBox::information(nullptr, "Info",
                                 "PDF Generation Service Started...");
      });

    } else if (name == "Request Hub") {
      cLayout->addWidget(createStyledTable(
          "Pending Absence Requests",
          {"Staff Member", "Type", "Duration", "Status"},
          {{"Jane Smith", "Vacation", "Nov 5 - Nov 12", "Pending Approval"},
           {"Robert Brown", "Sick Leave", "Oct 26", "Approved"}},
          true, false, true));
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("oil press manager");
  resize(1200, 820);
  setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

  // --- Main Application UI ---
  QWidget *mainAppWidget = new QWidget(this);
  setCentralWidget(mainAppWidget);
  mainAppWidget->setStyleSheet("background-color: #f4f6f8;");

  QVBoxLayout *verticalRoot = new QVBoxLayout(mainAppWidget);
  verticalRoot->setContentsMargins(0, 0, 0, 0);
  verticalRoot->setSpacing(0);

  // Add Custom Title Bar
  verticalRoot->addWidget(createTitleBar());

  // Content Container (Sidebar + Stack)
  QWidget *contentContainer = new QWidget();
  QHBoxLayout *contentLayout = new QHBoxLayout(contentContainer);
  contentLayout->setContentsMargins(0, 0, 0, 0);
  contentLayout->setSpacing(0);
  verticalRoot->addWidget(contentContainer,
                          1); // Give it stretch factor 1 to fill height

  // Sidebar
  QWidget *sidebar = new QWidget();
  sidebar->setFixedWidth(280);
  sidebar->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                         "stop:0 #111111, stop:1 #1a1a1a);"
                         "border-right: 1px solid #333;");

  QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
  sidebarLayout->setContentsMargins(20, 30, 20, 40);
  sidebarLayout->setSpacing(10);

  // LOGO INTEGRATION
  QLabel *brand = new QLabel();
  brand->setStyleSheet(
      "background: transparent; border: none;"); // Force transparency to fix
                                                 // the light-colored box
  QPixmap logoPixmap(":/logo.png");
  if (!logoPixmap.isNull()) {
    brand->setPixmap(logoPixmap.scaledToHeight(160, Qt::SmoothTransformation));
    brand->setAlignment(Qt::AlignCenter);
  } else {
    brand->setText("oilpress managment app");
    brand->setStyleSheet("color: white; font-size: 24px; font-weight: 900;");
  }

  sidebarLayout->addWidget(brand);
  sidebarLayout->addSpacing(20);

  // ... helper for styles ...
  auto getSidebarStyle = []() {
    return "QPushButton {"
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
  };

  navItems.clear(); // Ensure it starts fresh in case of multiple logins (if
                    // supported)

  auto addNav = [&](const QString &title) {
    QPushButton *btn = new QPushButton(title);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(getSidebarStyle());
    sidebarLayout->addWidget(btn);
    navItems.append({title, btn});
    return btn;
  };

  QPushButton *btnHome = addNav("Overview");
  sidebarLayout->addSpacing(10);
  QLabel *menuLabel = new QLabel("MODULES");
  menuLabel->setStyleSheet(
      "color: #444; font-weight: bold; font-size: 10px; margin-top: 10px; "
      "margin-bottom: 5px; padding-left: 10px;");
  sidebarLayout->addWidget(menuLabel);

  QPushButton *btnClient = addNav("Order Management");
  QPushButton *btnFinance = addNav("Financial Management");
  QPushButton *btnInventory = addNav("Stock Management");
  QPushButton *btnMaintenance = addNav("Maintenance Management");
  QPushButton *btnProduct = addNav("Product Management");
  QPushButton *btnPersonnel = addNav("Personnel Management"); // NEW BUTTON

  sidebarLayout->addStretch();

  // Eye Saver Button Integration
  EyeSaverButton *eyeSaver = new EyeSaverButton();
  sidebarLayout->addWidget(eyeSaver, 0, Qt::AlignLeft);
  sidebarLayout->addSpacing(10);

  // Logout Button
  QPushButton *btnLogout = new QPushButton("Log Out");
  btnLogout->setCursor(Qt::PointingHandCursor);
  btnLogout->setStyleSheet(
      "QPushButton { background-color: transparent; color: #e74c3c; "
      "font-weight: bold; border: none; text-align: left; padding: 10px; "
      "margin-left: -5px; } QPushButton:hover { background-color: rgba(231, "
      "76, 60, 0.1); border-radius: 5px; }");
  sidebarLayout->addWidget(btnLogout, 0, Qt::AlignLeft);

  // Logout Logic
  connect(btnLogout, &QPushButton::clicked, this,
          [this]() { emit logoutRequested(); });

  stackedWidget = new QStackedWidget();

  // --- HOME DASHBOARD ---
  QWidget *homePage = new QWidget();
  QVBoxLayout *homeLayout = new QVBoxLayout(homePage);
  homeLayout->setContentsMargins(30, 30, 30,
                                 30); // Reduced from 50 to look better on 1080p
  homeLayout->setSpacing(30);

  QWidget *topBar = new QWidget();
  QHBoxLayout *topLayout = new QHBoxLayout(topBar);
  topLayout->setContentsMargins(0, 0, 0, 0);
  QLabel *welcome = new QLabel("Welcome back, Admin");
  welcome->setStyleSheet("font-size: 36px; font-weight: 800; color: #1a1a1a; "
                         "letter-spacing: -1px;");
  QLabel *dateLabel = new QLabel(QDate::currentDate().toString("dddd, MMMM d"));
  dateLabel->setStyleSheet("font-size: 16px; color: #777; font-weight: 500;");
  topLayout->addWidget(welcome);
  topLayout->addStretch();
  topLayout->addWidget(dateLabel);
  homeLayout->addWidget(topBar);

  QWidget *statsContainer = new QWidget();
  QHBoxLayout *statsLayout = new QHBoxLayout(statsContainer);
  statsLayout->setContentsMargins(0, 0, 0, 0);
  statsLayout->setSpacing(20);
  statsLayout->addWidget(createStatCard("Total Revenue", "$128,400",
                                        "+12% this month", "#3DDC84"));
  statsLayout->addWidget(
      createStatCard("Active Clients", "1,245", "+5 new today", "#3498db"));
  statsLayout->addWidget(
      createStatCard("Pending Orders", "18", "Requires attention", "#e74c3c"));
  statsLayout->addWidget(createStatCard("System Status", "99.9%",
                                        "All systems operational", "#f1c40f"));
  homeLayout->addWidget(statsContainer, 2); // Stretch 2 for stats

  QWidget *bottomSection = new QWidget();
  QHBoxLayout *splitLayout = new QHBoxLayout(bottomSection);
  splitLayout->setContentsMargins(0, 0, 0, 0);
  splitLayout->setSpacing(30);

  QWidget *quickActions = new QWidget();
  quickActions->setStyleSheet(getCardStyle());
  QVBoxLayout *qaLayout = new QVBoxLayout(quickActions);
  qaLayout->setContentsMargins(25, 25, 25, 25);
  QLabel *qaTitle = new QLabel("Quick Actions");
  qaTitle->setStyleSheet(
      "font-size: 18px; font-weight: 700; color: #222; margin-bottom: 15px;");
  qaLayout->addWidget(qaTitle);

  QString actionBtnStyle =
      "QPushButton { text-align: left; padding: 15px; border: 1px solid #eee; "
      "border-radius: 8px; background-color: #fafafa; font-weight: 600; color: "
      "#444; } QPushButton:hover { background-color: #f0f0f0; border-color: "
      "#ddd; }";

  QPushButton *btnQInvoice = new QPushButton("Draft New Invoice");
  btnQInvoice->setStyleSheet(actionBtnStyle);
  QPushButton *btnQClient = new QPushButton("Register New Client");
  btnQClient->setStyleSheet(actionBtnStyle);
  QPushButton *btnQStock = new QPushButton("Add Inventory Stock");
  btnQStock->setStyleSheet(actionBtnStyle);

  qaLayout->addWidget(btnQInvoice);
  qaLayout->addSpacing(5);
  qaLayout->addWidget(btnQClient);
  qaLayout->addSpacing(5);
  qaLayout->addWidget(btnQStock);
  qaLayout->addStretch();

  QWidget *activityWidget = new QWidget();
  activityWidget->setStyleSheet(getCardStyle());
  QVBoxLayout *actLayout = new QVBoxLayout(activityWidget);
  actLayout->setContentsMargins(25, 25, 25, 25);
  actLayout->addWidget(
      new QLabel("Recent Activity")); // Placeholder for brevity
  actLayout->addStretch();
  splitLayout->addWidget(quickActions, 1);
  splitLayout->addWidget(activityWidget, 2);
  homeLayout->addWidget(bottomSection, 3); // Stretch 3 for bottom area

  stackedWidget->addWidget(homePage);

  // --- SUB PAGES & Internal Logic ---
  QStackedWidget *stackClient, *stackFinance, *stackInventory,
      *stackMaintenance, *stackProduct, *stackPersonnel;

  stackedWidget->addWidget(createClientPage(stackClient));
  stackedWidget->addWidget(createFinancePage(stackFinance));
  stackedWidget->addWidget(createInventoryPage(stackInventory));
  stackedWidget->addWidget(createMaintenancePage(stackMaintenance));
  stackedWidget->addWidget(createProductPage(stackProduct));
  stackedWidget->addWidget(createPersonnelPage(stackPersonnel)); // Add to stack

  contentLayout->addWidget(sidebar);
  contentLayout->addWidget(stackedWidget);

  mainAppWidget->setLayout(verticalRoot);

  // Sidebar Navigation
  auto setActive = [=](int index, QPushButton *activeBtn) {
    stackedWidget->setCurrentIndex(index);
    for (auto &item : navItems)
      item.btn->setChecked(item.btn == activeBtn);
  };

  connect(btnHome, &QPushButton::clicked, this,
          [=]() { setActive(0, btnHome); });
  connect(btnClient, &QPushButton::clicked, this,
          [=]() { setActive(1, btnClient); });
  connect(btnFinance, &QPushButton::clicked, this,
          [=]() { setActive(2, btnFinance); });
  connect(btnInventory, &QPushButton::clicked, this,
          [=]() { setActive(3, btnInventory); });
  connect(btnMaintenance, &QPushButton::clicked, this,
          [=]() { setActive(4, btnMaintenance); });
  connect(btnProduct, &QPushButton::clicked, this,
          [=]() { setActive(5, btnProduct); });
  connect(btnPersonnel, &QPushButton::clicked, this,
          [=]() { setActive(6, btnPersonnel); }); // Link button

  // Quick Action Signal Connections
  connect(btnQClient, &QPushButton::clicked, this, [=]() {
    setActive(1, btnClient);         // Switch to Client Page
    stackClient->setCurrentIndex(0); // Add New Client tab
  });

  connect(btnQInvoice, &QPushButton::clicked, this, [=]() {
    setActive(2, btnFinance);         // Switch to Finance Page
    stackFinance->setCurrentIndex(0); // Create Invoice tab
  });

  connect(btnQStock, &QPushButton::clicked, this, [=]() {
    setActive(3, btnInventory);         // Switch to Inventory Page
    stackInventory->setCurrentIndex(0); // Add Stock Item tab
  });

  // Start at Home (StackedWidget Index 0)
  stackedWidget->setCurrentIndex(0);

  // Auto-select Home in the sidebar
  btnHome->setChecked(true);
}

void MainWindow::applyRole(int roleIndex) {
  // roleIndex 0: Super Admin
  // 1: Order Manager, 2: Financial, 3: Stock, 4: Maintenance, 5: Product, 6:
  // Personnel

  // Overview (index 0 in navItems) is always enabled
  for (int i = 0; i < navItems.size(); ++i) {
    bool enabled = (roleIndex == 0) || (i == 0) || (i == roleIndex);
    navItems[i].btn->setEnabled(enabled);

    if (!enabled) {
      navItems[i].btn->setStyleSheet(
          "QPushButton { color: #555; background-color: transparent; border: "
          "none; padding: 14px 20px; text-align: left; font-size: 14px; }");
    } else {
      // Restore normal style
      navItems[i].btn->setStyleSheet(
          "QPushButton { color: #999; background-color: transparent; border: "
          "none; border-radius: 8px; padding: 14px 20px; text-align: left; "
          "font-size: 14px; font-weight: 500; } "
          "QPushButton:hover { background-color: rgba(255, 255, 255, 0.05); "
          "color: #fff; } "
          "QPushButton:checked { background-color: #3DDC84; color: #ffffff; "
          "font-weight: 700; }");
    }
  }
}
