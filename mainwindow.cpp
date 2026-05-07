#include "mainwindow.h"
#include "ConfigManager.h"
#include "smsapi.h"
#include "trackingapi.h"
#include "AuthWidgets.h"
#include "EyeSaverButton.h"
#include "article.h"
#include "commande.h"
#include "transaction.h"
#include "produit.h"
#include "personnel.h"
#include "machine.h"
#include "smtp.h"
#include "GasAlertWidget.h"
#include "emailapi.h"
#include "marketapi.h"
#include "consultantagent.h"
#include "dealgenerator.h"
#include "anomalyapi.h"
#include "ocrscannerapi.h"
#include <functional>
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
#include <QUrl>
#include <QPrintDialog>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextStream>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLinearGradient>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPageSize>
#include <QPageLayout>
#include <QSqlQuery>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPrinter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QTimer>
#include <QSqlQueryModel>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>
#include <QDesktopServices>
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

// ──────────────────────────────────────────────────────────────────────────
// PDF Invoice Generator — High Fidelity (Matches USER Picture) — Balanced Size
// ──────────────────────────────────────────────────────────────────────────
static void generateOrderInvoicePdf(QWidget *parent,
                                     int     orderId,
                                     const QString &ref,
                                     const QString &dateCommande,
                                     const QString &datelivraison,
                                     const QString &etat,
                                     const QString &client,
                                     const QString &address)
{
    Q_UNUSED(datelivraison);
    // Ask user where to save
    QString defaultName = (orderId > 0) ? QString("invoice_%1.pdf").arg(ref) : QString("receipt_%1.pdf").arg(QDate::currentDate().toString("yyyyMMdd"));
    QString fileName = QFileDialog::getSaveFileName(
        parent, "Save PDF Document", defaultName, "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    // Fetch Line Items
    struct LineItem { QString ref; double price; int qty; };
    QVector<LineItem> items;
    double calculatedTotal = 0.0;
    QString payMode = "Cash"; // Default

    if (orderId > 0) {
        QSqlQuery q;
        q.prepare("SELECT P.REF, P.PRIX_UNITAIRE, C.QUANTITE_DEMANDEE FROM CONTENIR C JOIN PRODUIT P ON C.ID_CONTENAIR = P.ID_CONTENAIR WHERE C.ID_COMMANDE = :id");
        q.bindValue(":id", orderId);
        if (q.exec()) {
            while(q.next()) {
                QString itemRef = q.value(0).toString();
                double itemPrice = q.value(1).toDouble();
                int itemQty = q.value(2).toInt();
                items.append({itemRef, itemPrice, itemQty});
                calculatedTotal += (itemPrice * itemQty);
            }
        }

        // Check if a Revenue record already exists in FINANCE
        QSqlQuery checkF;
        checkF.prepare("SELECT ID_TRANSACTION, MODE_PAIEMENT FROM FINANCE WHERE ID_COMMANDE = :id AND TYPE_TRANSACTION = 'Revenue'");
        checkF.bindValue(":id", orderId);

        if (checkF.exec() && checkF.next()) {
            int transId = checkF.value(0).toInt();
            payMode = checkF.value(1).toString();
            // Update existing record
            QSqlQuery updF;
            updF.prepare("UPDATE FINANCE SET MONTANT = :mt WHERE ID_TRANSACTION = :tid");
            updF.bindValue(":mt", calculatedTotal);
            updF.bindValue(":tid", transId);
            updF.exec();
        } else {
            // Create new Revenue record
            QSqlQuery nextIdQ;
            nextIdQ.exec("SELECT NVL(MAX(ID_TRANSACTION), 0) + 1 FROM FINANCE");
            int nextId = (nextIdQ.next()) ? nextIdQ.value(0).toInt() : 1;

            QSqlQuery insF;
            insF.prepare("INSERT INTO FINANCE (ID_TRANSACTION, MONTANT, DATE_TRANSACTION, TYPE_TRANSACTION, MODE_PAIEMENT, DESCRIPTION, ID_COMMANDE) "
                         "VALUES (:id, :mt, :dt, 'Revenue', :mode, :desc, :oid)");
            insF.bindValue(":id", nextId);
            insF.bindValue(":mt", calculatedTotal);
            insF.bindValue(":dt", QDate::currentDate());
            insF.bindValue(":mode", "Cash");
            insF.bindValue(":desc", "Invoice Payment (" + ref + ")");
            insF.bindValue(":oid", orderId);
            insF.exec();
        }
    }

    // Build the Balanced HTML
    QString html =
        "<html><body style='font-family: Arial, sans-serif; color: #1a1a1a; margin: 0; padding: 0;'>"
        "<div style='padding: 30pt;'>"
        "<table width='100%' cellpadding='0' cellspacing='0' style='margin-bottom: 20pt;'>"
        "  <tr>"
        "    <td width='100pt'><img src='qrc:/logo.png' width='80'></td>"
        "    <td style='font-size: 32pt; font-weight: 900; color: #3DDC84; letter-spacing: -1pt;'>INVOICE</td>"
        "    <td align='right' style='font-size: 11pt; color: #666;'>"
        "      <span style='font-weight: bold; color: #2C3E1F; font-size: 14pt;'>Oil Press Manager Pro</span><br>"
        "      Tunis, Tunisia &nbsp;|&nbsp; contact@oilpress.tn"
        "    </td>"
        "  </tr>"
        "</table>"

        // THE GREEN BAR
        "<div style='background-color: #2C3E1F; height: 4pt; width: 100%; margin-bottom: 30pt;'></div>"

        "<table width='100%' cellpadding='0' cellspacing='0' style='margin-bottom: 40pt;'>"
        "  <tr>"
        "    <td width='55%' valign='top'>"
        "      <div style='font-size: 9pt; color: #888; text-transform: uppercase; font-weight: 800; margin-bottom: 5pt;'>Bill To</div>"
        "      <div style='font-size: 14pt; font-weight: 800;'>" + (client.isEmpty() ? "Walk-in Customer" : client.toHtmlEscaped()) + "</div>"
        "      <div style='font-size: 11pt; color: #555;'>" + (address.isEmpty() ? "Address N/A" : address.toHtmlEscaped()) + "</div>"
        "    </td>"
        "    <td width='45%' valign='top' align='right' style='font-size: 11pt; line-height: 1.6;'>"
        "      <b>Reference:</b> " + (ref.isEmpty() ? "N/A" : ref.toHtmlEscaped()) + "<br>"
        "      <b>Date:</b> " + dateCommande + "<br>"
        "      <b>Payment:</b> " + (payMode.isEmpty() ? "Cash" : payMode) + "<br>"
        "      <b>Status:</b> <span style='color: #27ae60;'>" + etat.toUpper() + "</span>"
        "    </td>"
        "  </tr>"
        "</table>"

        "<table width='100%' cellpadding='12' cellspacing='0' style='border: 1pt solid #eee; margin-bottom: 30pt;'>"
        "  <tr style='background-color: #2C3E1F; color: white;'>"
        "    <td style='font-weight: 800; font-size: 10pt;'>Description</td>"
        "    <td align='center' style='font-weight: 800; font-size: 10pt;'>Qty</td>"
        "    <td align='right' style='font-weight: 800; font-size: 10pt;'>Unit Price</td>"
        "    <td align='right' style='font-weight: 800; font-size: 10pt;'>Total</td>"
        "  </tr>";

    if (items.isEmpty()) {
        html += "<tr><td colspan='4' align='center' style='padding: 30pt; color: #aaa; border-bottom: 1pt solid #eee; font-size: 12pt;'>No items found.</td></tr>";
    } else {
        for (const auto &li : items) {
            html +=
                "<tr>"
                "  <td style='border-bottom: 1pt solid #eee; font-size: 11pt;'><b>" + li.ref + "</b></td>"
                "  <td align='center' style='border-bottom: 1pt solid #eee; font-size: 11pt;'>" + QString::number(li.qty) + "</td>"
                "  <td align='right' style='border-bottom: 1pt solid #eee; font-size: 11pt;'>" + QString::number(li.price, 'f', 2) + "</td>"
                "  <td align='right' style='border-bottom: 1pt solid #eee; font-size: 11pt;'><b>" + QString::number(li.qty * li.price, 'f', 2) + " TND</b></td>"
                "</tr>";
        }
    }

    html += "</table>"

        "<table width='100%'><tr>"
        "  <td width='60%'></td>"
        "  <td width='40%' align='right' style='background: #f9fafb; padding: 20pt; border: 1pt solid #3DDC84;'>"
        "    <div style='font-size: 10pt; color: #888; text-transform: uppercase; font-weight: bold;'>Total Revenue</div>"
        "    <div style='font-size: 20pt; font-weight: 900; color: #3DDC84;'>" + QString::number(calculatedTotal, 'f', 2) + " TND</div>"
        "  </td>"
        "</tr></table>"

        "<div style='margin-top: 80pt;'>"
        "  <table width='100%'><tr>"
        "    <td width='70%' style='font-size: 10pt; color: #999;'>Thank you for your business. Generated by Oil Press Manager.</td>"
        "    <td align='center' style='width: 150pt; border-top: 1.5pt solid #000; padding-top: 5pt; font-size: 11pt; font-weight: 800;'>Authorized Signature</td>"
        "  </tr></table>"
        "</div>"
        "</div></body></html>";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(printer.pageRect(QPrinter::Point).size());
    doc.print(&printer);

    QMessageBox::information(parent, "Success", "Invoice Generated and Revenue Updated!");
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
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
  QStringList tabNames = {"New Order", "Order Hub", "Analytics"};
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

      QWidget *wRef, *wDate, *wClient, *wAddress, *wDelivery, *wPhone;
      addField(formLayout, labelStyle, inputStyle, "Reference: (e.g. AB12)", "e.g. AB12", wRef, false);
      // Reference: exactly 2 letters then one or more digits (e.g. AB12, OP999)
      static_cast<QLineEdit*>(wRef)->setValidator(
          new QRegularExpressionValidator(QRegularExpression("^[A-Za-z]{0,2}[0-9]*$"), page));
      static_cast<QLineEdit*>(wRef)->setPlaceholderText("e.g. AB12");
      addField(formLayout, labelStyle, inputStyle, "Order Date:", "", wDate, true);
      static_cast<QDateEdit*>(wDate)->setMinimumDate(QDate::currentDate().addDays(-7));
      static_cast<QDateEdit*>(wDate)->setMaximumDate(QDate::currentDate());
      addField(formLayout, labelStyle, inputStyle, "Client's Name:", "Enter client's name", wClient, false);
      static_cast<QLineEdit*>(wClient)->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z \\.]*$"), page));
      addField(formLayout, labelStyle, inputStyle, "Client's Address:", "Enter address", wAddress, false);
      addField(formLayout, labelStyle, inputStyle, "Phone Number:", "+216 12 345 678", wPhone, false);
      addField(formLayout, labelStyle, inputStyle, "Delivery Date:", "", wDelivery, true);
      static_cast<QDateEdit*>(wDelivery)->setMinimumDate(QDate::currentDate());

      // Delivery Status dropdown
      QLabel *delivStatusLbl = new QLabel("Delivery Status:");
      delivStatusLbl->setStyleSheet(labelStyle);
      formLayout->addWidget(delivStatusLbl);
      QComboBox *wDeliveryStatus = new QComboBox();
      wDeliveryStatus->addItems({"Preparing", "Dispatched", "In Transit", "Delivered"});
      wDeliveryStatus->setStyleSheet(inputStyle);
      formLayout->addWidget(wDeliveryStatus);

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

      // --- NEW PRODUCT SELECTION SECTION ---
      formLayout->addSpacing(20);
      QLabel *itemsTitle = new QLabel("Order Items (Products):");
      itemsTitle->setStyleSheet("font-size: 16px; font-weight: 700; color: #1a1a1a; margin-top: 10px;");
      formLayout->addWidget(itemsTitle);

      QWidget *itemSelectArea = new QWidget();
      itemSelectArea->setStyleSheet("background-color: #f9fafb; border-radius: 8px; border: 1px solid #eee;");
      QHBoxLayout *itemSelectLayout = new QHBoxLayout(itemSelectArea);

      QComboBox *prodCombo = new QComboBox();
      prodCombo->setPlaceholderText("Select Product...");
      prodCombo->setStyleSheet(inputStyle + " min-width: 200px;");
      // Populate Products
      QSqlQuery prodQuery;
      prodQuery.exec("SELECT ID_CONTENAIR, REF, PRIX_UNITAIRE FROM PRODUIT");
      while(prodQuery.next()) {
          prodCombo->addItem(prodQuery.value(1).toString() + " (" + QString::number(prodQuery.value(2).toDouble(), 'f', 2) + " TND)",
                             QVariantList() << prodQuery.value(0).toInt() << prodQuery.value(2).toDouble() << prodQuery.value(1).toString());
      }

      QSpinBox *qtySpin = new QSpinBox();
      qtySpin->setRange(1, 10000);
      qtySpin->setStyleSheet(inputStyle + " min-width: 80px;");

      QPushButton *btnAddItem = new QPushButton("Add Item");
      btnAddItem->setStyleSheet(getTabButtonStyle() + " background-color: #e8fdf2; color: #27ae60; border-color: #3DDC84; font-weight: 700;");
      btnAddItem->setCursor(Qt::PointingHandCursor);

      itemSelectLayout->addWidget(new QLabel("Product:"));
      itemSelectLayout->addWidget(prodCombo);
      itemSelectLayout->addWidget(new QLabel("Qty:"));
      itemSelectLayout->addWidget(qtySpin);
      itemSelectLayout->addWidget(btnAddItem);
      formLayout->addWidget(itemSelectArea);

      // Temporary items table for the current order
      QTableWidget *itemsTable = new QTableWidget(0, 4);
      itemsTable->setHorizontalHeaderLabels({"Ref", "Qty", "Price", "Total"});
      itemsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      itemsTable->setFixedHeight(150);
      itemsTable->setStyleSheet("QTableWidget { background-color: #fff; border: 1px solid #ddd; border-radius: 4px; }");
      formLayout->addWidget(itemsTable);

      // List to store IDs and data for final save
      struct PendingItem { int prodId; QString ref; int qty; double price; };
      QList<PendingItem> *pendingItems = new QList<PendingItem>();

      QObject::connect(btnAddItem, &QPushButton::clicked, [=]() {
          if (prodCombo->currentIndex() == -1) return;
          QVariantList data = prodCombo->currentData().toList();
          int pid = data[0].toInt();
          double price = data[1].toDouble();
          QString ref = data[2].toString();
          int qty = qtySpin->value();

          pendingItems->append({pid, ref, qty, price});

          int row = itemsTable->rowCount();
          itemsTable->insertRow(row);
          itemsTable->setItem(row, 0, new QTableWidgetItem(ref));
          itemsTable->setItem(row, 1, new QTableWidgetItem(QString::number(qty)));
          itemsTable->setItem(row, 2, new QTableWidgetItem(QString::number(price, 'f', 2)));
          itemsTable->setItem(row, 3, new QTableWidgetItem(QString::number(price * qty, 'f', 2)));
      });

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
          int id = 0; // ID will be auto-generated
          QString ref = static_cast<QLineEdit*>(wRef)->text().trimmed();
          QDate date = static_cast<QDateEdit*>(wDate)->date();
          QString client = static_cast<QLineEdit*>(wClient)->text().trimmed();
          QString address = static_cast<QLineEdit*>(wAddress)->text().trimmed();
          QString phone = static_cast<QLineEdit*>(wPhone)->text().trimmed();
          QString delivStatus = wDeliveryStatus->currentText();
          QDate delivery = static_cast<QDateEdit*>(wDelivery)->date();
          QString state = pendingRadio->isChecked() ? "Pending" : "Completed";

          // Validate Reference: must be exactly 2 letters then 1+ digits
          QRegularExpression refRegex("^[A-Za-z]{2}[0-9]+$");
          if (ref.isEmpty()) {
              QMessageBox::warning(nullptr, "Validation Error", "Reference cannot be empty.");
              return;
          }
          if (!refRegex.match(ref).hasMatch()) {
              QMessageBox::warning(nullptr, "Validation Error",
                  "Reference must start with exactly 2 letters followed by at least 1 digit.\n"
                  "Example: AB12, OP999");
              return;
          }
          if (client.isEmpty()) {
              QMessageBox::warning(nullptr, "Validation Error", "Client's Name cannot be empty.");
              return;
          }
          if (address.isEmpty()) {
              QMessageBox::warning(nullptr, "Validation Error", "Client's Address cannot be empty.");
              return;
          }
          if (delivery < date) {
              QMessageBox::warning(nullptr, "Validation Error", "Delivery date must be on or after the order date.");
              return;
          }
          if (phone.isEmpty()) {
              QMessageBox::warning(nullptr, "Validation Error", "Client's Phone Number cannot be empty.");
              return;
          }
          if (pendingItems->isEmpty()) {
              QMessageBox::warning(nullptr, "Validation Error", "Please add at least one product to the order.");
              return;
          }

          Commande c(id, ref, date, state, client, address, delivery, phone, delivStatus);
          if (c.ajouter()) {
              // Now we need to get the ID of the command we just added
              // Commande::ajouter() uses MAX(ID)+1, so let's fetch it
              QSqlQuery idQ;
              idQ.exec("SELECT MAX(ID_COMMANDE) FROM COMMANDE");
              int newOid = 0;
              if (idQ.next()) newOid = idQ.value(0).toInt();

              // Insert into CONTENIR
              for (const auto &item : *pendingItems) {
                  QSqlQuery itemQ;
                  itemQ.prepare("INSERT INTO CONTENIR (ID_CONTENAIR, ID_COMMANDE, QUANTITE_DEMANDEE) VALUES (:pid, :oid, :qty)");
                  itemQ.bindValue(":pid", item.prodId);
                  itemQ.bindValue(":oid", newOid);
                  itemQ.bindValue(":qty", item.qty);
                  if (!itemQ.exec()) {
                      qDebug() << "Failed to add item to order:" << itemQ.lastError().text();
                  }
              }


              // Capture phone again to be 100% safe
              QString phoneForSms = static_cast<QLineEdit*>(wPhone)->text().trimmed();

              // Calculate total for SMS
              double totalAmount = 0;
              for (const auto &item : *pendingItems) {
                  totalAmount += item.price * item.qty;
              }

              // Clear state
              static_cast<QLineEdit*>(wRef)->clear();
              static_cast<QLineEdit*>(wClient)->clear();
              static_cast<QLineEdit*>(wAddress)->clear();
              static_cast<QLineEdit*>(wPhone)->clear();
              wDeliveryStatus->setCurrentIndex(0);
              static_cast<QDateEdit*>(wDate)->setDate(QDate::currentDate());
              static_cast<QDateEdit*>(wDelivery)->setDate(QDate::currentDate());
              pendingItems->clear();
              itemsTable->setRowCount(0);

              // Automatically refresh and switch to Order Hub
              QPushButton *refreshBtn = outNestedStack->findChild<QPushButton*>("orderHubRefreshBtn");
              if (refreshBtn) refreshBtn->click();

              // NEW: Automatically refresh Analytics chart
              QPushButton *anaRefreshBtn = outNestedStack->findChild<QPushButton*>("analyticsRefreshBtn");
              if (anaRefreshBtn) anaRefreshBtn->click();

              outNestedStack->setCurrentIndex(1); // Order Hub is index 1
              for(QPushButton *btn : tabButtons) {
                  btn->setChecked(btn->text() == "Order Hub");
              }

              // Fire SMS Confirmation
              SmsAPI *sms = new SmsAPI(outNestedStack);

              // Connect signal for feedback
              QObject::connect(sms, &SmsAPI::smsSent, [=](bool success, const QString &msg){
                  if (!success) {
                      QMessageBox::warning(nullptr, "SMS Notification",
                          "The order was registered, but the SMS notification could not be sent.\n\nDetails: " + msg);
                  } else {
                      QMessageBox::information(nullptr, "SMS Notification", "SMS sent successfully to the client!");
                  }
                  sms->deleteLater();
              });

              QString txtMsg = QString("Order confirmed successfuly! Client: %1, Ref: %2, Total: %3 TND")
                                  .arg(client, ref, QString::number(totalAmount, 'f', 2));
              sms->sendSMS(phoneForSms, txtMsg);

          } else {
              QMessageBox::critical(nullptr, "Error", "Failed to add order.\n\nDB Error: " + c.getLastError());
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
      searchEdit->setPlaceholderText("Search by Client Name or Reference...");
      searchEdit->setStyleSheet(getInputStyle());
      searchEdit->setMinimumWidth(260);

      // Sort
      QLabel *lblSort = new QLabel("Sort by:");
      lblSort->setStyleSheet(getLabelStyle());
      lblSort->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

      QComboBox *sortCombo = new QComboBox();
      sortCombo->addItems({"Date (Newest First)", "Date (Oldest First)",
                           "Client Name (A → Z)", "Client Name (Z → A)"});
      sortCombo->setStyleSheet(getInputStyle());
      sortCombo->setFixedWidth(200);


      // Refresh Button
      QPushButton *btnRefresh = new QPushButton("Refresh");
      btnRefresh->setObjectName("orderHubRefreshBtn");
      btnRefresh->setCursor(Qt::PointingHandCursor);
      btnRefresh->setStyleSheet(getButtonStyle());
      btnRefresh->setFixedWidth(100);

      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(15);
      controlLayout->addWidget(lblSort);
      controlLayout->addWidget(sortCombo);
      controlLayout->addStretch();
      controlLayout->addWidget(btnRefresh);

      histLayout->addWidget(controlBar);

      // 2. Table
      QStringList headers = {"Reference", "Date", "State", "Client", "Address", "Delivery", "Actions"};
      // Note: we store the hidden orderId in column 7 (index 7) and show actions in col 6
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


      // 3. Data & Logic
      auto updateTable = [table, searchEdit, sortCombo, btnRefresh]() {
        Commande c;
        QSqlQueryModel *model = c.afficher();
        if (!model) return;

        QString query = searchEdit->text().toLower().trimmed();
        QString sortOpt = sortCombo->currentText();

        // Collect all rows first
        struct Row {
            QString id, ref;
            QDateTime date;
            QString etat, client, address;
            QDateTime livraison;
            QString delivStatus, telephone;
        };
        QVector<Row> rows;
        for (int i = 0; i < model->rowCount(); ++i) {
          Row row;
          row.id       = model->record(i).value("ID_COMMANDE").toString();
          row.ref      = model->record(i).value("REFERENCE").toString();
          row.date     = model->record(i).value("DATE_COMMANDE").toDateTime();
          row.etat     = model->record(i).value("ETAT_COMMANDE").toString();
          row.client   = model->record(i).value("NOM_CLIENT").toString();
          row.address  = model->record(i).value("ADRESSE_CLIENT").toString();
          row.livraison= model->record(i).value("DATE_LIVRAISON").toDateTime();
          row.delivStatus = model->record(i).value("DELIVERY_STATUS").toString();
          row.telephone= model->record(i).value("NUMERO_TELEPHONE").toString();

          // Filter: only by client name OR reference
          if (!query.isEmpty()) {
            bool matchClient = row.client.toLower().contains(query);
            bool matchRef    = row.ref.toLower().contains(query);
            if (!matchClient && !matchRef) continue;
          }
          rows.append(row);
        }

        // Sort
        int sortIdx = sortCombo->currentIndex();
        if (sortIdx == 0) { // Date (Newest First)
          std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b){
            return a.date > b.date;
          });
        } else if (sortIdx == 1) { // Date (Oldest First)
          std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b){
            return a.date < b.date;
          });
        } else if (sortIdx == 2) { // Client Name (A -> Z)
          std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b){
            return a.client.toLower() < b.client.toLower();
          });
        } else if (sortIdx == 3) { // Client Name (Z -> A)
          std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b){
            return a.client.toLower() > b.client.toLower();
          });
        }

        table->setRowCount(0);
        int rowIdx = 0;
        for (const Row &row : rows) {
          QString id       = row.id;
          QString ref      = row.ref;
          QString date     = row.date.toString("yyyy-MM-dd");
          QString etat     = row.etat;
          QString client   = row.client;
          QString address  = row.address;
          QString livraison= row.livraison.toString("yyyy-MM-dd");
          QString delivStatus = row.delivStatus;
          QString phone    = row.telephone;

          table->insertRow(rowIdx);
          table->setItem(rowIdx, 0, new QTableWidgetItem(ref));
          table->setItem(rowIdx, 1, new QTableWidgetItem(date));
          table->setItem(rowIdx, 2, new QTableWidgetItem(etat));
          table->setItem(rowIdx, 3, new QTableWidgetItem(client));
          table->setItem(rowIdx, 4, new QTableWidgetItem(address));
          table->setItem(rowIdx, 5, new QTableWidgetItem(livraison));

          // Action Column
          QWidget *actionWidget = new QWidget();
          QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
          actionLayout->setContentsMargins(5, 2, 5, 2);
          actionLayout->setSpacing(5);

          QPushButton *btnModify = new QPushButton("Edit");
          btnModify->setCursor(Qt::PointingHandCursor);
          btnModify->setMinimumWidth(60);
          btnModify->setFixedHeight(28);
          btnModify->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#cccccc; border-radius: 6px; padding: 0px 8px; font-weight: "
              "normal; font-size: 13px; color: #0066cc; } QPushButton:hover { "
              "border-color: #aaaaaa; color: #004c99; background-color: "
              "#f6f6f6; }");

          QPushButton *btnDelete = new QPushButton("Remove");
          btnDelete->setCursor(Qt::PointingHandCursor);
          btnDelete->setMinimumWidth(60);
          btnDelete->setFixedHeight(28);
          btnDelete->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#d32f2f; color: #d32f2f; border-radius: 6px; padding: 0px 8px; "
              "font-weight: 600; font-size: 13px; } QPushButton:hover { "
              "background-color: #ffebee; border-color: #b71c1c; color: "
              "#b71c1c; }");

          // ── PDF Invoice button ──────────────────────────────────────────
          QPushButton *btnInvoice = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\x84 Invoice"));
          btnInvoice->setCursor(Qt::PointingHandCursor);
          btnInvoice->setMinimumWidth(80);
          btnInvoice->setFixedHeight(28);
          btnInvoice->setToolTip("Generate PDF invoice for this order");
          btnInvoice->setStyleSheet(
              "QPushButton { background-color: #2C3E1F; color: #ffffff; "
              "border: none; border-radius: 6px; padding: 0px 10px; "
              "font-weight: 700; font-size: 12px; } "
              "QPushButton:hover { background-color: #3a5228; } "
              "QPushButton:pressed { background-color: #1e2b15; }");

          QPushButton *btnTrack = new QPushButton("Track");
          btnTrack->setCursor(Qt::PointingHandCursor);
          btnTrack->setMinimumWidth(60);
          btnTrack->setFixedHeight(28);
          btnTrack->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#007acc; border-radius: 6px; padding: 0px 8px; font-weight: "
              "bold; font-size: 13px; color: #007acc; } QPushButton:hover { "
              "border-color: #005999; color: #ffffff; background-color: "
              "#007acc; }");

          QPushButton *btnQuickSMS = new QPushButton("SMS");
          btnQuickSMS->setCursor(Qt::PointingHandCursor);
          btnQuickSMS->setMinimumWidth(50);
          btnQuickSMS->setFixedHeight(28);
          btnQuickSMS->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#9b59b6; color: #9b59b6; border-radius: 6px; padding: 0px 8px; "
              "font-weight: 600; font-size: 13px; } QPushButton:hover { "
              "background-color: #f5eef8; border-color: #8e44ad; color: "
              "#8e44ad; }");

          actionLayout->addWidget(btnInvoice);
          actionLayout->addWidget(btnTrack);
          actionLayout->addWidget(btnQuickSMS);
          actionLayout->addWidget(btnModify);
          actionLayout->addWidget(btnDelete);
          table->setCellWidget(rowIdx, 6, actionWidget);

          int orderId = id.toInt();

          // Connect Quick SMS action
          QObject::connect(btnQuickSMS, &QPushButton::clicked, [table, phone, client, ref]() {
              QDialog *smsDlg = new QDialog(table->window());
              smsDlg->setWindowTitle("Send Manual SMS — " + client);
              smsDlg->setFixedSize(400, 300);
              smsDlg->setStyleSheet("QDialog { background-color: #ffffff; }");

              QVBoxLayout *vbox = new QVBoxLayout(smsDlg);
              vbox->setContentsMargins(25, 25, 25, 20);

              QLabel *title = new QLabel("Quick Message to Client");
              title->setStyleSheet("font-size: 18px; font-weight: 800; color: #1a1a1a;");
              vbox->addWidget(title);

              QLabel *info = new QLabel("Sending to: " + phone);
              info->setStyleSheet("font-size: 13px; color: #666; margin-bottom: 10px;");
              vbox->addWidget(info);

              QTextEdit *msgEdit = new QTextEdit();
              msgEdit->setPlaceholderText("Enter your message here...");
              msgEdit->setStyleSheet("QTextEdit { background-color: #f9fafb; border: 1px solid #eaeaea; "
                                    "border-radius: 8px; padding: 10px; font-size: 13px; color: #333; } "
                                    "QTextEdit:focus { border-color: #9b59b6; background-color: #ffffff; }");
              vbox->addWidget(msgEdit);

              QHBoxLayout *btnBox = new QHBoxLayout();
              QPushButton *cancelBtn = new QPushButton("Cancel");
              cancelBtn->setStyleSheet(getTabButtonStyle());
              QPushButton *sendBtn = new QPushButton("Send SMS");
              sendBtn->setStyleSheet("QPushButton { background-color: #9b59b6; color: white; border: none; "
                                    "border-radius: 8px; padding: 8px 16px; font-weight: 700; } "
                                    "QPushButton:hover { background-color: #8e44ad; }");

              btnBox->addStretch();
              btnBox->addWidget(cancelBtn);
              btnBox->addWidget(sendBtn);
              vbox->addLayout(btnBox);

              QObject::connect(cancelBtn, &QPushButton::clicked, smsDlg, &QDialog::reject);
              QObject::connect(sendBtn, &QPushButton::clicked, [smsDlg, msgEdit, phone, table]() {
                  QString text = msgEdit->toPlainText().trimmed();
                  if (text.isEmpty()) {
                      QMessageBox::warning(smsDlg, "Error", "Message cannot be empty.");
                      return;
                  }

                  SmsAPI *sms = new SmsAPI(table->window());
                  sms->sendSMS(phone, text);

                  QObject::connect(sms, &SmsAPI::smsSent, [=](bool success, const QString &res) {
                      if (success) {
                          QMessageBox::information(nullptr, "SMS Status", "Message sent successfully!");
                      } else {
                          QMessageBox::critical(nullptr, "SMS Status", "Failed to send: " + res);
                      }
                      sms->deleteLater();
                  });
                  smsDlg->accept();
              });

              smsDlg->exec();
              smsDlg->deleteLater();
          });

          // Connect Track action — delivery stage progress + real map route
          QObject::connect(btnTrack, &QPushButton::clicked, [table, delivStatus, ref, client, livraison, address]() {
              QDialog *trackDlg = new QDialog(table->window());
              trackDlg->setWindowTitle("Delivery Tracking — " + ref);
              trackDlg->setFixedSize(500, 420);
              trackDlg->setStyleSheet("QDialog { background-color: #f7f9fb; }");
              QVBoxLayout *vbox = new QVBoxLayout(trackDlg);
              vbox->setContentsMargins(30, 25, 30, 20);
              vbox->setSpacing(14);

              QLabel *title = new QLabel("Delivery Tracking — " + ref);
              title->setStyleSheet("font-size: 18px; font-weight: 800; color: #1a1a1a;");
              vbox->addWidget(title);

              QLabel *sub = new QLabel("Client: " + client + "  |  Expected: " + livraison);
              sub->setStyleSheet("font-size: 13px; color: #666;");
              vbox->addWidget(sub);

              QLabel *addrLbl = new QLabel("Delivery to: " + address);
              addrLbl->setStyleSheet("font-size: 12px; color: #888; font-style: italic;");
              addrLbl->setWordWrap(true);
              vbox->addWidget(addrLbl);

              // 4-stage visual progress bar
              const QStringList stages = {"Preparing", "Dispatched", "In Transit", "Delivered"};
              int currentStage = stages.indexOf(delivStatus);
              if (currentStage < 0) currentStage = 0;

              QWidget *stepsWidget = new QWidget();
              stepsWidget->setStyleSheet("background: transparent;");
              QHBoxLayout *stepsLayout = new QHBoxLayout(stepsWidget);
              stepsLayout->setSpacing(0);
              stepsLayout->setContentsMargins(0, 10, 0, 10);

              const QStringList icons = {"\xF0\x9F\x93\xA6", "\xF0\x9F\x9A\x9A", "\xF0\x9F\x9B\xA3", "\xE2\x9C\x85"};

              for (int s = 0; s < stages.size(); ++s) {
                  QLabel *circle = new QLabel(QString::fromUtf8(icons[s].toUtf8()));
                  circle->setAlignment(Qt::AlignCenter);
                  circle->setFixedSize(54, 54);
                  bool isDone = (s <= currentStage);
                  circle->setStyleSheet(QString(
                      "QLabel { background-color: %1; border-radius: 27px; "
                      "font-size: 20px; border: 3px solid %2; }")
                      .arg(isDone ? "#3DDC84" : "#e0e0e0")
                      .arg(isDone ? "#2DB66F" : "#cccccc"));

                  QLabel *stageLbl = new QLabel(stages[s]);
                  stageLbl->setAlignment(Qt::AlignCenter);
                  stageLbl->setStyleSheet(
                      QString("font-size: 11px; font-weight: %1; color: %2; margin-top: 3px;")
                      .arg(s == currentStage ? "800" : "500")
                      .arg(s == currentStage ? "#2DB66F" : "#888"));

                  QVBoxLayout *stepCol = new QVBoxLayout();
                  stepCol->addWidget(circle, 0, Qt::AlignHCenter);
                  stepCol->addWidget(stageLbl, 0, Qt::AlignHCenter);
                  stepsLayout->addLayout(stepCol);

                  if (s < stages.size() - 1) {
                      QLabel *line = new QLabel();
                      line->setFixedHeight(4);
                      line->setStyleSheet(QString("background-color: %1; border-radius: 2px; margin-bottom: 28px;")
                          .arg(s < currentStage ? "#3DDC84" : "#e0e0e0"));
                      stepsLayout->addWidget(line, 1);
                  }
              }
              vbox->addWidget(stepsWidget);

              QLabel *currentLbl = new QLabel("Current Stage:  " + delivStatus);
              currentLbl->setStyleSheet("font-size: 14px; font-weight: 700; color: #2DB66F; padding: 4px 0;");
              vbox->addWidget(currentLbl);

              vbox->addStretch();

              // Map button — opens Google Maps with the real delivery route
              QPushButton *mapBtn = new QPushButton("  View Delivery Route on Map");
              mapBtn->setFixedHeight(42);
              mapBtn->setCursor(Qt::PointingHandCursor);
              mapBtn->setStyleSheet(
                  "QPushButton { background-color: #4285F4; color: white; border: none; "
                  "border-radius: 8px; font-size: 14px; font-weight: 700; } "
                  "QPushButton:hover { background-color: #3367d6; } "
                  "QPushButton:pressed { background-color: #2a56c6; }");
              // The origin is your company/warehouse in Tunisia
              const QString origin = "El Ghazela, Ariana, Tunisie"; // Change to your actual address
              QObject::connect(mapBtn, &QPushButton::clicked, [origin, address]() {
                  TrackingAPI::openDeliveryRouteInBrowser(origin, address);
              });
              vbox->addWidget(mapBtn);

              QPushButton *closeBtn = new QPushButton("Close");
              closeBtn->setFixedHeight(38);
              closeBtn->setCursor(Qt::PointingHandCursor);
              closeBtn->setStyleSheet(
                  "QPushButton { background-color: #ffffff; color: #555; border: 1px solid #ddd; "
                  "border-radius: 8px; font-size: 13px; font-weight: 600; } "
                  "QPushButton:hover { border-color: #bbb; color: #333; }");
              vbox->addWidget(closeBtn);
              QObject::connect(closeBtn, &QPushButton::clicked, trackDlg, &QDialog::accept);
              trackDlg->exec();
              trackDlg->deleteLater();
          });


          // Connect Invoice PDF
          QObject::connect(btnInvoice, &QPushButton::clicked,
              [table, orderId, ref, date, livraison, etat, client, address]() {
                  generateOrderInvoicePdf(
                      table->window(),
                      orderId, ref, date, livraison, etat, client, address);
              });

          // Connect Modify
          QObject::connect(btnModify, &QPushButton::clicked, [table, orderId, ref, date, etat, client, address, livraison, delivStatus, phone, btnRefresh]() {
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
                form->setSpacing(10);
                form->setRowWrapPolicy(QFormLayout::WrapAllRows);
                form->setLabelAlignment(Qt::AlignLeft);

                // Fields
                QLineEdit *refEdit = new QLineEdit(ref);
                refEdit->setReadOnly(true);  // Reference cannot be changed after creation
                QDateEdit *dateEdit = new QDateEdit(QDate::fromString(date, "yyyy-MM-dd"));
                dateEdit->setDisplayFormat("yyyy-MM-dd");
                dateEdit->setCalendarPopup(true);
                dateEdit->setMinimumDate(QDate::currentDate().addDays(-7));
                dateEdit->setMaximumDate(QDate::currentDate());
                QLineEdit *clientEdit = new QLineEdit(client);
                clientEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z \\.]*$"), &dlg));
                QLineEdit *addressEdit = new QLineEdit(address);
                // Fetch current quantity from CONTENIR
                int currentQty = 0;
                QSqlQuery qtyQ;
                qtyQ.prepare("SELECT QUANTITE_DEMANDEE FROM CONTENIR WHERE ID_COMMANDE = :id");
                qtyQ.bindValue(":id", orderId);
                if (qtyQ.exec() && qtyQ.next()) currentQty = qtyQ.value(0).toInt();

                QDateEdit *livraisonEdit = new QDateEdit(QDate::fromString(livraison, "yyyy-MM-dd"));
                livraisonEdit->setDisplayFormat("yyyy-MM-dd");
                livraisonEdit->setCalendarPopup(true);

                QLineEdit *phoneEdit = new QLineEdit(phone);
                QSpinBox *qtyEdit = new QSpinBox();
                qtyEdit->setRange(1, 10000);
                qtyEdit->setValue(currentQty);

                auto styleField = [&](QWidget *w) {
                  w->setStyleSheet(
                      "QLineEdit, QDateEdit, QComboBox { background-color: #f9fafb; border: 1px "
                      "solid #eaeaea; border-radius: 6px; padding: 6px 10px; "
                      "font-size: 13px; color: #333; } QLineEdit:focus, QDateEdit:focus, QComboBox:focus { "
                      "border-color: #3DDC84; background-color: #ffffff; }");
                  w->setFixedHeight(35);
                };
                auto styleReadOnly = [&](QLineEdit *w) {
                  w->setStyleSheet(
                      "QLineEdit { background-color: #f0f0f0; border: 1px solid #d0d0d0; "
                      "border-radius: 6px; padding: 6px 10px; font-size: 13px; color: #888; "
                      "font-style: italic; }");
                  w->setFixedHeight(35);
                };

                styleReadOnly(refEdit);
                styleField(dateEdit);
                styleField(clientEdit);
                styleField(addressEdit);
                styleField(livraisonEdit);
                styleField(phoneEdit);
                styleField(qtyEdit);

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

                addRow("Reference:", refEdit);
                addRow("Order Date:", dateEdit);
                addRow("Client Name:", clientEdit);
                addRow("Address:", addressEdit);
                addRow("Phone:", phoneEdit);
                addRow("Quantity:", qtyEdit);
                addRow("Delivery Date:", livraisonEdit);
                addRow("State:", radioWidget);

                // Delivery Status dropdown
                QComboBox *delivStatusEdit = new QComboBox();
                delivStatusEdit->addItems({"Preparing", "Dispatched", "In Transit", "Delivered"});
                int dsIdx = delivStatusEdit->findText(delivStatus);
                if (dsIdx >= 0) delivStatusEdit->setCurrentIndex(dsIdx);
                styleField(delivStatusEdit);
                addRow("Delivery Status:", delivStatusEdit);

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

                QObject::connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
                // Validate before accepting
                QObject::connect(bbox, &QDialogButtonBox::accepted, [&]() {
                    if (clientEdit->text().trimmed().isEmpty()) {
                        QMessageBox::warning(&dlg, "Validation Error", "Client's Name cannot be empty.");
                        return;
                    }
                    if (addressEdit->text().trimmed().isEmpty()) {
                        QMessageBox::warning(&dlg, "Validation Error", "Client's Address cannot be empty.");
                        return;
                    }
                    if (livraisonEdit->date() < dateEdit->date()) {
                        QMessageBox::warning(&dlg, "Validation Error", "Delivery date must be on or after the order date.");
                        return;
                    }
                    dlg.accept();
                });

                if (dlg.exec() == QDialog::Accepted) {
                    QString newState = pendingRadio->isChecked() ? "Pending" : "Completed";
                    QString newDelivStatus = delivStatusEdit->currentText();
                    QString newPhone = phoneEdit->text().trimmed();
                    int newQty = qtyEdit->value();

                    // Update quantity in CONTENIR
                    QSqlQuery updQty;
                    updQty.prepare("UPDATE CONTENIR SET QUANTITE_DEMANDEE = :qty WHERE ID_COMMANDE = :id");
                    updQty.bindValue(":qty", newQty);
                    updQty.bindValue(":id", orderId);
                    updQty.exec();

                    Commande c(orderId, refEdit->text().trimmed(), dateEdit->date(), newState, clientEdit->text().trimmed(), addressEdit->text().trimmed(), livraisonEdit->date(), newPhone, newDelivStatus);
                    if (c.modifier()) {
                        QMessageBox::information(table->window(), "Success", "Order updated successfully!");

                        // Automated SMS on status change
                        if (newDelivStatus != delivStatus) {
                            SmsAPI *sms = new SmsAPI(table->window());
                            QString statusMsg = QString("Hello %1! Your order %2 status has been updated to: %3.")
                                                    .arg(client, ref, newDelivStatus);
                            sms->sendSMS(phone, statusMsg);

                            // Non-blocking feedback
                            QObject::connect(sms, &SmsAPI::smsSent, [=](bool success, const QString &msg){
                                if (success) {
                                    qDebug() << "Status Update SMS sent!";
                                } else {
                                    qDebug() << "Status Update SMS failed:" << msg;
                                }
                                sms->deleteLater();
                            });
                        }

                        // Refresh table
                        if (btnRefresh) btnRefresh->click();
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
    } else if (name == "Analytics") {
      // Bar Chart Analytics
      QWidget *analyticsHeader = new QWidget();
      QHBoxLayout *headerLayout = new QHBoxLayout(analyticsHeader);
      headerLayout->setContentsMargins(0, 0, 0, 10);

      QLabel *anaTitle = new QLabel("Monthly Delivery Analytics");
      anaTitle->setStyleSheet("font-size: 20px; font-weight: 700; color: #1a1a1a;");

      QPushButton *btnRefreshAna = new QPushButton("Refresh Data");
      btnRefreshAna->setObjectName("analyticsRefreshBtn");
      btnRefreshAna->setCursor(Qt::PointingHandCursor);
      btnRefreshAna->setFixedWidth(140);
      btnRefreshAna->setStyleSheet(getButtonStyle());

      headerLayout->addWidget(anaTitle);
      headerLayout->addStretch();
      headerLayout->addWidget(btnRefreshAna);
      cLayout->addWidget(analyticsHeader);

      GenericBarChart *chart = new GenericBarChart("Deliveries per Month");

      auto updateAnalytics = [chart]() {
          chart->clearBars();
          QSqlQuery chartQuery;
          // Fetch count of orders per month, sorted chronologically
          chartQuery.prepare("SELECT TO_CHAR(DATE_LIVRAISON, 'Mon YYYY'), COUNT(*) "
                             "FROM COMMANDE "
                             "GROUP BY TO_CHAR(DATE_LIVRAISON, 'Mon YYYY'), TRUNC(DATE_LIVRAISON, 'MM') "
                             "ORDER BY TRUNC(DATE_LIVRAISON, 'MM') ASC");

          QColor colors[] = { QColor(52, 152, 219), QColor(61, 220, 132), QColor(155, 89, 182), QColor(241, 196, 15), QColor(230, 126, 34), QColor(26, 188, 156) };
          int colorIdx = 0;

          if (chartQuery.exec()) {
              bool hasData = false;
              while (chartQuery.next()) {
                  hasData = true;
                  QString monthStr = chartQuery.value(0).toString();
                  int count = chartQuery.value(1).toInt();
                  chart->addBar(monthStr, count, colors[colorIdx % 6]);
                  colorIdx++;
              }
              if (!hasData) {
                  chart->addBar("No Data Available", 0, QColor(189, 195, 199));
              }
          } else {
              chart->addBar("Database Error", 0, QColor(231, 76, 60));
          }
      };

      QObject::connect(btnRefreshAna, &QPushButton::clicked, updateAnalytics);

      updateAnalytics();

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
          ".QWidget {"
          "  background-color: #ffffff;"
          "  border-radius: 12px;"
          "}");

      QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
      outerLayout->setContentsMargins(40, 35, 40, 35);
      outerLayout->setSpacing(0);

      // Title
      // Title with Scan Button
      QHBoxLayout *titleRow = new QHBoxLayout();
      QLabel *titleLabel = new QLabel("New Transaction");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a;");
      
      QPushButton *btnScanInvoice = new QPushButton("Scan Invoice Image");
      btnScanInvoice->setStyleSheet("QPushButton { background-color: #2C3E1F; color: white; border: none; border-radius: 8px; font-size: 13px; font-weight: 700; padding: 10px 15px; } QPushButton:hover { background-color: #3DDC84; }");
      btnScanInvoice->setCursor(Qt::PointingHandCursor);
      btnScanInvoice->setFixedWidth(180);

      titleRow->addWidget(titleLabel);
      titleRow->addStretch();
      titleRow->addWidget(btnScanInvoice);
      outerLayout->addLayout(titleRow);
      outerLayout->addSpacing(18);

      // Field style — explicitly covers QLineEdit, QDateEdit AND QComboBox
      QString fieldStyle =
          "QLineEdit, QDateEdit, QComboBox {"
          "  background-color: #f8f9fb;"
          "  border: 1.5px solid #d0d5dd;"
          "  border-radius: 6px;"
          "  padding: 6px 10px;"
          "  font-size: 13px;"
          "  color: #1a1a1a;"
          "  min-height: 35px;"
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
          "margin-top: 5px; margin-bottom: 2px;";

      // Helper to add a label + widget pair
      auto createErrLabel = []() {
        QLabel *lbl = new QLabel("");
        lbl->setStyleSheet("color: #d32f2f; font-size: 11px; font-weight: 600; margin-top: -3px; margin-bottom: 2px;");
        lbl->hide();
        return lbl;
      };

      auto addField = [&](const QString &label, QWidget *w, QLabel *errLbl = nullptr) {
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet(lblStyle);
        w->setStyleSheet(fieldStyle);
        outerLayout->addWidget(lbl);
        outerLayout->addWidget(w);
        if (errLbl) {
            outerLayout->addWidget(errLbl);
        }
      };

      // --- Fields ---
      QLineEdit *inputMontant = new QLineEdit();
      inputMontant->setPlaceholderText("e.g. 1500.00");
      QLabel *errMontant = createErrLabel();
      addField("Amount (TND):", inputMontant, errMontant);

      QDateEdit *inputDate = new QDateEdit(QDate::currentDate());
      inputDate->setCalendarPopup(true);
      inputDate->setDisplayFormat("yyyy-MM-dd");
      QLabel *errDate = createErrLabel();
      addField("Transaction Date:", inputDate, errDate);

      QComboBox *inputType = new QComboBox();
      inputType->addItems({"Revenue", "Expense", "Refund", "Transfer"});
      addField("Transaction Type:", inputType);

      QComboBox *inputMode = new QComboBox();
      inputMode->addItems({"Cash", "Card", "Bank Transfer", "Check"});
      addField("Payment Mode:", inputMode);

      QLineEdit *inputDesc = new QLineEdit();
      inputDesc->setPlaceholderText("Enter description...");
      QLabel *errDesc = createErrLabel();
      addField("Description:", inputDesc, errDesc);

      QLineEdit *inputCommande = new QLineEdit();
      inputCommande->setPlaceholderText("0");
      QLabel *errCommande = createErrLabel();
      addField("Order ID:", inputCommande, errCommande);

      // --- Submit Button ---
      outerLayout->addSpacing(10);
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
           inputCommande, &transTable, errMontant, errDate, errCommande, errDesc]() {

            errMontant->hide();
            errDate->hide();
            errCommande->hide();
            errDesc->hide();

            bool isValid = true;
            double montant = 0;

            if (inputMontant->text().trimmed().isEmpty()) {
              errMontant->setText("Amount is required.");
              errMontant->show();
              isValid = false;
            } else {
              bool ok = false;
              montant = inputMontant->text().toDouble(&ok);
              if (!ok || montant < 0) {
                errMontant->setText("Amount must be a valid positive number.");
                errMontant->show();
                isValid = false;
              }
            }

            if (inputDate->date() > QDate::currentDate()) {
              errDate->setText("Transaction Date cannot be in the future.");
              errDate->show();
              isValid = false;
            }

            if (inputDesc->text().trimmed().isEmpty()) {
              errDesc->setText("Description is required.");
              errDesc->show();
              isValid = false;
            }

            if (!inputCommande->text().trimmed().isEmpty()) {
              bool cmdOk = false;
              int cmdId = inputCommande->text().toInt(&cmdOk);
              if (!cmdOk || cmdId < 0) {
                errCommande->setText("Order ID must be a valid positive number.");
                errCommande->show();
                isValid = false;
              }
            }

            if (!isValid) return;

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

      // --- Connect Scan Invoice ---
      QObject::connect(btnScanInvoice, &QPushButton::clicked, [=]() {
          QString fileName = QFileDialog::getOpenFileName(formContainer->window(), "Select Invoice File", "", "Invoice Files (*.png *.jpg *.jpeg *.bmp *.pdf);;All Files (*.*)");
          if (fileName.isEmpty()) return;

          OCRScannerAPI *scanner = new OCRScannerAPI(formContainer);
          
          // Disable button during scan
          btnScanInvoice->setEnabled(false);
          btnScanInvoice->setText("Scanning...");

          QObject::connect(scanner, &OCRScannerAPI::scanFinished, [=](bool success, double amount, const QString &dateStr, const QString &text) {
              btnScanInvoice->setEnabled(true);
              btnScanInvoice->setText("Scan Invoice Image");

              if (!success) {
                  QMessageBox::warning(formContainer->window(), "Scan Failed", "OCR Error: " + text);
              } else {
                  if (amount > 0) {
                      inputMontant->setText(QString::number(amount, 'f', 2));
                  }
                  
                  if (!dateStr.isEmpty()) {
                      QString normalizedDate = dateStr;
                      normalizedDate.replace(".", "-").replace("/", "-");
                      
                      QDate parsedDate = QDate::fromString(normalizedDate, "yyyy-MM-dd");
                      if (!parsedDate.isValid()) parsedDate = QDate::fromString(normalizedDate, "dd-MM-yyyy");
                      if (!parsedDate.isValid()) parsedDate = QDate::fromString(normalizedDate, "d-M-yyyy");
                      if (!parsedDate.isValid()) parsedDate = QDate::fromString(normalizedDate, "yyyy-M-d");
                      
                      if (parsedDate.isValid()) {
                          inputDate->setDate(parsedDate);
                      }
                  }
                  
                  if (amount > 0 || !dateStr.isEmpty()) {
                      QMessageBox::information(formContainer->window(), "Scan Complete", "Invoice data extracted successfully.");
                  } else {
                      // Diagnostic view to help user understand what the OCR saw
                      QDialog *diag = new QDialog(formContainer->window());
                      diag->setWindowTitle("Scan Results - No Data Found");
                      diag->setMinimumSize(400, 300);
                      QVBoxLayout *v = new QVBoxLayout(diag);
                      v->addWidget(new QLabel("The OCR engine read the text, but couldn't identify a Total or Date.\nRaw text detected:"));
                      QTextEdit *te = new QTextEdit(text);
                      te->setReadOnly(true);
                      te->setStyleSheet("font-family: monospace; font-size: 11px; background: #f4f4f4;");
                      v->addWidget(te);
                      QPushButton *btnOk = new QPushButton("Close");
                      btnOk->setStyleSheet(getButtonStyle());
                      QObject::connect(btnOk, &QPushButton::clicked, diag, &QDialog::accept);
                      v->addWidget(btnOk);
                      diag->show(); // Show non-modally so they can compare with image
                  }
              }
              scanner->deleteLater();
          });

          scanner->scanInvoice(fileName);
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

      QPushButton *btnScan = new QPushButton("ANOMALY SCAN");
      btnScan->setStyleSheet(getButtonStyle());
      btnScan->setCursor(Qt::PointingHandCursor);
      btnScan->setFixedWidth(160);

      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(15);
      controlLayout->addWidget(lblSort);
      controlLayout->addWidget(sortType);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnRefresh);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnScan);
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
      transTable->setColumnWidth(headers.size() - 1, 350);
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
          r.date = model->data(model->index(i, 2)).toDate().toString("yyyy-MM-dd");
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

          QPushButton *btnInvoice = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\x84 Invoice"));
          btnInvoice->setCursor(Qt::PointingHandCursor);
          btnInvoice->setMinimumWidth(80);
          btnInvoice->setFixedHeight(28);
          btnInvoice->setStyleSheet(
              "QPushButton { background-color: #2C3E1F; color: #ffffff; "
              "border: none; border-radius: 6px; padding: 0px 10px; "
              "font-weight: 700; font-size: 11px; } "
              "QPushButton:hover { background-color: #3DDC84; }");

          actionBtnLayout->addWidget(btnInvoice);
          actionBtnLayout->addWidget(btnModify);
          actionBtnLayout->addWidget(btnDelete);
          transTable->setCellWidget(i, 7, actionWidget);

          // --- ANOMALY DETECTION (METIER AVANCE) ---
          QString riskTip = "";
          int riskLevel = 0; // 0=OK, 1=Potential, 2=High Risk

          // Use methods from Transaction class to detect anomalies
          QDate transDate = QDate::fromString(r.date, "yyyy-MM-dd");

          QString dateErr = Transaction::checkDateAnomaly(transDate);
          if (!dateErr.isEmpty()) {
              riskTip += dateErr;
              riskLevel = 2; // Future date is high risk
          }

          QString amountErr = Transaction::checkAmountMismatch(r.commande, r.montant);
          if (!amountErr.isEmpty()) {
              riskTip += amountErr;
              riskLevel = 2; // Mismatch is high risk
          }

          QString dupErr = Transaction::checkDuplicateAnomaly(r.id, r.montant, transDate, r.desc);
          if (!dupErr.isEmpty()) {
              riskTip += dupErr;
              if (riskLevel < 1) riskLevel = 1; // Duplicate is potential issue
          }

          if (riskLevel > 0) {
              QLabel *riskIcon = new QLabel(riskLevel == 2 ? "⚠️" : "💡");
              riskIcon->setToolTip("Anomaly Detected:\n" + riskTip.trimmed());
              riskIcon->setCursor(Qt::WhatsThisCursor);
              riskIcon->setStyleSheet(riskLevel == 2 ? "color: #e74c3c; font-size: 16px; font-weight:bold; padding-right:5px;" : "color: #f39c12; font-size: 16px; padding-right:5px;");
              actionBtnLayout->insertWidget(0, riskIcon);
          }

          // --- Connect Invoice ---
          int orderIdForPdf = r.commande;
          QString descForPdf = r.desc;
          QString dateForPdf = r.date;
          QString typeForPdf = r.type;

          QObject::connect(btnInvoice, &QPushButton::clicked, [transTable, orderIdForPdf, descForPdf, dateForPdf, typeForPdf]() {
              // If there's an Order ID, we try to fetch associated order details
              QString client = "N/A", address = "N/A", ref = descForPdf, dateCmd = dateForPdf, dateLiv = "N/A", etat = typeForPdf;

              if (orderIdForPdf > 0) {
                  QSqlQuery q;
                  q.prepare("SELECT NOM_CLIENT, ADRESSE_CLIENT, REFERENCE, DATE_COMMANDE, DATE_LIVRAISON, ETAT_COMMANDE "
                            "FROM COMMANDE WHERE ID_COMMANDE = :id");
                  q.bindValue(":id", orderIdForPdf);
                  if (q.exec() && q.next()) {
                      client = q.value(0).toString();
                      address = q.value(1).toString();
                      ref = q.value(2).toString();
                      dateCmd = q.value(3).toDate().toString("yyyy-MM-dd");
                      dateLiv = q.value(4).toDate().toString("yyyy-MM-dd");
                      etat = q.value(5).toString();
                  }
              }

              generateOrderInvoicePdf(transTable->window(), orderIdForPdf, ref, dateCmd, dateLiv, etat, client, address);
          });

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
                form->setSpacing(10);
                form->setRowWrapPolicy(QFormLayout::WrapAllRows);
                form->setLabelAlignment(Qt::AlignLeft);

                auto styleField = [&](QWidget *w) {
                  w->setStyleSheet(
                      "QLineEdit, QDateEdit, QComboBox { background-color: #f9fafb; border: 1px "
                      "solid #eaeaea; border-radius: 6px; padding: 6px 10px; "
                      "font-size: 13px; color: #333; } "
                      "QLineEdit:focus, QDateEdit:focus, QComboBox:focus { "
                      "border-color: #3DDC84; background-color: #ffffff; }");
                  w->setFixedHeight(35);
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

                auto createErrLabelDlg = []() {
                  QLabel *lbl = new QLabel("");
                  lbl->setStyleSheet("color: #d32f2f; font-size: 11px; font-weight: 600; margin-top: -3px; background: transparent;");
                  lbl->hide();
                  return lbl;
                };

                QLabel *errMontant = createErrLabelDlg();
                QLabel *errDate = createErrLabelDlg();
                QLabel *errCommande = createErrLabelDlg();
                QLabel *errDesc = createErrLabelDlg();

                auto addRow = [&](const QString &label, QWidget *w, QLabel *errLbl = nullptr) {
                  QLabel *l = new QLabel(label);
                  l->setStyleSheet(
                      "font-weight: 600; color: #444; font-size: 13px;");
                  if (errLbl) {
                      QVBoxLayout *v = new QVBoxLayout();
                      v->setContentsMargins(0,0,0,0);
                      v->setSpacing(0);
                      v->addWidget(w);
                      v->addWidget(errLbl);
                      form->addRow(l, v);
                  } else {
                      form->addRow(l, w);
                  }
                };

                addRow("Amount:", editMontant, errMontant);
                addRow("Date:", editDate, errDate);
                addRow("Type:", editType);
                addRow("Payment Mode:", editMode);
                addRow("Description:", editDesc, errDesc);
                addRow("Order ID:", editCommande, errCommande);

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

                QObject::connect(bbox, &QDialogButtonBox::accepted, [&]() {
                  errMontant->hide();
                  errDate->hide();
                  errCommande->hide();
                  errDesc->hide();

                  bool isValid = true;
                  if (editMontant->text().trimmed().isEmpty()) {
                    errMontant->setText("Amount is required.");
                    errMontant->show();
                    isValid = false;
                  } else {
                    bool ok = false;
                    double montant = editMontant->text().toDouble(&ok);
                    if (!ok || montant < 0) {
                      errMontant->setText("Amount must be a valid positive number.");
                      errMontant->show();
                      isValid = false;
                    }
                  }

                  if (editDate->date() > QDate::currentDate()) {
                    errDate->setText("Transaction Date cannot be in the future.");
                    errDate->show();
                    isValid = false;
                  }

                  if (editDesc->text().trimmed().isEmpty()) {
                    errDesc->setText("Description is required.");
                    errDesc->show();
                    isValid = false;
                  }

                  if (!editCommande->text().trimmed().isEmpty()) {
                    bool cmdOk = false;
                    int cmdId = editCommande->text().toInt(&cmdOk);
                    if (!cmdOk || cmdId < 0) {
                      errCommande->setText("Order ID must be a valid positive number.");
                      errCommande->show();
                      isValid = false;
                    }
                  }

                  if (isValid) {
                      dlg.accept();
                  }
                });
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

      // Print PDF (Global Report)
      QObject::connect(btnPrint, &QPushButton::clicked, [transTable]() {
        QString fileName = QFileDialog::getSaveFileName(
            transTable->window(), "Export Financial Report", "financial_report.pdf",
            "PDF Files (*.pdf)");
        if (fileName.isEmpty()) return;

        double totalRev = 0, totalExp = 0;
        for (int r = 0; r < transTable->rowCount(); ++r) {
            double amt = transTable->item(r, 1) ? transTable->item(r, 1)->text().toDouble() : 0.0;
            QString type = transTable->item(r, 3) ? transTable->item(r, 3)->text().toUpper() : "";
            if (type == "REVENUE") totalRev += amt;
            else if (type == "EXPENSE") totalExp += amt;
        }

        QString html =
            "<html><body style='font-family: Arial, sans-serif; color: #1a1a1a;'>"
            "<div style='padding: 30pt;'>"
            "<table width='100%' cellpadding='0' cellspacing='0' style='margin-bottom: 25pt;'>"
            "  <tr><td style='font-size: 32pt; font-weight: bold;'>Financial Report</td><td align='right' style='font-size: 14pt; color: #666;'>Oil Press Manager Pro</td></tr>"
            "</table>"
            "<div style='background-color: #3DDC84; height: 8pt; width: 100%; margin-bottom: 30pt;'></div>"

            "<table width='100%' cellpadding='20' cellspacing='0' style='margin-bottom: 40pt;'>"
            "  <tr>"
            "    <td style='background: #f1f8f5; border: 1pt solid #d4edda; text-align: center;'>"
            "      <div style='font-size: 11pt; color: #555; text-transform: uppercase;'>Total Revenue</div>"
            "      <div style='font-size: 22pt; font-weight: 800; color: #27ae60;'>" + QString::number(totalRev, 'f', 2) + " TND</div>"
            "    </td>"
            "    <td style='background: #fff5f5; border: 1pt solid #fed7d7; text-align: center;'>"
            "      <div style='font-size: 11pt; color: #555; text-transform: uppercase;'>Total Expenses</div>"
            "      <div style='font-size: 22pt; font-weight: 800; color: #e74c3c;'>" + QString::number(totalExp, 'f', 2) + " TND</div>"
            "    </td>"
            "  </tr>"
            "</table>"

            "<table width='100%' cellpadding='12' cellspacing='0' style='border: 1pt solid #eee;'>"
            "  <tr style='background-color: #2C3E1F; color: white;'>"
            "    <td style='font-size: 11pt;'><b>Amount</b></td><td style='font-size: 11pt;'><b>Date</b></td><td style='font-size: 11pt;'><b>Type</b></td><td style='font-size: 11pt;'><b>Payment</b></td><td style='font-size: 11pt;'><b>Description</b></td>"
            "  </tr>";

        for (int r = 0; r < transTable->rowCount(); ++r) {
          html += "<tr>";
          for (int c = 1; c <= 5; ++c) {
            QString val = transTable->item(r, c) ? transTable->item(r, c)->text() : "";
            html += "<td style='border-bottom: 1pt solid #eee; font-size: 12pt;'>" + val + "</td>";
          }
          html += "</tr>";
        }
        html += "</table><div style='text-align: center; margin-top: 50pt; font-size: 10pt; color: #999;'>Oil Press Manager Professional Reporting Suite &copy; 2026</div></div></body></html>";

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        printer.setPageSize(QPageSize(QPageSize::A4));

        QTextDocument doc;
        doc.setHtml(html);
        doc.setPageSize(printer.pageRect(QPrinter::Point).size());
        doc.print(&printer);

        QMessageBox::information(nullptr, "Success", "Record Exported Successfully!");
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
      });

      // Connect Scan Button to External API
      QObject::connect(btnScan, &QPushButton::clicked, [transTable, refreshTable, btnScan]() {
          refreshTable(); // Auto-refresh to include latest entries before scanning
          
          btnScan->setText("SCANNING VIA API...");
          btnScan->setEnabled(false);
          btnScan->setStyleSheet("background-color: #f39c12; color: white; border-radius: 6px; font-weight: bold;");

          QJsonArray transactionsToScan;
          for (int i = 0; i < transTable->rowCount(); ++i) {
              QJsonObject tx;
              tx["id"] = transTable->item(i, 0)->text().toInt();
              tx["amount"] = transTable->item(i, 1)->text().toDouble();
              tx["date"] = transTable->item(i, 2)->text();
              tx["type"] = transTable->item(i, 3)->text();
              tx["payment_mode"] = transTable->item(i, 4)->text();
              tx["description"] = transTable->item(i, 5)->text();
              transactionsToScan.append(tx);
          }

          AnomalyAPI *api = new AnomalyAPI(btnScan);
          QObject::connect(api, &AnomalyAPI::scanFinished, [transTable, btnScan, api](const QJsonObject &report) {
              btnScan->setText("ANOMALY SCAN");
              btnScan->setEnabled(true);
              btnScan->setStyleSheet("QPushButton { background-color: #2C3E50; color: #ffffff; border: none; border-radius: 6px; padding: 10px 20px; font-weight: 700; font-size: 13px; } QPushButton:hover { background-color: #34495e; }");

              int totalScanned = report["total_scanned"].toInt();
              int totalAnomalies = report["anomalies_found"].toInt();
              int highRisk = report["high_risk"].toInt();
              QJsonArray results = report["results"].toArray();

              QString htmlReport = "<h3>Cloud API Anomaly Detection Report</h3><hr>";
              
              if (totalAnomalies > 0) {
                  for(const QJsonValue& val : results) {
                      QJsonObject res = val.toObject();
                      QString status = res["fraudlabspro_status"].toString();
                      if (status == "REVIEW" || status == "REJECT" || status == "ERROR" || status == "NETWORK_ERROR") {
                          QJsonObject tx = res["transaction"].toObject();
                          QString riskLevel = (res["fraudlabspro_score"].toInt() > 70 || status == "REJECT") ? "<span style='color:#e74c3c;font-weight:bold;'>HIGH RISK</span>" : "<span style='color:#f39c12;font-weight:bold;'>POTENTIAL RISK</span>";
                          htmlReport += QString("<p><b>Tx ID %1:</b> %2<br>%3<br><small style='color:#e74c3c'>Score: %4 - %5</small></p>")
                                          .arg(tx["id"].toInt())
                                          .arg(tx["description"].toString())
                                          .arg(riskLevel)
                                          .arg(res["fraudlabspro_score"].toInt())
                                          .arg(res["fraudlabspro_message"].toString());
                      }
                  }
              }

              QDialog *dlg = new QDialog(transTable->window());
              dlg->setWindowTitle("Advanced Anomaly Scan (FraudLabs Pro API)");
              dlg->setMinimumSize(500, 450);
              dlg->setStyleSheet("QDialog { background: white; border-radius: 12px; }");
              QVBoxLayout *layout = new QVBoxLayout(dlg);
              layout->setContentsMargins(25, 25, 25, 25);

              QLabel *title = new QLabel("Anomaly Radar Results");
              title->setStyleSheet("font-size: 18px; font-weight: 800; color: #2c3e50;");
              layout->addWidget(title);

              QTextEdit *area = new QTextEdit();
              area->setHtml(totalAnomalies > 0 ? htmlReport : "<p style='color:#27ae60; font-weight:bold; font-size:14px;'>Scan complete. No issues found by API!</p>");
              area->setReadOnly(true);
              area->setStyleSheet("background: #fdfdfd; border: 1px solid #eee; border-radius: 8px; padding: 15px;");
              layout->addWidget(area);

              QLabel *summary = new QLabel(QString("Summary: %1 Scanned | %2 Issues found (%3 High Risk)").arg(totalScanned).arg(totalAnomalies).arg(highRisk));
              summary->setStyleSheet("font-weight: 800; color: " + QString(highRisk > 0 ? "#e74c3c" : "#2c3e50") + ";");
              layout->addWidget(summary);

              QPushButton *close = new QPushButton("Close Dashboard");
              close->setStyleSheet("background: #2c3e50; color: white; height: 35px; border-radius: 6px; font-weight: bold;");
              QObject::connect(close, &QPushButton::clicked, dlg, &QDialog::accept);
              layout->addWidget(close);

              api->deleteLater();
              dlg->exec();
          });

          api->scanTransactions(transactionsToScan);
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
            expTable->window(), "Export Expense Report", "expense_report.pdf",
            "PDF Files (*.pdf)");
        if (fileName.isEmpty()) return;

        // Calculate Stats
        double totalExp = 0;
        for (int r = 0; r < expTable->rowCount(); ++r) {
            totalExp += expTable->item(r, 3) ? expTable->item(r, 3)->text().toDouble() : 0.0;
        }

        QString html =
            "<!DOCTYPE html><html><head><style>"
            "  body { font-family: 'Segoe UI', sans-serif; padding: 40px; color: #1a1a1a; }"
            "  .header { border-bottom: 3px solid #e74c3c; padding-bottom: 20px; margin-bottom: 30px; }"
            "  .title { font-size: 28px; font-weight: 800; color: #1a1a1a; }"
            "  .stat-card { background: #fff5f5; border-radius: 12px; padding: 20px; border: 1px solid #fed7d7; width: 300px; margin-bottom: 30px; }"
            "  .stat-lbl { font-size: 11px; text-transform: uppercase; color: #c53030; font-weight: 700; }"
            "  .stat-val { font-size: 22px; font-weight: 800; margin-top: 5px; color: #e74c3c; }"
            "  table { width: 100%; border-collapse: collapse; margin-top: 20px; border: 1px solid #eee; border-radius: 8px; overflow: hidden; }"
            "  th { background: #2d3748; color: white; padding: 12px; text-align: left; font-size: 11px; text-transform: uppercase; }"
            "  td { padding: 10px; border-bottom: 1px solid #eee; font-size: 12px; color: #4a5568; }"
            "  .footer { text-align: center; margin-top: 40px; font-size: 10px; color: #aaa; }"
            "</style></head><body>"
            "<div class='header'><span class='title'>Expense Tracking Report</span><br>"
            "<small>Generated on " + QDate::currentDate().toString("dd/MM/yyyy") + "</small></div>"
            "<div class='stat-card'><div class='stat-lbl'>Total Expenses</div><div class='stat-val'>" + QString::number(totalExp, 'f', 2) + " TND</div></div>"
            "<table><thead><tr>"
            "<th>Date</th><th>Type</th><th>Description</th><th>Amount</th><th>Payment</th>"
            "</tr></thead><tbody>";

        for (int r = 0; r < expTable->rowCount(); ++r) {
          html += "<tr>";
          for (int c = 0; c < expTable->columnCount(); ++c) {
            QString val = expTable->item(r, c) ? expTable->item(r, c)->text() : "";
            html += "<td>" + val + "</td>";
          }
          html += "</tr>";
        }
        html += "</tbody></table><div class='footer'>Oil Press Manager Backend - Professional Edition</div></body></html>";

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        printer.setPageSize(QPageSize(QPageSize::A4));

        QTextDocument doc;
        doc.setHtml(html);
        doc.print(&printer);

        QMessageBox::information(expTable->window(), "Success", "Expense report exported and saved!");
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
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

       // Connect Print Report for Analytics
       QObject::connect(btnPrint, &QPushButton::clicked, [totalRevenue, totalExpense, net]() {
           QString fileName = QFileDialog::getSaveFileName(
               nullptr, "Export Analytics Report", "analytics_report.pdf", "PDF Files (*.pdf)");
           if (fileName.isEmpty()) return;

           QString html =
               "<!DOCTYPE html><html><head><style>"
               "  body { font-family: 'Segoe UI', sans-serif; padding: 50px; color: #1a1a1a; }"
               "  .header { border-bottom: 4px solid #3DDC84; padding-bottom: 20px; margin-bottom: 40px; }"
               "  .title { font-size: 32px; font-weight: 900; color: #2C3E1F; }"
               "  .card-container { display: table; width: 100%; border-spacing: 15px; margin-left: -15px; }"
               "  .card { display: table-cell; background: #f9fafb; border-radius: 12px; padding: 25px; border: 1px solid #eee; text-align: center; }"
               "  .card-lbl { font-size: 12px; text-transform: uppercase; color: #888; font-weight: 700; margin-bottom: 10px; }"
               "  .card-val { font-size: 24px; font-weight: 800; }"
               "  .footer { text-align: center; margin-top: 60px; font-size: 11px; color: #aaa; }"
               "</style></head><body>"
               "<div class='header'><span class='title'>Financial Analytics Summary</span><br>"
               "<span>Oil Press Manager Professional Report</span></div>"

               "<table width='100%' cellspacing='15'><tr>"
               "  <td width='33%'><div class='card'><div class='card-lbl'>Total Revenue</div><div class='card-val' style='color:#3DDC84'>" + QString::number(totalRevenue, 'f', 2) + " TND</div></div></td>"
               "  <td width='33%'><div class='card'><div class='card-lbl'>Total Expenses</div><div class='card-val' style='color:#e74c3c'>" + QString::number(totalExpense, 'f', 2) + " TND</div></div></td>"
               "  <td width='33%'><div class='card'><div class='card-lbl'>Net Balance</div><div class='card-val' style='color:#2c3e50'>" + QString::number(net, 'f', 2) + " TND</div></div></td>"
               "</tr></table>"

               "<div style='margin-top:50px; padding:30px; background:#fff; border:1px solid #eee; border-radius:12px;'>"
               "  <h3 style='margin-top:0;'>Report Insights</h3>"
               "  <p>This report provides a high-level overview of the current financial health of the oil press operations. "
               "  The data reflects all processed transactions including raw material procurement and product sales.</p>"
               "</div>"

               "<div class='footer'>Generated by Oil Press Manager Suite</div></body></html>";

           QPrinter printer(QPrinter::HighResolution);
           printer.setOutputFormat(QPrinter::PdfFormat);
           printer.setOutputFileName(fileName);
           printer.setPageSize(QPageSize(QPageSize::A4));

           QTextDocument doc;
           doc.setHtml(html);
           doc.print(&printer);

           QMessageBox::information(nullptr, "Success", "Analytics report saved!");
           QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
       });
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
                          "Analytics", "Market IA"};
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
      outerLayout->setContentsMargins(15, 15, 15, 15);

      QWidget *formContent = new QWidget();
      formContent->setAttribute(Qt::WA_TranslucentBackground);
      formContent->setStyleSheet("background: transparent;");
      QVBoxLayout *formLayout = new QVBoxLayout(formContent);
      formLayout->setContentsMargins(0, 0, 0, 0);
      formLayout->setSpacing(8);

      QLabel *titleLabel = new QLabel("Add New Stock Item");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: "
                                "#1a1a1a; margin-bottom: 10px; border: none;");
      formLayout->addWidget(titleLabel);

      QString labelStyle = getLabelStyle();
      QString inputStyle = getInputStyle();

      auto createErrLabel = []() {
        QLabel *lbl = new QLabel("");
        lbl->setStyleSheet("color: #d32f2f; font-size: 11px; font-weight: 600; margin-top: -3px; margin-bottom: 2px;");
        lbl->hide();
        return lbl;
      };

      // NOM
      QLabel *lblNom = new QLabel("Item Name:");
      lblNom->setStyleSheet(labelStyle);
      QLineEdit *inputNom = new QLineEdit();
      inputNom->setStyleSheet(inputStyle);
      inputNom->setPlaceholderText("Extra Virgin 1L");
      QLabel *errNom = createErrLabel();
      formLayout->addWidget(lblNom);
      formLayout->addWidget(inputNom);
      formLayout->addWidget(errNom);

      // QUANTITE
      QLabel *lblQty = new QLabel("Quantity:");
      lblQty->setStyleSheet(labelStyle);
      QLineEdit *inputQty = new QLineEdit();
      inputQty->setStyleSheet(inputStyle);
      inputQty->setPlaceholderText("0");
      QLabel *errQty = createErrLabel();
      formLayout->addWidget(lblQty);
      formLayout->addWidget(inputQty);
      formLayout->addWidget(errQty);

      // UNITE
      QLabel *lblUnt = new QLabel("Unit:");
      lblUnt->setStyleSheet(labelStyle);
      QLineEdit *inputUnt = new QLineEdit();
      inputUnt->setStyleSheet(inputStyle);
      inputUnt->setPlaceholderText("Liters / Kg / Units");
      QLabel *errUnt = createErrLabel();
      formLayout->addWidget(lblUnt);
      formLayout->addWidget(inputUnt);
      formLayout->addWidget(errUnt);

      // PRIX UNITAIRE
      QLabel *lblPrix = new QLabel("Unit Price:");
      lblPrix->setStyleSheet(labelStyle);
      QLineEdit *inputPrix = new QLineEdit();
      inputPrix->setStyleSheet(inputStyle);
      inputPrix->setPlaceholderText("0");
      QLabel *errPrix = createErrLabel();
      formLayout->addWidget(lblPrix);
      formLayout->addWidget(inputPrix);
      formLayout->addWidget(errPrix);

      // DATE D'ACHAT
      QLabel *lblDate = new QLabel("Date of Purchase:");
      lblDate->setStyleSheet(labelStyle);
      QDateEdit *inputDate = new QDateEdit(QDate::currentDate());
      inputDate->setStyleSheet(inputStyle);
      inputDate->setCalendarPopup(true);
      QLabel *errDate = createErrLabel();
      formLayout->addWidget(lblDate);
      formLayout->addWidget(inputDate);
      formLayout->addWidget(errDate);

      // Submit Button
      formLayout->addSpacing(10);
      QPushButton *btnSubmit = new QPushButton("Add Item");
      btnSubmit->setStyleSheet(getButtonStyle());
      btnSubmit->setCursor(Qt::PointingHandCursor);
      btnSubmit->setFixedHeight(45);
      formLayout->addWidget(btnSubmit);

      // Connect Add Button -> Article::ajouter()
      QObject::connect(
          btnSubmit, &QPushButton::clicked, [page, inputNom, inputQty, inputUnt, inputPrix, inputDate, errNom, errQty, errUnt, errPrix, errDate]() {
            MainWindow* mw = qobject_cast<MainWindow*>(page->window());
            // Reset errors
            errNom->hide();
            errQty->hide();
            errUnt->hide();
            errPrix->hide();
            errDate->hide();

            bool isValid = true;

            // Input validation
            if (inputNom->text().trimmed().isEmpty()) {
              errNom->setText("Item Name is required.");
              errNom->show();
              isValid = false;
            } else {
              Article testA;
              if (testA.exists(inputNom->text().trimmed())) {
                errNom->setText("Item name already exists. Please use a unique name.");
                errNom->show();
                isValid = false;
              }
            }

            bool qtyOk;
            int qty = inputQty->text().toInt(&qtyOk);
            if (!qtyOk || qty < 0) {
              errQty->setText("Quantity must be a valid positive number.");
              errQty->show();
              isValid = false;
            }

            QRegularExpression unitRegex("^[a-zA-Z\\s]+$");
            if (!unitRegex.match(inputUnt->text().trimmed()).hasMatch()) {
              errUnt->setText("Unit must strictly contain letters and spaces.");
              errUnt->show();
              isValid = false;
            }

            bool prixOk;
            int prix = inputPrix->text().toInt(&prixOk);
            if (!prixOk || prix < 0) {
              errPrix->setText("Unit Price must be a valid positive number.");
              errPrix->show();
              isValid = false;
            }

            if (inputDate->date() > QDate::currentDate()) {
              errDate->setText("Purchase Date cannot be in the future.");
              errDate->show();
              isValid = false;
            }

            if (!isValid) return;

            Article a;
            a.setNom(inputNom->text().trimmed());
            a.setQuantite(inputQty->text().toInt());
            a.setUnite(inputUnt->text().trimmed());
            a.setPrixUnitaire(inputPrix->text().toInt());
            a.setDateAchat(inputDate->date());

            if (a.ajouter()) {
              QMessageBox::information(nullptr, "Success",
                                       "Item added successfully!");
              // Clear the form
              inputNom->clear();
              inputQty->clear();
              inputUnt->clear();
              inputPrix->clear();
              inputDate->setDate(QDate::currentDate());
              if (mw) mw->checkStockAlerts(true);
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

      QPushButton *btnPrintPdf = new QPushButton("Print Shortage Report");
      btnPrintPdf->setStyleSheet(getButtonStyle());
      btnPrintPdf->setCursor(Qt::PointingHandCursor);
      btnPrintPdf->setFixedWidth(220);

      QPushButton *btnAlerts = new QPushButton("Check Alerts");
      btnAlerts->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; border: none; border-radius: 8px; padding: 10px 15px; font-weight: 700; } QPushButton:hover { background-color: #c0392b; }");
      btnAlerts->setCursor(Qt::PointingHandCursor);
      btnAlerts->setFixedWidth(140);

      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(lblSort);
      controlLayout->addWidget(sortCombo);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnRefresh);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnPrintPdf);
      controlLayout->addSpacing(10);
      controlLayout->addWidget(btnAlerts);
      controlLayout->addStretch();
      reportLayout->addWidget(controlBar);

      // 2. Table
      stockTable = new QTableWidget();
      QStringList headers = {"ID", "Item Name", "Current Qty", "Unit", "Unit Price", "Purchase Date",
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
      auto refreshTable = [stockTable, sortCombo]() {
        Article a;
        QString sortBy = "";
        QString order = "ASC";

        QString currentSort = sortCombo->currentText();
        if (currentSort == "Quantity (High-Low)") {
          sortBy = "QUANTITE";
          order = "DESC";
        } else if (currentSort == "Quantity (Low-High)") {
          sortBy = "QUANTITE";
          order = "ASC";
        }

        QSqlQueryModel *model = a.afficher(sortBy, order);

        stockTable->setRowCount(0); // Clear existing rows

        int rowCount = model->rowCount();
        stockTable->setRowCount(rowCount);

        for (int i = 0; i < rowCount; ++i) {
          // Columns: ID(0), NOM(1), QUANTITE(2), UNITE(3), PRIX(4), DATE(5)
          QString id = model->data(model->index(i, 0)).toString();
          QString nom = model->data(model->index(i, 1)).toString();
          QString qty = model->data(model->index(i, 2)).toString();
          QString unt = model->data(model->index(i, 3)).toString();
          QString prix = model->data(model->index(i, 4)).toString();
          QString dateStr = model->data(model->index(i, 5)).toString();

          stockTable->setItem(i, 0, new QTableWidgetItem(id));
          stockTable->setItem(i, 1, new QTableWidgetItem(nom));
          stockTable->setItem(i, 2, new QTableWidgetItem(qty));
          stockTable->setItem(i, 3, new QTableWidgetItem(unt));
          stockTable->setItem(i, 4, new QTableWidgetItem(prix));
          stockTable->setItem(i, 5, new QTableWidgetItem(dateStr));

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
              [stockTable, itemId, nom, qty, unt, prix, dateStr]() {
                QDialog *dialog = new QDialog(stockTable->window());
                dialog->setWindowTitle("Modify Item");
                dialog->setMinimumWidth(450);
                dialog->setStyleSheet(
                    "QDialog { background-color: #ffffff; }"
                    "QLabel { font-size: 14px; font-weight: 700; color: #333; }"
                    "QLineEdit, QDateEdit { background-color: #fcfcfc; border: 1px solid "
                    "#e0e0e0; border-radius: 6px; padding: 6px 10px; "
                    "font-size: 13px; color: #333; min-height: 30px; }"
                    "QLineEdit:focus, QDateEdit:focus { border: 2px solid #3DDC84; "
                    "background-color: #ffffff; }");

                QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
                dialogLayout->setContentsMargins(30, 30, 30, 30);
                dialogLayout->setSpacing(15);

                QLabel *dlgTitle = new QLabel("Modify Item");
                dlgTitle->setStyleSheet("font-size: 22px; font-weight: 800; "
                                        "color: #1a1a1a; margin-bottom: 10px;");
                dialogLayout->addWidget(dlgTitle);

                auto createErrLabelDlg = []() {
                  QLabel *lbl = new QLabel("");
                  lbl->setStyleSheet("color: #d32f2f; font-size: 11px; font-weight: 600; margin-top: -5px; border: none; background: transparent;");
                  lbl->hide();
                  return lbl;
                };

                QLabel *lblNom = new QLabel("Item Name:");
                QLineEdit *editNom = new QLineEdit(nom);
                QLabel *errNom = createErrLabelDlg();
                dialogLayout->addWidget(lblNom);
                dialogLayout->addWidget(editNom);
                dialogLayout->addWidget(errNom);

                QLabel *lblQty = new QLabel("Quantity:");
                QLineEdit *editQty = new QLineEdit(qty);
                QLabel *errQty = createErrLabelDlg();
                dialogLayout->addWidget(lblQty);
                dialogLayout->addWidget(editQty);
                dialogLayout->addWidget(errQty);

                QLabel *lblUnt = new QLabel("Unit:");
                QLineEdit *editUnt = new QLineEdit(unt);
                QLabel *errUnt = createErrLabelDlg();
                dialogLayout->addWidget(lblUnt);
                dialogLayout->addWidget(editUnt);
                dialogLayout->addWidget(errUnt);

                QLabel *lblPrix = new QLabel("Unit Price:");
                QLineEdit *editPrix = new QLineEdit(prix);
                QLabel *errPrix = createErrLabelDlg();
                dialogLayout->addWidget(lblPrix);
                dialogLayout->addWidget(editPrix);
                dialogLayout->addWidget(errPrix);

                QLabel *lblDate = new QLabel("Purchase Date:");
                QDateEdit *editDate = new QDateEdit(QDate::fromString(dateStr, Qt::ISODate));
                editDate->setCalendarPopup(true);
                QLabel *errDate = createErrLabelDlg();
                dialogLayout->addWidget(lblDate);
                dialogLayout->addWidget(editDate);
                dialogLayout->addWidget(errDate);

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
                    [dialog, editNom, editQty, editUnt, editPrix, editDate, itemId, qty,
                     stockTable, errNom, errQty, errUnt, errPrix, errDate]() {
                      errNom->hide();
                      errQty->hide();
                      errUnt->hide();
                      errPrix->hide();
                      errDate->hide();

                      bool isValid = true;

                      if (editNom->text().trimmed().isEmpty()) {
                        errNom->setText("Item Name is required.");
                        errNom->show();
                        isValid = false;
                      } else {
                        Article testA;
                        if (testA.exists(editNom->text().trimmed(), itemId)) {
                          errNom->setText("Item name already exists. Please use a unique name.");
                          errNom->show();
                          isValid = false;
                        }
                      }

                      bool qtyOk;
                      int q = editQty->text().toInt(&qtyOk);
                      if (!qtyOk || q < 0) {
                        errQty->setText("Quantity must be a valid positive number.");
                        errQty->show();
                        isValid = false;
                      }

                      QRegularExpression unitRegex("^[a-zA-Z\\s]+$");
                      if (!unitRegex.match(editUnt->text().trimmed()).hasMatch()) {
                        errUnt->setText("Unit must strictly contain letters and spaces.");
                        errUnt->show();
                        isValid = false;
                      }

                      bool prixOk;
                      int p = editPrix->text().toInt(&prixOk);
                      if (!prixOk || p < 0) {
                        errPrix->setText("Unit Price must be a valid positive number.");
                        errPrix->show();
                        isValid = false;
                      }

                      if (editDate->date() > QDate::currentDate()) {
                        errDate->setText("Purchase Date cannot be in the future.");
                        errDate->show();
                        isValid = false;
                      }

                      if (!isValid) return;

                      // Increment usage if quantity decreased
                      int oldQ = qty.toInt();
                      int newQ = editQty->text().toInt();
                      int currentUsage = 0;
                      
                      QSqlQuery usageQuery;
                      usageQuery.prepare("SELECT USAGE_COUNT FROM ARTICLE WHERE ID_ARTICLE = :id");
                      usageQuery.bindValue(":id", itemId);
                      if (usageQuery.exec() && usageQuery.next()) {
                          currentUsage = usageQuery.value(0).toInt();
                      }

                      Article a;
                      a.setId(itemId);
                      a.setNom(editNom->text().trimmed());
                      a.setQuantite(newQ);
                      a.setUnite(editUnt->text().trimmed());
                      a.setPrixUnitaire(editPrix->text().toInt());
                      a.setDateAchat(editDate->date());
                      
                      if (newQ < oldQ) {
                          a.setUsageCount(currentUsage + (oldQ - newQ));
                      } else {
                          a.setUsageCount(currentUsage);
                      }

                        if (a.modifier()) {
                          QMessageBox::information(nullptr, "Success",
                                                   "Item updated successfully!");
                          dialog->accept();
                          MainWindow* mw = qobject_cast<MainWindow*>(stockTable->window());
                          if (mw) mw->checkStockAlerts(true);
                        for (int r = 0; r < stockTable->rowCount(); ++r) {
                          if (stockTable->item(r, 0) &&
                              stockTable->item(r, 0)->text().toInt() ==
                                  itemId) {
                            stockTable->item(r, 1)->setText(
                                editNom->text().trimmed());
                            stockTable->item(r, 2)->setText(editQty->text());
                            stockTable->item(r, 3)->setText(editUnt->text());
                            stockTable->item(r, 4)->setText(editPrix->text());
                            stockTable->item(r, 5)->setText(editDate->date().toString(Qt::ISODate));
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

          stockTable->setCellWidget(i, 6, actionWidget);
        }

        delete model;
      };

      // Initial load
      refreshTable();

      // Trigger refresh on sort change
      QObject::connect(sortCombo, &QComboBox::activated, refreshTable);

      // Refresh button
      QObject::connect(btnRefresh, &QPushButton::clicked, refreshTable);

      // Alert check button connection
      QObject::connect(btnAlerts, &QPushButton::clicked, [stockTable]() {
          MainWindow* mw = qobject_cast<MainWindow*>(stockTable->window());
          if (mw) mw->checkStockAlerts(false); // Manually check and show details

          // Row highlighting logic
          for (int i = 0; i < stockTable->rowCount(); ++i) {
              int qty = stockTable->item(i, 2)->text().toInt();
              if (qty <= 10) {
                  for (int j = 0; j < stockTable->columnCount(); ++j) {
                      if (stockTable->item(i, j))
                          stockTable->item(i, j)->setBackground(QColor(255, 235, 238)); // Light red tint
                  }
              } else {
                  for (int j = 0; j < stockTable->columnCount(); ++j) {
                      if (stockTable->item(i, j))
                          stockTable->item(i, j)->setBackground(QBrush()); // Reset
                  }
              }
          }
      });

      // Low Stock PDF Export
      QObject::connect(btnPrintPdf, &QPushButton::clicked, [stockTable]() {
          QSqlQuery query;
          query.prepare("SELECT NOM_ARTICLE, QUANTITE, UNITE, PRIX_UNITAIRE FROM ARTICLE WHERE QUANTITE <= 10 ORDER BY QUANTITE ASC");

          if (!query.exec()) {
              QMessageBox::critical(stockTable->window(), "Database Error", "Failed to fetch low stock data.");
              return;
          }

          QString strStream;
          QTextStream out(&strStream);

          out << "<html>\n"
                 "<head>\n"
                 "<meta Content=\"Text/html; charset=utf-8\">\n"
                 "<title>Low Stock Report</title>\n"
                 "</head>\n"
                 "<body bgcolor=#ffffff>\n"
                 "<div style=\"text-align: center;\">"
                 "<img src=\":/logo.png\" width=\"120\">"
                 "</div>"
                 "<h1 style=\"text-align: center; color: #e74c3c; font-family: Arial, sans-serif;\">⚠️ Low Stock Alert Report</h1>\n"
                 "<p style=\"text-align: center; color: #7f8c8d; font-family: Arial, sans-serif;\">Threshold: 10 units or less | Generated on: " + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm") + "</p>\n"
                 "<table border=1 cellspacing=0 cellpadding=10 width=\"100%\" style=\"border-collapse: collapse; font-family: Arial, sans-serif; margin-top: 20px;\">\n"
                 "<thead><tr bgcolor=#f8f9fa style=\"color: #333;\">"
                 "<th style=\"border: 1px solid #ddd;\">Item Name</th>"
                 "<th style=\"border: 1px solid #ddd;\">Quantity to Buy</th>"
                 "<th style=\"border: 1px solid #ddd;\">Unit</th>"
                 "<th style=\"border: 1px solid #ddd;\">Unit Price</th>"
                 "</tr></thead>\n<tbody>\n";

          int count = 0;
          while (query.next()) {
              count++;
              out << "<tr>"
                  << "<td style=\"border: 1px solid #ddd;\">" << query.value(0).toString() << "</td>"
                  << "<td style=\"border: 1px solid #ddd; font-weight: bold; color: #d32f2f; text-align: center;\">" << (30 - query.value(1).toInt()) << "</td>"
                  << "<td style=\"border: 1px solid #ddd; text-align: center;\">" << query.value(2).toString() << "</td>"
                  << "<td style=\"border: 1px solid #ddd; text-align: right;\">" << query.value(3).toString() << " DT</td>"
                  << "</tr>\n";
          }

          if (count == 0) {
              QMessageBox::information(stockTable->window(), "Info", "No items are currently below the threshold of 10.");
              return;
          }

          out << "</tbody></table>\n"
                 "<p style=\"margin-top: 30px; font-size: 12px; color: #95a5a6; border-top: 1px solid #eee; padding-top: 10px;\">Total items requiring attention: " << count << "</p>\n"
                 "</body>\n"
                 "</html>\n";

          QTextDocument document;
          document.setHtml(strStream);

          QString defaultName = "Alerte_Stock_Bas_" + QDateTime::currentDateTime().toString("dd_MM_yyyy") + ".pdf";
          QString fileName = QFileDialog::getSaveFileName(stockTable->window(), "Save Shortage Report", QDir::currentPath() + "/" + defaultName, "PDF Files (*.pdf)");

          if (fileName.isEmpty()) return;

          QPrinter printer(QPrinter::PrinterResolution);
          printer.setOutputFormat(QPrinter::PdfFormat);
          printer.setOutputFileName(fileName);
          printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

          document.print(&printer);
          QMessageBox::information(stockTable->window(), "Success", "Shortage report has been saved to:\n" + fileName);
      });

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

    } else if (name == "Analytics") {
      // --- Analytics Tab Header with Refresh Button ---
      QWidget *headerWidget = new QWidget();
      QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
      headerLayout->setContentsMargins(0, 0, 0, 10);

      QLabel *statTitle = new QLabel("Real-Time Stock Analytics");
      statTitle->setStyleSheet("font-size: 18px; font-weight: 700; color: #1a1a1a;");

      QPushButton *btnRefreshStats = new QPushButton("Refresh Data");
      btnRefreshStats->setStyleSheet(getButtonStyle());
      btnRefreshStats->setFixedWidth(140);
      btnRefreshStats->setCursor(Qt::PointingHandCursor);

      headerLayout->addWidget(statTitle);
      headerLayout->addStretch();
      headerLayout->addWidget(btnRefreshStats);
      cLayout->addWidget(headerWidget);

      QWidget *chartsContainer = new QWidget();
      QVBoxLayout *chartsLayout = new QVBoxLayout(chartsContainer);
      chartsLayout->setContentsMargins(0, 0, 0, 0);
      chartsLayout->setSpacing(20);
      cLayout->addWidget(chartsContainer);

      auto refreshAnalytics = [chartsLayout]() {
          // Clear previous charts
          QLayoutItem *child;
          while ((child = chartsLayout->takeAt(0)) != nullptr) {
              if (child->widget()) child->widget()->deleteLater();
              delete child;
          }

          QList<QColor> colors = {QColor(52, 152, 219), QColor(46, 204, 113), QColor(241, 196, 15), QColor(155, 89, 182), QColor(231, 76, 60)};

          // 2. Most Used Items Chart
          GenericBarChart *usageChart = new GenericBarChart("Most Frequently Used Items (Top 5)");
          QSqlQuery q2("SELECT NOM_ARTICLE, USAGE_COUNT FROM ARTICLE WHERE USAGE_COUNT > 0 ORDER BY USAGE_COUNT DESC");
          int count2 = 0;
          while (q2.next() && count2 < 5) {
              usageChart->addBar(q2.value(0).toString(), q2.value(1).toDouble(), colors[(count2 + 2) % colors.size()]);
              count2++;
          }
          if (count2 == 0) usageChart->addBar("No Usage Data Yet", 0, QColor(189, 195, 199));

          QWidget *usageCard = new QWidget();
          usageCard->setStyleSheet(getCardStyle());
          QVBoxLayout *v2 = new QVBoxLayout(usageCard);
          v2->setContentsMargins(20, 20, 20, 20);
          v2->addWidget(usageChart);
          chartsLayout->addWidget(usageCard);
      };

      QObject::connect(btnRefreshStats, &QPushButton::clicked, refreshAnalytics);
      refreshAnalytics(); // Initial load

    } else if (name == "Market IA") {
      // ========== MARKET IA (TABS: Consultant & Deals) ==========
      QWidget *marketContainer = new QWidget();
      QVBoxLayout *marketLayout = new QVBoxLayout(marketContainer);
      marketLayout->setContentsMargins(0, 0, 0, 0);

      QTabWidget *marketTabs = new QTabWidget();
      marketTabs->setMaximumHeight(600); // Shorter dashboard
      marketTabs->setStyleSheet(
          "QTabWidget::pane { border: 1px solid #eee; border-radius: 12px; background: white; } "
          "QTabBar::tab { background: #f8f9fa; color: #666; padding: 10px 20px; border-top-left-radius: 8px; border-top-right-radius: 8px; margin-right: 2px; } "
          "QTabBar::tab:selected { background: white; color: #1a1a1a; font-weight: bold; border-bottom: 3px solid #3DDC84; }");

      // --- TAB 1: AI CONSULTANT (CHAT) ---
      QWidget *chatTab = new QWidget();
      QVBoxLayout *chatTabLayout = new QVBoxLayout(chatTab);
      chatTabLayout->setContentsMargins(10, 10, 10, 10); // Compact margins

      QScrollArea *chatScroll = new QScrollArea();
      chatScroll->setWidgetResizable(true);
      chatScroll->setFrameShape(QFrame::NoFrame);
      QWidget *chatContent = new QWidget();
      QVBoxLayout *chatList = new QVBoxLayout(chatContent);
      chatList->setSpacing(10);
      chatList->addStretch();
      chatScroll->setWidget(chatContent);
      chatTabLayout->addWidget(chatScroll);

      auto addMessage = [chatList, chatScroll](const QString &sender, const QString &text, bool isUser) {
          QWidget *msg = new QWidget();
          msg->setStyleSheet(isUser ? "background: #e6f9ef; border-radius: 15px; border-bottom-right-radius: 2px;" : "background: #f1f3f5; border-radius: 15px; border-bottom-left-radius: 2px;");
          QVBoxLayout *ml = new QVBoxLayout(msg);
          QLabel *s = new QLabel(sender);
          s->setStyleSheet(QString("font-weight: 800; font-size: 10px; color: %1;").arg(isUser ? "#2e7d32" : "#495057"));
          QLabel *t = new QLabel(text);
          t->setWordWrap(true);
          t->setStyleSheet("font-size: 13px; color: #212529;");
          ml->addWidget(s);
          ml->addWidget(t);

          QHBoxLayout *row = new QHBoxLayout();
          if (isUser) row->addStretch();
          row->addWidget(msg);
          if (!isUser) row->addStretch();

          chatList->insertLayout(chatList->count() - 1, row);
          QTimer::singleShot(50, [chatScroll]() { chatScroll->verticalScrollBar()->setValue(chatScroll->verticalScrollBar()->maximum()); });
      };

      QWidget *inputArea = new QWidget();
      QHBoxLayout *il = new QHBoxLayout(inputArea);
      il->setContentsMargins(0, 10, 0, 0);
      QLineEdit *chatInput = new QLineEdit();
      chatInput->setPlaceholderText("Ask about machinery, bottles, prices...");
      chatInput->setStyleSheet("padding: 12px; border: 1px solid #ddd; border-radius: 20px; background: white;");
      QPushButton *btnSend = new QPushButton("Send");
      btnSend->setStyleSheet("background: #1a1a1a; color: white; padding: 12px 20px; border-radius: 20px; font-weight: 700;");
      btnSend->setCursor(Qt::PointingHandCursor);
      il->addWidget(chatInput);
      il->addWidget(btnSend);
      chatTabLayout->addWidget(inputArea);

      ConsultantAgent *agent = new ConsultantAgent(chatTab);
      auto handleSend = [chatInput, agent, addMessage]() {
          QString text = chatInput->text().trimmed();
          if (text.isEmpty()) return;
          addMessage("YOU", text, true);
          chatInput->clear();
          QString response = agent->getResponse(text);
          QTimer::singleShot(500, [addMessage, response]() { addMessage("MARKET IA", response, false); });
      };

      QObject::connect(btnSend, &QPushButton::clicked, handleSend);
      QObject::connect(chatInput, &QLineEdit::returnPressed, handleSend);

      addMessage("MARKET IA", "Welcome! I am your 2026 Market Intelligence Agent. Ask me about the best **Machinery**, **Packaging/Bottles**, **Fertilizers**, or **Prices**.", false);

      // --- TAB 2: EXCLUSIVE DEALS 2026 (DYNAMIC AI GENERATED) ---
      QWidget *dealsTab = new QWidget();
      dealsTab->setStyleSheet("background: #f8f9fa;");
      QVBoxLayout *dealsLayout = new QVBoxLayout(dealsTab);
      dealsLayout->setContentsMargins(10, 10, 10, 10);
      dealsLayout->setSpacing(10);

      QWidget *dealsHeader = new QWidget();
      QHBoxLayout *hlHeader = new QHBoxLayout(dealsHeader);
      QLabel *dealsTitle = new QLabel("Top Procurement Deals - Q2 2026");
      dealsTitle->setStyleSheet("font-size: 20px; font-weight: 900; color: #1a1a1a; text-transform: uppercase;");

      QPushButton *btnRefreshDeals = new QPushButton("Scan Market with AI");
      btnRefreshDeals->setStyleSheet("background: #3DDC84; color: white; padding: 10px 20px; border-radius: 8px; font-weight: 800;");
      btnRefreshDeals->setCursor(Qt::PointingHandCursor);

      hlHeader->addWidget(dealsTitle);
      hlHeader->addStretch();
      hlHeader->addWidget(btnRefreshDeals);
      dealsLayout->addWidget(dealsHeader);

      QLabel *statusLabel = new QLabel("Market intelligence active. Click scan for live 2026 deals.");
      statusLabel->setStyleSheet("color: #666; font-size: 11px; font-weight: 600; margin-left: 10px;");
      dealsLayout->addWidget(statusLabel);

      QScrollArea *dealsScroll = new QScrollArea();
      dealsScroll->setWidgetResizable(true);
      dealsScroll->setFrameShape(QFrame::NoFrame);
      dealsScroll->setStyleSheet("background: transparent;");
      QWidget *dealsContent = new QWidget();
      dealsContent->setStyleSheet("background: transparent;");
      QVBoxLayout *dealsList = new QVBoxLayout(dealsContent);
      dealsList->setSpacing(15);
      dealsList->addStretch();
      dealsScroll->setWidget(dealsContent);
      dealsLayout->addWidget(dealsScroll);

      DealGenerator *gen = new DealGenerator();

      auto refreshDeals = [dealsList, gen, statusLabel, btnRefreshDeals]() {
          btnRefreshDeals->setEnabled(false);
          btnRefreshDeals->setText("Scanning...");
          statusLabel->setText("AI AGENT: Analyzing global 2026 logistics, machinery costs, and raw material trends...");

          // Clear current
          while (dealsList->count() > 1) {
              QLayoutItem *item = dealsList->takeAt(0);
              if (item->widget()) item->widget()->deleteLater();
              delete item;
          }

          QTimer::singleShot(1500, [dealsList, gen, statusLabel, btnRefreshDeals]() {
              QList<GeneratedDeal> deals = gen->generateDeals(8);
              for (const auto &d : deals) {
                  QWidget *card = new QWidget();
                  card->setStyleSheet("background: white; border-radius: 12px; border: 1px solid #eee;");
                  QVBoxLayout *cl = new QVBoxLayout(card);
                  cl->setContentsMargins(20, 20, 20, 20);

                  QHBoxLayout *hl = new QHBoxLayout();
                  QLabel *t = new QLabel(d.title);
                  t->setStyleSheet("font-size: 17px; font-weight: 800; color: #1a1a1a;");
                  QLabel *sTag = new QLabel(d.savings);
                  sTag->setStyleSheet("background: #3DDC84; color: white; padding: 4px 12px; border-radius: 15px; font-weight: 900; font-size: 10px;");
                  hl->addWidget(t);
                  hl->addStretch();
                  hl->addWidget(sTag);

                  QLabel *sup = new QLabel("Detected Supplier: " + d.supplier);
                  sup->setStyleSheet("color: #2e7d32; font-size: 11px; font-weight: 700;");

                  QLabel *descLbl = new QLabel(d.description);
                  descLbl->setWordWrap(true);
                  descLbl->setStyleSheet("font-size: 12px; color: #555; margin-top: 5px;");

                  QPushButton *btnLink = new QPushButton("View Intelligence Details");
                  btnLink->setStyleSheet("background: white; border: 2px solid #3DDC84; color: #3DDC84; padding: 10px; border-radius: 6px; font-weight: 900; margin-top: 10px;");
                  btnLink->setCursor(Qt::PointingHandCursor);

                  QString url = d.url;
                  QObject::connect(btnLink, &QPushButton::clicked, [url]() {
                      QDesktopServices::openUrl(QUrl(url));
                  });

                  cl->addLayout(hl);
                  cl->addWidget(sup);
                  cl->addWidget(descLbl);
                  cl->addWidget(btnLink);
                  dealsList->insertWidget(dealsList->count() - 1, card);
              }
              btnRefreshDeals->setEnabled(true);
              btnRefreshDeals->setText("Scan Market with AI");
              statusLabel->setText("AI AGENT: 8 High-value procurement opportunities detected for Q2 2026.");
          });
      };

      QObject::connect(btnRefreshDeals, &QPushButton::clicked, refreshDeals);

      // Initial scan
      refreshDeals();

      marketTabs->addTab(chatTab, "AI Industry Consultant");
      marketTabs->addTab(dealsTab, "Exclusive Deals 2026");
      marketLayout->addWidget(marketTabs);
      marketLayout->addStretch(); // Push tabs to top of container
      cLayout->addWidget(marketContainer);
      cLayout->addStretch(); // Push container to top of page
    }
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
  QStringList tabNames = {"Add Asset", "Asset Hub", "Analytics"};
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

      table->setItem(i, 6, new QTableWidgetItem(model->data(model->index(i, 6)).toString()));

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
      table->setCellWidget(i, 7, actionWidget);

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
          QString currentLoc = table->item(row, 6)->text();

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
          edName->setMaxLength(50);
          edName->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9\\s\\-_]+$"), edName));

          mainV->addWidget(new QLabel("Type:"));
          QComboBox *edType = new QComboBox();
          edType->addItems({"press", "filter"});
          edType->setCurrentText(currentType);
          edType->setStyleSheet("QComboBox { background-color: #f9fafb; border: 1px solid #eaeaea; border-radius: 8px; padding: 10px; font-size: 14px; }");
          mainV->addWidget(edType);

          mainV->addWidget(new QLabel("Status:"));
          QComboBox *edStatus = new QComboBox();
          edStatus->addItems({"Normal", "En panne", "En maintenance"});
          edStatus->setCurrentText(currentStatus);
          edStatus->setStyleSheet("QComboBox { background-color: #f9fafb; border: 1px solid #eaeaea; border-radius: 8px; padding: 10px; font-size: 14px; }");
          mainV->addWidget(edStatus);

          QLineEdit *edHours = addField("Hours:", QString::number(currentHours));
          edHours->setValidator(new QIntValidator(0, 9999999, edHours));
          QLineEdit *edSeuil = addField("Threshold:", QString::number(currentSeuil));
          edSeuil->setValidator(new QIntValidator(0, 9999999, edSeuil));
          QLineEdit *edLoc = addField("Location:", currentLoc);

          QPushButton *btnSave = new QPushButton("Save Changes");
          btnSave->setStyleSheet("QPushButton { background-color: #3DDC84; color: white; border: none; border-radius: 8px; padding: 12px; font-weight: 700; }");
          mainV->addWidget(btnSave);

          QObject::connect(btnSave, &QPushButton::clicked, [=, &dlg]() {
              QString nom = edName->text().trimmed();
              QString heuresStr = edHours->text().trimmed();
              QString seuilStr = edSeuil->text().trimmed();

              if (nom.isEmpty() || heuresStr.isEmpty() || seuilStr.isEmpty()) {
                  QMessageBox::warning(&dlg, "Erreur de Saisie", "Veuillez remplir tous les champs obligatoires (Nom, Heures, Seuil).");
                  return;
              }

              QRegularExpression nomRegex("^[a-zA-Z0-9\\s\\-_]+$");
              if (!nomRegex.match(nom).hasMatch()) {
                  QMessageBox::warning(&dlg, "Erreur de Saisie", "Le nom de la machine contient des caractères non autorisés.");
                  return;
              }

              int heures = heuresStr.toInt();
              int seuil = seuilStr.toInt();

              if (heures < 0) {
                  QMessageBox::warning(&dlg, "Erreur de Saisie", "Les heures de fonctionnement doivent être positives ou nulles.");
                  return;
              }

              if (seuil <= 0) {
                  QMessageBox::warning(&dlg, "Erreur de Saisie", "Le seuil de maintenance doit être strictement supérieur à 0.");
                  return;
              }

              Machine updateObj(mid, nom, edType->currentText(), edStatus->currentText(), heures, seuil, edLoc->text().trimmed());
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
                  QMessageBox::information(table->window(), "Succès", "La machine a été supprimée avec succès.");
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
      nameInput->setMaxLength(50);
      nameInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9\\s\\-_]+$"), nameInput));
      QComboBox *typeInput = new QComboBox();
      typeInput->addItems({"press", "filter"});
      QComboBox *statusInput = new QComboBox();
      statusInput->addItems({"Normal", "En panne", "En maintenance"});
      QLineEdit *hoursInput = new QLineEdit("0");
      hoursInput->setValidator(new QIntValidator(0, 9999999, hoursInput));
      QLineEdit *seuilInput = new QLineEdit("100");
      seuilInput->setValidator(new QIntValidator(0, 9999999, seuilInput));
      QLineEdit *locInput = new QLineEdit();

      auto addInput = [&](const QString &txt, QWidget *le) {
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
      addInput("Machine Location:", locInput);

      formLayout->addSpacing(20);
      QPushButton *btnAdd = new QPushButton("Add Machine");
      btnAdd->setStyleSheet(getButtonStyle());
      btnAdd->setFixedHeight(45);
      formLayout->addWidget(btnAdd);
      formLayout->addStretch();

      cLayout->addWidget(formContainer);

      QObject::connect(btnAdd, &QPushButton::clicked, [=]() {
          QString nom = nameInput->text().trimmed();
          QString heuresStr = hoursInput->text().trimmed();
          QString seuilStr = seuilInput->text().trimmed();

          if (nom.isEmpty()) {
              QMessageBox::warning(nullptr, "Erreur de Saisie", "Le champ 'Nom' est obligatoire.");
              return;
          }

          QRegularExpression nomRegex("^[a-zA-Z0-9\\s\\-_]+$");
          if (!nomRegex.match(nom).hasMatch()) {
              QMessageBox::warning(nullptr, "Erreur de Saisie", "Le nom de la machine contient des caractères non autorisés.");
              return;
          }

          if (heuresStr.isEmpty() || seuilStr.isEmpty()) {
              QMessageBox::warning(nullptr, "Erreur de Saisie", "Les heures et le seuil ne peuvent pas être vides.");
              return;
          }

          int heures = heuresStr.toInt();
          int seuil = seuilStr.toInt();
          QString loc = locInput->text().trimmed();

          if (heures < 0) {
              QMessageBox::warning(nullptr, "Erreur de Saisie", "Les heures de fonctionnement doivent être positives ou nulles.");
              return;
          }

          if (seuil <= 0) {
              QMessageBox::warning(nullptr, "Erreur de Saisie", "Le seuil de maintenance doit être strictement supérieur à 0.");
              return;
          }

          Machine newM(0, nom, typeInput->currentText(), statusInput->currentText(), heures, seuil, loc);
          if (newM.ajouter()) {
              QMessageBox::information(nullptr, "Success", "Machine added with location!");
              nameInput->clear(); typeInput->setCurrentIndex(0); statusInput->setCurrentIndex(0); hoursInput->setText("0"); seuilInput->setText("100"); locInput->clear();

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

      QStringList headers = {"ID", "Name", "Type", "Status", "Hours", "Threshold", "Location", "Actions"};
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

      QObject::connect(btnAlert, &QPushButton::clicked, [table, refreshMachineTable]() {
          if (!table || !table->model()) return;
          
          QString alertMsg;
          bool hasAlerts = false;
          int rowCount = table->rowCount();

          for (int i = 0; i < rowCount; ++i) {
             if (table->isRowHidden(i)) continue;

             // Use model for reliable data extraction
             int id = table->model()->index(i, 0).data().toInt();
             QString nom = table->model()->index(i, 1).data().toString();
             int heures = table->model()->index(i, 4).data().toInt();
             int seuil = table->model()->index(i, 5).data().toInt();
             QString currentStatus = table->model()->index(i, 3).data().toString();

             if (id <= 0) continue;

             Machine m;
             m.setId(id);
             m.setNom(nom);
             
             // Fetch machine details for update
             QSqlQuery fetchQuery;
             fetchQuery.prepare("SELECT TYPE_MACHINE, HEURESFONCTIONNEMENT, SEUILMAINTENANCE, LOCALISATION, DATEDERNIEREMAINTENANCE FROM MACHINE WHERE ID_MACHINE = :id");
             fetchQuery.bindValue(":id", id);
             if(fetchQuery.exec() && fetchQuery.next()) {
                 m.setType(fetchQuery.value(0).toString());
                 m.setHeures(fetchQuery.value(1).toInt());
                 m.setSeuil(fetchQuery.value(2).toInt());
                 m.setLocalisation(fetchQuery.value(3).toString());
                 m.setDateM(fetchQuery.value(4).toDate());
             }

             if (heures >= seuil && seuil > 0) {
                alertMsg += "Machine " + QString::number(id) + " (" + nom + ") nécessite maintenance.\n";
                hasAlerts = true;
                
                if (currentStatus != "En maintenance") {
                    m.setEtat("En maintenance");
                    m.modifier();
                }

                for(int c=0; c<table->columnCount() - 1; ++c) {
                   if(table->item(i, c)) table->item(i, c)->setBackground(QColor(255, 200, 200));
                }
             } else {
                if (currentStatus == "En maintenance") {
                    m.setEtat("Normal");
                    m.modifier();
                }

                for(int c=0; c<table->columnCount() - 1; ++c) {
                   if(table->item(i, c)) table->item(i, c)->setBackground(QBrush());
                }
             }
          }

          if (hasAlerts) {
              QMessageBox::warning(table->window(), "Alertes de Maintenance", alertMsg);
              
              // Direct API call in mainwindow.cpp
              QNetworkAccessManager *mgr = new QNetworkAccessManager(table);
              QUrl url("https://api.brevo.com/v3/smtp/email"); 
              QNetworkRequest request(url);
              
              QString apiKey = ConfigManager::getInstance().getBrevoKey();
              QString senderEmail = ConfigManager::getInstance().getSenderEmail();
              QString adminEmail = ConfigManager::getInstance().getAdminEmail();

              if (apiKey.isEmpty() || apiKey == "YOUR_BREVO_API_KEY_HERE") {
                  QMessageBox::critical(table->window(), "Email Error", "Brevo API Key is missing or invalid in config.json.\nPlease update the configuration to enable maintenance alerts.");
                  return;
              }

              request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
              request.setRawHeader("api-key", apiKey.toUtf8());

              QJsonObject sender;
              sender["name"] = "Oil Press Manager";
              sender["email"] = senderEmail;

              QJsonObject to;
              to["email"] = adminEmail;
              
              QJsonArray toArray;
              toArray.append(to);

              QJsonObject root;
              root["sender"] = sender;
              root["to"] = toArray;
              root["subject"] = "Alerte Maintenance Critique";
              root["htmlContent"] = "<html><body><h2>Alerte de Seuil Atteint</h2><p>" + alertMsg.replace("\n", "<br>") + "</p></body></html>";

              QObject::connect(mgr, &QNetworkAccessManager::finished, [mgr, table](QNetworkReply* reply) {
                  if (reply->error() == QNetworkReply::NoError) {
                      qDebug() << "Email sent successfully via direct URL.";
                      QMessageBox::information(table->window(), "Succès", "L'e-mail d'alerte a été envoyé avec succès à l'administrateur.");
                  } else {
                      QString errorResp = reply->readAll();
                      qDebug() << "Email failed:" << errorResp;
                      QMessageBox::critical(table->window(), "Erreur Email", "Échec de l'envoi de l'e-mail: " + reply->errorString() + "\n\n" + errorResp);
                  }
                  reply->deleteLater();
                  mgr->deleteLater();
              });

              mgr->post(request, QJsonDocument(root).toJson());

          } else {
              QMessageBox::information(table->window(), "Maintenance", "Tout est opérationnel.");
          }

          (*refreshMachineTable)();
      });

      QObject::connect(btnPrint, &QPushButton::clicked, [table]() {
          if (!table) return;

          QString fileName = QFileDialog::getSaveFileName(table->window(), "Exporter en PDF", "Rapport_Maintenance.pdf", "PDF Files (*.pdf)");
          if (fileName.isEmpty()) return;

          QString strStream;
          QTextStream out(&strStream);

          out << "<html><head><meta charset='utf-8'><style>"
              << "body { font-family: sans-serif; }"
              << ".header { background: #1D9E75; color: white; padding: 15px; text-align: center; }"
              << "table { width: 100%; border-collapse: collapse; margin-top: 15px; }"
              << "th { background: #eee; padding: 8px; border: 1px solid #ccc; font-size: 10px; }"
              << "td { padding: 6px; border: 1px solid #ddd; text-align: center; font-size: 10px; }"
              << ".title { font-weight: bold; margin-top: 20px; color: #1D9E75; border-bottom: 2px solid #1D9E75; padding-bottom: 5px; }"
              << "</style></head><body>"
              << "<div class='header'><h1>RAPPORT DE MAINTENANCE DÉTAILLÉ</h1></div>";

          auto generateSection = [&](QString title, QStringList states) {
              bool hasData = false;
              QString html = QString("<div class='title'>%1</div><table><thead><tr>").arg(title);
              html += "<th>ID</th><th>Nom</th><th>Type</th><th>État</th><th>Heures</th><th>Seuil</th><th>Localisation</th>";
              html += "</tr></thead><tbody>";

              for (int r = 0; r < table->rowCount(); ++r) {
                  if (table->isRowHidden(r)) continue;
                  
                  // Get ID and Status from table (reliable)
                  int id = table->model()->index(r, 0).data().toInt();
                  QString s = table->model()->index(r, 3).data().toString().trimmed();
                  
                  bool match = false;
                  for(const QString& target : states) if(s.compare(target, Qt::CaseInsensitive) == 0) match = true;
                  
                  if (match && id > 0) {
                      // FETCH FRESH DATA FROM DB
                      QSqlQuery query;
                      query.prepare("SELECT NOM_MACHINE, TYPE_MACHINE, ETAT_MACHINE, HEURESFONCTIONNEMENT, SEUILMAINTENANCE, LOCALISATION FROM MACHINE WHERE ID_MACHINE = :id");
                      query.bindValue(":id", id);
                      
                      if (query.exec() && query.next()) {
                          hasData = true;
                          html += "<tr>";
                          html += QString("<td>%1</td>").arg(id);
                          html += QString("<td>%1</td>").arg(query.value(0).toString().toHtmlEscaped());
                          html += QString("<td>%1</td>").arg(query.value(1).toString().toHtmlEscaped());
                          html += QString("<td>%1</td>").arg(query.value(2).toString().toHtmlEscaped());
                          html += QString("<td>%1</td>").arg(query.value(3).toString());
                          html += QString("<td>%1</td>").arg(query.value(4).toString());
                          html += QString("<td>%1</td>").arg(query.value(5).toString().toHtmlEscaped());
                          html += "</tr>";
                      }
                  }
              }
              html += "</tbody></table>";
              if (hasData) out << html;
              else out << QString("<p>%1 : Aucun actif trouvé.</p>").arg(title);
          };

          generateSection("ACTIFS OPÉRATIONNELS", {"Normal", "En marche", "Fonctionnel"});
          generateSection("ACTIFS EN PANNE / DANGER", {"En panne", "Critique", "En danger"});
          generateSection("ACTIFS EN MAINTENANCE", {"En maintenance", "Réparation"});

          out << "</body></html>";

          QPrinter printer(QPrinter::HighResolution);
          printer.setOutputFormat(QPrinter::PdfFormat);
          printer.setOutputFileName(fileName);
          printer.setPageSize(QPageSize(QPageSize::A4));
          printer.setPageOrientation(QPageLayout::Landscape);

          QTextDocument doc;
          doc.setHtml(strStream);
          doc.print(&printer);
          
          if (QFileInfo::exists(fileName)) {
              QMessageBox::information(table->window(), "Succès", "Rapport PDF généré à partir de la base de données !");
          }
      });

      (*refreshMachineTable)();

    } else if (name == "Analytics") {
      // --- Title ---
      QLabel *analyticsTitle = new QLabel("Machine Status Overview");
      analyticsTitle->setStyleSheet("font-size: 18px; font-weight: 700; color: #1a1a1a; margin-bottom: 8px;");
      cLayout->addWidget(analyticsTitle);

      // --- Refresh button ---
      QWidget *analyticsBar = new QWidget();
      QHBoxLayout *analyticsBarLayout = new QHBoxLayout(analyticsBar);
      analyticsBarLayout->setContentsMargins(0, 0, 0, 0);
      QPushButton *btnAnalyticsRefresh = new QPushButton("Refresh");
      btnAnalyticsRefresh->setStyleSheet(getButtonStyle());
      btnAnalyticsRefresh->setCursor(Qt::PointingHandCursor);
      btnAnalyticsRefresh->setFixedWidth(120);
      analyticsBarLayout->addStretch();
      analyticsBarLayout->addWidget(btnAnalyticsRefresh);
      cLayout->addWidget(analyticsBar);

      // --- Chart ---
      GenericBarChart *chart = new GenericBarChart("Machine Status Overview");
      chart->setMinimumHeight(350);

      // Helper lambda to load data into chart
      auto loadAnalytics = [chart]() {
          chart->clearBars();
          Machine m;
          QSqlQueryModel *model = m.afficher();
          int normal = 0, panne = 0, maintenance = 0;
          for (int i = 0; i < model->rowCount(); ++i) {
              QString status = model->data(model->index(i, 3)).toString();
              if (status == "Normal") normal++;
              else if (status == "En panne") panne++;
              else if (status == "En maintenance") maintenance++;
          }
          delete model;
          chart->addBar("Operational", normal,   QColor(46, 204, 113));
          chart->addBar("Broken",      panne,     QColor(231, 76, 60));
          chart->addBar("Maintenance", maintenance, QColor(241, 196, 15));
      };

      loadAnalytics(); // Initial load

      QObject::connect(btnAnalyticsRefresh, &QPushButton::clicked, [loadAnalytics]() {
          loadAnalytics();
      });

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
  QStringList tabNames = {"Product Hub", "Add Product", "Estimation", "Analytics", "AI Advisor"};
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
      QTableWidgetItem *idItem = new QTableWidgetItem();
      idItem->setData(Qt::EditRole, pid);
      productTable->setItem(i, 0, idItem);

      productTable->setItem(i, 1, new QTableWidgetItem(model->data(model->index(i, 1)).toDate().toString("yyyy-MM-dd")));

      QTableWidgetItem *qntItem = new QTableWidgetItem();
      qntItem->setData(Qt::EditRole, model->data(model->index(i, 2)).toInt());
      productTable->setItem(i, 2, qntItem);

      productTable->setItem(i, 3, new QTableWidgetItem(model->data(model->index(i, 3)).toString()));
      productTable->setItem(i, 4, new QTableWidgetItem(model->data(model->index(i, 4)).toString()));
      productTable->setItem(i, 5, new QTableWidgetItem(model->data(model->index(i, 5)).toString()));
      productTable->setItem(i, 6, new QTableWidgetItem(model->data(model->index(i, 6)).toString()));

      QTableWidgetItem *capItem = new QTableWidgetItem();
      capItem->setData(Qt::EditRole, model->data(model->index(i, 7)).toInt());
      productTable->setItem(i, 7, capItem);

      productTable->setItem(i, 8, new QTableWidgetItem(model->data(model->index(i, 8)).toString()));
      QTableWidgetItem *prixItem = new QTableWidgetItem(QString::number(model->data(model->index(i, 9)).toDouble(), 'f', 2) + " TND");
      prixItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
      productTable->setItem(i, 9, prixItem);

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
      productTable->setCellWidget(i, 10, actionWidget);

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

          auto addField = [&](const QString &lbl, const QString &val, QValidator *valdr = nullptr) {
              mainV->addWidget(new QLabel(lbl));
              QLineEdit *le = new QLineEdit(val);
              le->setStyleSheet("QLineEdit { background-color: #f9fafb; border: 1px solid #eaeaea; "
                                "border-radius: 8px; padding: 10px; font-size: 14px; }");
              if (valdr) le->setValidator(valdr);

              QObject::connect(le, &QLineEdit::textChanged, [le]() {
                  bool valid = true;
                  if (le->validator()) valid = le->hasAcceptableInput();
                  if (valid && !le->text().trimmed().isEmpty()) {
                      le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #3DDC84; border-radius: 8px; padding: 10px; font-size: 14px; }");
                  } else {
                      le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #d32f2f; border-radius: 8px; padding: 10px; font-size: 14px; }");
                  }
              });

              le->textChanged(le->text()); // Trigger initial validation highlighting
              mainV->addWidget(le);
              return le;
          };

          // Removing eIdC and eRef as requested (from original comment)
          QLineEdit *eDate = addField("Date Pressage:", productTable->item(i, 1)->text());
          QLineEdit *eQnt = addField("Quantité:", productTable->item(i, 2)->text(), new QIntValidator(1, 999999));
          QLineEdit *eRef = addField("Ref:", productTable->item(i, 3)->text(), new QRegularExpressionValidator(QRegularExpression("^[A-Za-z]{2}-\\d{2}$")));

          QDoubleValidator *viscVal = new QDoubleValidator(0.01, 10.0, 2);
          viscVal->setNotation(QDoubleValidator::StandardNotation);
          QLineEdit *eVisc = addField("Viscosité:", productTable->item(i, 4)->text(), viscVal);

          QLineEdit *eCol = addField("Couleur:", productTable->item(i, 5)->text());
          QLineEdit *eTst = addField("Test:", productTable->item(i, 6)->text());
          QLineEdit *eCap = addField("Capacité:", productTable->item(i, 7)->text(), new QIntValidator(1, 999999));
          QLineEdit *eIdM = addField("ID Machine:", productTable->item(i, 8)->text(), new QIntValidator(1, 999999));

          QDoubleValidator *prixValEdit = new QDoubleValidator(0.00, 999999.00, 2);
          prixValEdit->setNotation(QDoubleValidator::StandardNotation);
          // Strip "TND" if present for editing
          QString currentPrix = productTable->item(i, 9)->text().section(' ', 0, 0);
          QLineEdit *ePrix = addField("Prix Unitaire (TND):", currentPrix, prixValEdit);

          QPushButton *btnSave = new QPushButton("Save Changes");
          btnSave->setStyleSheet("QPushButton { background-color: #3DDC84; color: white; border: none; "
                                 "border-radius: 8px; padding: 12px; font-weight: 700; }");
          mainV->addWidget(btnSave);

          QObject::connect(btnSave, &QPushButton::clicked, [&dlg, eDate, eQnt, eRef, eVisc, eCol, eTst, eCap, eIdM, ePrix, pid, refreshProductTable]() {
              if (eQnt->text().isEmpty() || !eQnt->hasAcceptableInput() ||
                  eCap->text().isEmpty() || !eCap->hasAcceptableInput() ||
                  eIdM->text().isEmpty() || !eIdM->hasAcceptableInput() ||
                  eVisc->text().isEmpty() || !eVisc->hasAcceptableInput() ||
                  ePrix->text().isEmpty() || !ePrix->hasAcceptableInput() ||
                  eRef->text().trimmed().isEmpty() || !eRef->hasAcceptableInput() ||
                  eTst->text().trimmed().isEmpty()) {
                  QMessageBox::warning(&dlg, "Erreur de Saisie", "Veuillez vérifier les champs en rouge. Certaines valeurs sont invalides ou manquantes.");
                  return;
              }

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
              p.setPrixUnitaire(ePrix->text().toDouble());

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

      // 1. Top Buttons (Refresh & Print, Search & Sort)
      QWidget *topButtonsWidget = new QWidget();
      QHBoxLayout *topButtonsLayout = new QHBoxLayout(topButtonsWidget);
      topButtonsLayout->setContentsMargins(0, 0, 0, 0);
      topButtonsLayout->setSpacing(10);

      QLineEdit *searchEdit = new QLineEdit();
      searchEdit->setPlaceholderText("Search Product...");
      searchEdit->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #e0e0e0; border-radius: 8px; padding: 5px 10px; font-size: 13px; min-height: 35px; }");
      searchEdit->setFixedWidth(150);

      QComboBox *searchType = new QComboBox();
      searchType->addItems({"Ref", "Couleur", "Test"});
      searchType->setStyleSheet("QComboBox { background-color: #fcfcfc; border: 1px solid #e0e0e0; border-radius: 8px; padding: 5px 10px; font-size: 13px; min-height: 35px; }");
      searchType->setFixedWidth(100);

      QComboBox *sortType = new QComboBox();
      sortType->addItems({"All", "Quantité", "Capacité"});
      sortType->setStyleSheet("QComboBox { background-color: #fcfcfc; border: 1px solid #e0e0e0; border-radius: 8px; padding: 5px 10px; font-size: 13px; min-height: 35px; }");
      sortType->setFixedWidth(100);

      topButtonsLayout->addWidget(searchType);
      topButtonsLayout->addWidget(searchEdit);
      topButtonsLayout->addSpacing(10);
      QLabel *sortLabel = new QLabel("Sort by:");
      sortLabel->setStyleSheet("font-weight: 600; color: #555;");
      topButtonsLayout->addWidget(sortLabel);
      topButtonsLayout->addWidget(sortType);

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
                             "Ref", "Viscosité", "Couleur", "Test", "Capacité", "ID Machine", "Prix Unit. (TND)", "Actions"};
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

      // Connect Sort
      QObject::connect(sortType, &QComboBox::currentTextChanged, [productTablePtr](const QString &text) {
          QTableWidget *productTable = *productTablePtr;
          if (!productTable) return;
          if (text == "Quantité") productTable->sortItems(2, Qt::AscendingOrder);
          else if (text == "Capacité") productTable->sortItems(7, Qt::AscendingOrder);
          else if (text == "All") productTable->sortItems(0, Qt::AscendingOrder);
      });

      // Connect Filter
      auto updateFilter = [productTablePtr, searchEdit, searchType]() {
          QTableWidget *productTable = *productTablePtr;
          if (!productTable) return;
          QString lowerQuery = searchEdit->text().toLower();
          int col = 3; // "Ref"
          if (searchType->currentText() == "Couleur") col = 5;
          else if (searchType->currentText() == "Test") col = 6;

          for (int i = 0; i < productTable->rowCount(); ++i) {
              bool match = false;
              if (productTable->item(i, col)) {
                  match = productTable->item(i, col)->text().toLower().contains(lowerQuery);
              }
              productTable->setRowHidden(i, !match);
          }
      };

      QObject::connect(searchEdit, &QLineEdit::textChanged, updateFilter);
      QObject::connect(searchType, &QComboBox::currentTextChanged, updateFilter);

      // Connect Buttons
      QObject::connect(btnRefresh, &QPushButton::clicked, [refreshProductTable]() {
          (*refreshProductTable)();
      });

      QObject::connect(btnPrint, &QPushButton::clicked, [productTablePtr]() {
          QTableWidget *table = *productTablePtr;
          if (!table) return;

          QString strStream;
          QTextStream out(&strStream);

          const int rowCount = table->rowCount();
          const int columnCount = table->columnCount() - 1; // Exclude Actions col

          out <<  "<html>\n"
              "<head>\n"
              "<meta Content=\"Text/html; charset=utf-8\">\n"
              <<  QString("<title>%1</title>\n").arg("Liste des Produits")
              <<  "</head>\n"
              "<body bgcolor=#ffffff link=#5000A0>\n"
              "<h1 style=\"text-align: center; color: #3DDC84; font-family: Arial, sans-serif;\">Rapport des Produits</h1>\n"
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
              if (table->isRowHidden(row)) continue;
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

          QString defaultName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "_Rapport_Produits.pdf";
          QString fileName = QFileDialog::getSaveFileName(table->window(), "Exporter en PDF", QDir::currentPath() + "/" + defaultName, "PDF Files (*.pdf)");

          if (fileName.isEmpty()) return;

          QPrinter printer(QPrinter::PrinterResolution);
          printer.setOutputFormat(QPrinter::PdfFormat);
          printer.setOutputFileName(fileName);
          printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);

          document.print(&printer);
          QMessageBox::information(table->window(), "Succès", "Le PDF a été généré avec succès !\nEmplacement: " + fileName);
      });

    } else if (name == "Add Product") {
      QScrollArea *scrollArea = new QScrollArea();
      scrollArea->setWidgetResizable(true);
      scrollArea->setFrameShape(QFrame::NoFrame);
      scrollArea->setStyleSheet("QScrollArea { background-color: transparent; } QWidget#AddProductContainer { background-color: transparent; }");

      QWidget *formContainer = new QWidget();
      formContainer->setObjectName("AddProductContainer");
      QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
      formLayout->setSpacing(28);
      formLayout->setContentsMargins(10, 20, 30, 40);

      QLabel *titleLabel = new QLabel("Add New Product");
      titleLabel->setStyleSheet("font-size: 26px; font-weight: 700; color: #1a1a1a; margin-bottom: 10px;");
      formLayout->addWidget(titleLabel);

      auto attachVisualValidation = [](QLineEdit *le) {
          QObject::connect(le, &QLineEdit::textChanged, [le]() {
              bool valid = true;
              if (le->validator()) valid = le->hasAcceptableInput();
              if (valid && !le->text().trimmed().isEmpty()) {
                  le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #3DDC84; border-radius: 8px; padding: 10px 14px; font-size: 14px; min-height: 45px; }");
              } else {
                  le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #d32f2f; border-radius: 8px; padding: 10px 14px; font-size: 14px; min-height: 45px; }");
              }
          });
      };

      auto createField = [&](const QString &lbl, const QString &ph, QValidator *val = nullptr) {
          formLayout->addWidget(new QLabel(lbl));
          QLineEdit *le = new QLineEdit();
          le->setPlaceholderText(ph);
          le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #e0e0e0; "
                            "border-radius: 8px; padding: 10px 14px; font-size: 14px; min-height: 45px; }");
          if (val) le->setValidator(val);
          attachVisualValidation(le);
          formLayout->addWidget(le);
          return le;
      };

      formLayout->addWidget(new QLabel("Date Pressage:"));
      QDateEdit *iDate = new QDateEdit(QDate::currentDate());
      iDate->setCalendarPopup(true);
      iDate->setMaximumDate(QDate::currentDate());
      iDate->setDisplayFormat("yyyy-MM-dd");
      iDate->setStyleSheet("QDateEdit { background-color: #fcfcfc; border: 1px solid #e0e0e0; border-radius: 8px; padding: 10px 14px; font-size: 14px; min-height: 45px; }");
      formLayout->addWidget(iDate);

      QLineEdit *iQnt = createField("Quantité:", "e.g., 500", new QIntValidator(1, 999999));
      QLineEdit *iRef = createField("Ref:", "xx-nn (e.g., AB-12)", new QRegularExpressionValidator(QRegularExpression("^[A-Za-z]{2}-\\d{2}$")));

      QDoubleValidator *viscVal = new QDoubleValidator(0.01, 10.0, 2);
      viscVal->setNotation(QDoubleValidator::StandardNotation);
      QLineEdit *iVisc = createField("Viscosité (e.g., 0.85):", "0.85", viscVal);

      formLayout->addWidget(new QLabel("Couleur:"));
      QComboBox *iCol = new QComboBox();
      iCol->addItems({"Golden", "Yellow-Green", "Green", "Dark Green"});
      iCol->setStyleSheet("QComboBox { background-color: #fcfcfc; border: 1px solid #e0e0e0; border-radius: 8px; padding: 10px 14px; font-size: 14px; min-height: 45px; }");
      formLayout->addWidget(iCol);

      QLineEdit *iTst = createField("Test:", "Compliant");
      QLineEdit *iCap = createField("Capacité:", "e.g., 1000", new QIntValidator(1, 999999));
      QLineEdit *iIdM = createField("ID Machine:", "e.g., 101", new QIntValidator(1, 999999));

      QDoubleValidator *prixValAdd = new QDoubleValidator(0.00, 999999.00, 2);
      prixValAdd->setNotation(QDoubleValidator::StandardNotation);
      QLineEdit *iPrix = createField("Prix Unitaire (TND):", "e.g., 15.50", prixValAdd);

      formLayout->addSpacing(20);
      QPushButton *btnAdd = new QPushButton("Add Product");
      btnAdd->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3DDC84, stop:1 #2DB66F); "
                            "color: white; border: none; border-radius: 8px; padding: 12px; font-weight: 700; }");
      btnAdd->setCursor(Qt::PointingHandCursor);
      formLayout->addWidget(btnAdd);
      formLayout->addStretch();

        QObject::connect(btnAdd, &QPushButton::clicked, [=]() {
          if (iQnt->text().isEmpty() || !iQnt->hasAcceptableInput() ||
              iCap->text().isEmpty() || !iCap->hasAcceptableInput() ||
              iIdM->text().isEmpty() || !iIdM->hasAcceptableInput() ||
              iVisc->text().isEmpty() || !iVisc->hasAcceptableInput() ||
              iPrix->text().isEmpty() || !iPrix->hasAcceptableInput() ||
              iRef->text().trimmed().isEmpty() || !iRef->hasAcceptableInput() ||
              iTst->text().trimmed().isEmpty()) {
              QMessageBox::warning(nullptr, "Erreur de Saisie", "Veuillez vérifier les champs en rouge. Certaines valeurs sont invalides ou manquantes.");
              return;
          }

          Produit p;
          p.setDatePress(iDate->date());
          p.setQuantite(iQnt->text().toInt());
          p.setRef(iRef->text());
          p.setViscosite(iVisc->text());
          p.setCouleur(iCol->currentText());
          p.setTest(iTst->text());
          p.setCapacite(iCap->text().toInt());
          p.setIdMachine(iIdM->text().toInt());
          p.setPrixUnitaire(iPrix->text().toDouble());

          if (p.ajouter()) {
              QMessageBox::information(nullptr, "Success", "Product added successfully!");
              iQnt->clear(); iVisc->clear(); iTst->clear(); iIdM->clear(); iRef->clear(); iCap->clear(); iPrix->clear();
              iCol->setCurrentIndex(0);
              iDate->setDate(QDate::currentDate());

              // Switch to "Product Hub" tab first
              if (outNestedStack) outNestedStack->setCurrentIndex(0);
              if (!tabButtons.isEmpty()) tabButtons.first()->setChecked(true);

              // Refresh the table
              if (refreshProductTable) (*refreshProductTable)();
          } else {
              QMessageBox::critical(nullptr, "Error", p.getLastError());
          }
      });

      scrollArea->setWidget(formContainer);
      cLayout->addWidget(scrollArea);
    } else if (name == "Estimation") {
      QScrollArea *scrollArea = new QScrollArea();
      scrollArea->setWidgetResizable(true);
      scrollArea->setFrameShape(QFrame::NoFrame);
      scrollArea->setStyleSheet("QScrollArea { background-color: transparent; } QWidget#EstContainer { background-color: transparent; }");

      QWidget *estWidget = new QWidget();
      estWidget->setObjectName("EstContainer");
      QVBoxLayout *estLayout = new QVBoxLayout(estWidget);
      estLayout->setSpacing(20);
      estLayout->setContentsMargins(10, 20, 30, 40);

      QLabel *titleLabel = new QLabel("Olive Oil Price Estimation (API)");
      titleLabel->setStyleSheet("font-size: 26px; font-weight: 700; color: #1a1a1a; margin-bottom: 5px;");
      estLayout->addWidget(titleLabel);

      QLabel *subTitle = new QLabel("Connects to a predictive model to forecast future olive oil prices per liter.");
      subTitle->setStyleSheet("color: #7f8c8d; font-size: 14px; margin-bottom: 15px;");
      estLayout->addWidget(subTitle);

      QPushButton *btnPredict = new QPushButton("Run Prediction Model");
      btnPredict->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9); color: white; border: none; border-radius: 8px; padding: 12px 20px; font-weight: 700; font-size: 14px; } QPushButton:hover { background: #3cb0fd; }");
      btnPredict->setCursor(Qt::PointingHandCursor);
      btnPredict->setFixedWidth(250);

      QLabel *statusLabel = new QLabel("Status: Idle");
      statusLabel->setStyleSheet("color: #333; font-weight: bold; font-size: 14px;");

      QHBoxLayout *btnLayout = new QHBoxLayout();
      btnLayout->addWidget(btnPredict);
      btnLayout->addSpacing(15);
      btnLayout->addWidget(statusLabel);
      btnLayout->addStretch();
      estLayout->addLayout(btnLayout);
      estLayout->addSpacing(10);

      QTableWidget *predTable = new QTableWidget();
      predTable->setColumnCount(3);
      predTable->setHorizontalHeaderLabels({"Forecast Date", "Weather Prediction", "Predicted Price (DT / Liter)"});
      predTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      predTable->verticalHeader()->setVisible(false);
      predTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      predTable->setStyleSheet("QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; gridline-color: transparent; border-radius: 8px; alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; color: #666; text-transform: uppercase; font-size: 12px; }"
          "QTableWidget::item { padding: 12px; border-bottom: 1px solid #f5f5f5; color: #333; }");
      estLayout->addWidget(predTable);

      QNetworkAccessManager *manager = new QNetworkAccessManager(page);

      QObject::connect(btnPredict, &QPushButton::clicked, [manager, statusLabel, predTable]() {
          statusLabel->setText("Status: Connecting to forecasting API...");
          statusLabel->setStyleSheet("color: #f39c12; font-weight: bold; font-size: 14px;");
          predTable->setRowCount(0);

          QString apiKey = "AIzaSyCYkAxn6q9Cg3N0Ay3zuUtau0OX2RTZvfY"; // Reusing the valid key provided
          QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + apiKey);

          QString prompt = "You are a highly advanced agricultural economic model forecasting olive oil prices. "
                           "Generate a 6-month price forecast for Olive Oil in Tunisia (currency: DT/Liter) starting from next month. "
                           "The current base price is 28.50 DT. Factor in typical seasonal weather trends and market variables. "
                           "Return ONLY a valid JSON array of 6 objects. Do not include markdown formatting or explanation. "
                           "Example format: [{\"date\":\"2024-06\", \"weather\":\"Hot and Dry\", \"price\":28.75}, ...]";

          QNetworkRequest request(url);
          request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

          QJsonObject part; part["text"] = prompt;
          QJsonArray parts; parts.append(part);
          QJsonObject contentObj; contentObj["parts"] = parts;
          QJsonArray contents; contents.append(contentObj);
          QJsonObject payload; payload["contents"] = contents;

          QNetworkReply *reply = manager->post(request, QJsonDocument(payload).toJson());

          QObject::connect(reply, &QNetworkReply::finished, [reply, statusLabel, predTable]() {
              if (reply->error() == QNetworkReply::NoError) {
                  QByteArray response = reply->readAll();
                  QJsonDocument doc = QJsonDocument::fromJson(response);
                  QJsonArray candidates = doc.object()["candidates"].toArray();
                  if (!candidates.isEmpty()) {
                      QString rawText = candidates[0].toObject()["content"].toObject()["parts"].toArray()[0].toObject()["text"].toString();
                      rawText = rawText.replace("```json", "").replace("```", "").trimmed(); // Clean markdown if present
                      
                      QJsonDocument forecastDoc = QJsonDocument::fromJson(rawText.toUtf8());
                      if (forecastDoc.isArray()) {
                          QJsonArray forecastArray = forecastDoc.array();
                          predTable->setRowCount(forecastArray.size());
                          statusLabel->setText("Status: Prediction Complete (AI Forecast)");
                          statusLabel->setStyleSheet("color: #27ae60; font-weight: bold; font-size: 14px;");

                          for (int i = 0; i < forecastArray.size(); ++i) {
                              QJsonObject item = forecastArray[i].toObject();
                              predTable->setItem(i, 0, new QTableWidgetItem(item["date"].toString()));
                              predTable->setItem(i, 1, new QTableWidgetItem(item["weather"].toString()));
                              predTable->setItem(i, 2, new QTableWidgetItem(QString("%1 DT").arg(item["price"].toDouble(), 0, 'f', 2)));
                          }
                      } else {
                          statusLabel->setText("Status: Forecasting Error (Invalid Model Output)");
                          statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold; font-size: 14px;");
                      }
                  }
              } else {
                  statusLabel->setText("Status: API Error (" + reply->errorString() + ") - Using Fallback Simulation");
                  statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold; font-size: 14px;");

                  // Fallback Simulation logic if the network fails
                  double basePrice = 28.50;
                  predTable->setRowCount(6);
                  QDate d = QDate::currentDate();
                  for (int i = 0; i < 6; ++i) {
                      d = d.addMonths(1);
                      double factor = 1.0 + ((rand() % 200) - 50) / 1000.0;
                      basePrice *= factor;
                      predTable->setItem(i, 0, new QTableWidgetItem(d.toString("yyyy-MM")));
                      predTable->setItem(i, 1, new QTableWidgetItem("Unknown Weather"));
                      predTable->setItem(i, 2, new QTableWidgetItem(QString("%1 DT (Fallback)").arg(basePrice, 0, 'f', 2)));
                  }
              }
              reply->deleteLater();
          });
      });

      scrollArea->setWidget(estWidget);
      cLayout->addWidget(scrollArea);

    } else if (name == "Analytics") {
      QWidget *analyticsContainer = new QWidget();
      QVBoxLayout *analyticsLayout = new QVBoxLayout(analyticsContainer);
      analyticsLayout->setContentsMargins(0, 0, 0, 0);

      auto refreshAnalytics = [analyticsLayout]() {
          QLayoutItem *child;
          while ((child = analyticsLayout->takeAt(0)) != nullptr) {
              if (child->widget()) child->widget()->deleteLater();
              delete child;
          }

          GenericBarChart *chart = new GenericBarChart("Stock Overview by Capacity");
          QSqlQuery q("SELECT CAPACITE, SUM(QUANTITE) FROM PRODUIT GROUP BY CAPACITE ORDER BY SUM(QUANTITE) DESC");

          QList<QColor> colors = {QColor(61, 220, 132), QColor(52, 152, 219), QColor(241, 196, 15), QColor(155, 89, 182), QColor(231, 76, 60)};
          int count = 0;

          while (q.next() && count < 5) {
              QString capacityLabel = QString::number(q.value(0).toInt()) + " L";
              int totalStock = q.value(1).toInt();
              chart->addBar("Capacity " + capacityLabel, totalStock, colors[count % colors.size()]);
              count++;
          }

          if (count == 0) {
              chart->addBar("No Stock Data", 0, QColor(149, 165, 166));
          }

          analyticsLayout->addWidget(chart);
      };

      refreshAnalytics(); // Initial load

      QObject::connect(outNestedStack, &QStackedWidget::currentChanged, [refreshAnalytics](int index) {
          if (index == 3) { // 3 is the index for Analytics tab in Product Management
              refreshAnalytics();
          }
      });

      cLayout->addWidget(analyticsContainer);
    } else if (name == "AI Advisor") {
      QWidget *aiContainer = new QWidget();
      QVBoxLayout *aiLayout = new QVBoxLayout(aiContainer);
      aiLayout->setContentsMargins(20, 20, 20, 20);
      aiLayout->setSpacing(15);

      // Chat history area
      QTextEdit *chatHistory = new QTextEdit();
      chatHistory->setReadOnly(true);
      chatHistory->setStyleSheet("QTextEdit { background-color: #fcfcfc; border: 1px solid #e0e0e0; border-radius: 8px; padding: 10px; font-size: 14px; color: #333; }");
      chatHistory->append("<b>System:</b> Hello! I am the Oil Press AI Advisor. Ask me anything about improving your oil products.");

      // Input area
      QHBoxLayout *inputLayout = new QHBoxLayout();
      QLineEdit *userInput = new QLineEdit();
      userInput->setPlaceholderText("Ask for advice to improve oil grade...");
      userInput->setStyleSheet(getInputStyle());
      QPushButton *sendBtn = new QPushButton("Ask AI");
      sendBtn->setCursor(Qt::PointingHandCursor);
      sendBtn->setStyleSheet(getButtonStyle());

      inputLayout->addWidget(userInput);
      inputLayout->addWidget(sendBtn);

      aiLayout->addWidget(chatHistory, 1);
      aiLayout->addLayout(inputLayout);

      cLayout->addWidget(aiContainer);

      QNetworkAccessManager *netManager = new QNetworkAccessManager(aiContainer);

      auto askGemini = [=]() {
          QString apiKey = "AIzaSyCYkAxn6q9Cg3N0Ay3zuUtau0OX2RTZvfY";
          QString query = userInput->text().trimmed();
          if (query.isEmpty()) return;

          chatHistory->append("<b>You:</b> " + query.toHtmlEscaped() + "<br/>");
          userInput->clear();

          // Build context from DB
          QString dbContext = "Here is the current relevant stock in the oil press:\n";
          QSqlQuery q("SELECT CAPACITE, QUANTITE, COULEUR, VISCOSITE FROM PRODUIT ORDER BY ID_CONTENAIR DESC LIMIT 10");
          int count = 0;
          while (q.next() && count < 10) {
              dbContext += QString("- Olives/Oil capacity %1, qty %2, color %3, viscosity %4\n")
                            .arg(q.value(0).toString())
                            .arg(q.value(1).toString())
                            .arg(q.value(2).toString())
                            .arg(q.value(3).toString());
              count++;
          }
          if (count == 0) dbContext += "No recent products found.\n";

          QString fullPrompt = "You are an expert olive oil press consultant. Act as an advisor.\n" + dbContext + "\nUser question: " + query;

          QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + apiKey);
          QNetworkRequest request(url);
          request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

          QJsonObject part;
          part["text"] = fullPrompt;
          QJsonArray parts;
          parts.append(part);
          QJsonObject contentObj;
          contentObj["parts"] = parts;
          QJsonArray contents;
          contents.append(contentObj);
          QJsonObject payload;
          payload["contents"] = contents;

          QNetworkReply *reply = netManager->post(request, QJsonDocument(payload).toJson());
          sendBtn->setEnabled(false);

          QObject::connect(reply, &QNetworkReply::finished, [reply, chatHistory, sendBtn]() {
              sendBtn->setEnabled(true);
              if (reply->error() == QNetworkReply::NoError) {
                  QByteArray response = reply->readAll();
                  QJsonDocument doc = QJsonDocument::fromJson(response);
                  QJsonObject obj = doc.object();
                  QJsonArray candidates = obj["candidates"].toArray();
                  if (!candidates.isEmpty()) {
                      QJsonObject firstCandidate = candidates[0].toObject();
                      QJsonObject content = firstCandidate["content"].toObject();
                      QJsonArray parts = content["parts"].toArray();
                      if (!parts.isEmpty()) {
                          QString botText = parts[0].toObject()["text"].toString();
                          QString formattedText = botText.toHtmlEscaped();
                          
                          // Convert Markdown-like syntax to HTML
                          formattedText.replace(QRegularExpression("\\*\\*(.*?)\\*\\*"), "<b>\\1</b>"); // Bold
                          formattedText.replace(QRegularExpression("\\*(?!\\s)(.*?)(?<!\\s)\\*"), "<i>\\1</i>"); // Italic
                          formattedText.replace(QRegularExpression("(^|\n)[\\*\\-]\\s"), "\\1&bull; "); // Bullets
                          formattedText.replace("\n", "<br/>").replace("\r", ""); // Newlines
                          
                          chatHistory->append("<b>AI Advisor:</b><br/>" + formattedText + "<br/>");
                      }
                  } else {
                      chatHistory->append("<span style='color: orange;'><b>AI Advisor:</b> Received empty response.</span><br/>");
                  }
              } else {
                  chatHistory->append("<span style='color: red;'><b>Error:</b> " + reply->errorString() + "</span><br/>");
              }
              reply->deleteLater();
          });
      };

      QObject::connect(sendBtn, &QPushButton::clicked, askGemini);
      QObject::connect(userInput, &QLineEdit::returnPressed, askGemini);

    }
    if (name != "Add Product" && name != "Estimation" && name != "AI Advisor") {
    }
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
      btnRefresh->setObjectName("btnRefreshPersonnel");
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
      personnelTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
      personnelTable->horizontalHeader()->setStretchLastSection(true);
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
          int colCount = model->columnCount();

          personnelTable->setColumnCount(colCount + 1); // +1 for Actions
          QStringList headers;
          for (int j = 0; j < colCount; ++j) {
              headers << model->headerData(j, Qt::Horizontal).toString();
          }
          headers << "Actions";
          personnelTable->setHorizontalHeaderLabels(headers);
          personnelTable->setColumnHidden(0, true); // Hide ID (CIN)

          personnelTable->setRowCount(rowCount);

          for (int i = 0; i < rowCount; ++i) {
              for (int j = 0; j < colCount; ++j) {
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
                         QPushButton *btn = personnelTable->window()->findChild<QPushButton*>("btnRefreshPersonnel");
                         if (btn) btn->click();
                      }
                  }
              });

              QObject::connect(btnModify, &QPushButton::clicked, [personnelTable, i]() {
                  QDialog *dialog = new QDialog();
                  dialog->setWindowTitle("Modify Personnel");
                  dialog->setMinimumWidth(450);
                  dialog->setStyleSheet("QDialog { background-color: #f4f6f8; }");

                  QVBoxLayout *dLayout = new QVBoxLayout(dialog);
                  dLayout->setContentsMargins(25, 25, 25, 25);
                  dLayout->setSpacing(12);

                  QLabel *title = new QLabel("Update Employee Record");
                  title->setStyleSheet("font-size: 20px; font-weight: 700; color: #1a1a1a; margin-bottom: 15px;");
                  dLayout->addWidget(title);

                  QString inputStyle = getInputStyle();

                  QLineEdit *cinInput = new QLineEdit(personnelTable->item(i, 0)->text());
                  cinInput->setEnabled(false);
                  QLineEdit *nameInput = new QLineEdit(personnelTable->item(i, 1)->text());
                  QLineEdit *salInput = new QLineEdit(personnelTable->item(i, 2)->text());
                  QLineEdit *addrInput = new QLineEdit(personnelTable->item(i, 3)->text());
                  QLineEdit *telInput = new QLineEdit(personnelTable->item(i, 4)->text());
                  QLineEdit *expInput = new QLineEdit(personnelTable->item(i, 5)->text());

                  QComboBox *gradeInput = new QComboBox();
                  gradeInput->addItems({"Junior", "Senior", "Lead", "Principal", "Chief"});
                  gradeInput->setCurrentText(personnelTable->item(i, 6)->text());

                  QComboBox *roleInput = new QComboBox();
                  roleInput->addItems({"Stock Management", "Product Management", "Maintenance Management", "Personnel Management", "Order Management", "Financial Management"});
                  roleInput->setCurrentText(personnelTable->item(i, 7)->text());

                  auto addField = [&](QString label, QWidget *w) {
                      QLabel *lbl = new QLabel(label);
                      lbl->setStyleSheet("color: #555; font-weight: 600; font-size: 13px;");
                      dLayout->addWidget(lbl);
                      w->setStyleSheet(inputStyle);
                      if (qobject_cast<QLineEdit*>(w) || qobject_cast<QComboBox*>(w)) {
                          w->setFixedHeight(42);
                      }
                      dLayout->addWidget(w);
                  };

                  addField("CIN (Identifier - Cannot be changed):", cinInput);
                  addField("Full Name:", nameInput);
                  addField("Salary (Gross):", salInput);
                  addField("Address:", addrInput);
                  addField("Phone Number:", telInput);
                  addField("Experience (Years):", expInput);
                  addField("Grade:", gradeInput);
                  addField("Role:", roleInput);

                  dLayout->addSpacing(15);

                  QHBoxLayout *btnLayout = new QHBoxLayout();
                  QPushButton *btnCancel = new QPushButton("Cancel");
                  btnCancel->setStyleSheet("QPushButton { background-color: #ffffff; color: #333; border: 1px solid #ccc; border-radius: 8px; font-weight: bold; } QPushButton:hover { background-color: #f6f6f6; }");
                  btnCancel->setCursor(Qt::PointingHandCursor);
                  btnCancel->setFixedHeight(45);

                  QPushButton *btnSave = new QPushButton("Update Record");
                  btnSave->setStyleSheet(getButtonStyle());
                  btnSave->setCursor(Qt::PointingHandCursor);
                  btnSave->setFixedHeight(45);

                  btnLayout->addWidget(btnCancel);
                  btnLayout->addWidget(btnSave);
                  dLayout->addLayout(btnLayout);

                  QObject::connect(btnCancel, &QPushButton::clicked, dialog, &QDialog::reject);

                  QObject::connect(btnSave, &QPushButton::clicked, [=]() {
                      Personnel p;
                      p.setCin(cinInput->text().toInt());
                      p.setNom(nameInput->text());
                      p.setSalaire(salInput->text().toDouble());
                      p.setAdresse(addrInput->text());
                      p.setTel(telInput->text());
                      p.setExperience(expInput->text().toInt());
                      p.setGrade(gradeInput->currentText());
                      p.setRole(roleInput->currentText());

                      if (p.modifier()) {
                          QMessageBox::information(dialog, "Success", "Record updated successfully.");
                          dialog->accept();
                          QPushButton *btnRef = personnelTable->window()->findChild<QPushButton*>("btnRefreshPersonnel");
                          if (btnRef) btnRef->click();
                      } else {
                          QMessageBox::critical(dialog, "Database Error", p.getLastError());
                      }
                  });
                  dialog->exec();
              });

              personnelTable->setCellWidget(i, colCount, actionWidget);
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

      auto attachVisualValidation = [](QLineEdit *le) {
          QObject::connect(le, &QLineEdit::textChanged, [le]() {
              bool valid = true;
              if (le->validator()) valid = le->hasAcceptableInput();
              if (valid && !le->text().trimmed().isEmpty()) {
                  le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #3DDC84; border-radius: 8px; padding: 10px 14px; font-size: 14px; min-height: 45px; }");
              } else {
                  le->setStyleSheet("QLineEdit { background-color: #fcfcfc; border: 1px solid #d32f2f; border-radius: 8px; padding: 10px 14px; font-size: 14px; min-height: 45px; }");
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
          if (!cinInput->hasAcceptableInput() || !nameInput->hasAcceptableInput() ||
              !salInput->hasAcceptableInput() || !telInput->hasAcceptableInput() ||
              !expInput->hasAcceptableInput() || addrInput->text().trimmed().isEmpty()) {
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

          if (p.ajouter()) {
              QMessageBox::information(nullptr, "Success", "Employee added successfully.");
              cinInput->clear(); nameInput->clear(); salInput->clear(); addrInput->clear();
              telInput->clear(); expInput->clear();
              outNestedStack->setCurrentIndex(0);

              QPushButton *btn = outNestedStack->window()->findChild<QPushButton*>("btnRefreshPersonnel");
              if (btn) btn->click();
          } else {
              QMessageBox::critical(nullptr, "Error", p.getLastError());
          }
      });

      cLayout->addWidget(formContainer);

    } else if (name == "Payroll") {
      QWidget *payrollWidget = new QWidget();
      QVBoxLayout *payrollLayout = new QVBoxLayout(payrollWidget);
      payrollLayout->setContentsMargins(0, 0, 0, 0);
      payrollLayout->setSpacing(20);

      QLabel *title = new QLabel("Monthly Payroll Overview");
      title->setStyleSheet("font-size: 22px; font-weight: 700; color: #1a1a1a;");
      payrollLayout->addWidget(title);

      QWidget *summaryCard = new QWidget();
      summaryCard->setStyleSheet(getCardStyle());
      QVBoxLayout *cardLayout = new QVBoxLayout(summaryCard);
      cardLayout->setSpacing(10);

      QLabel *lblTotal = new QLabel("Loading...");
      lblTotal->setStyleSheet("font-size: 32px; font-weight: 900; color: #2ecc71;");

      QLabel *lblCount = new QLabel("Loading...");
      lblCount->setStyleSheet("font-size: 16px; color: #7f8c8d; font-weight: 700;");

      cardLayout->addWidget(lblTotal);
      cardLayout->addWidget(lblCount);
      payrollLayout->addWidget(summaryCard);

      QTableWidget *breakdownTable = new QTableWidget();
      breakdownTable->setColumnCount(4);
      breakdownTable->setHorizontalHeaderLabels({"CIN", "Employee Name", "Role", "Monthly Salary"});
      breakdownTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      breakdownTable->verticalHeader()->setVisible(false);
      breakdownTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
      breakdownTable->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: #ffffff; gridline-color: transparent; border-radius: 8px; alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; color: #666; text-transform: uppercase; font-size: 12px; }"
          "QTableWidget::item { padding: 12px; border-bottom: 1px solid #f5f5f5; color: #333; }"
      );
      payrollLayout->addWidget(breakdownTable);

      QPushButton *btnRefreshPayroll = new QPushButton("Recalculate Current Payroll");
      btnRefreshPayroll->setStyleSheet(getButtonStyle());
      btnRefreshPayroll->setCursor(Qt::PointingHandCursor);
      btnRefreshPayroll->setFixedWidth(250);
      btnRefreshPayroll->setFixedHeight(45);

      QHBoxLayout *btnLayout = new QHBoxLayout();
      btnLayout->addStretch();
      btnLayout->addWidget(btnRefreshPayroll);
      payrollLayout->addLayout(btnLayout);

      auto refreshPayrollData = [lblTotal, lblCount, breakdownTable]() {
          double nTotal = 0.0;
          int nCount = 0;
          QSqlQuery q("SELECT COUNT(ID_PERSONNEL), SUM(SALAIRE_BRUT) FROM PERSONNEL");
          if (q.next()) {
              nCount = q.value(0).toInt();
              nTotal = q.value(1).toDouble();
          }
          lblTotal->setText(QString("%1 TND / Month").arg(nTotal, 0, 'f', 2));
          lblCount->setText(QString("Active Staff Contributing to Payroll: %1").arg(nCount));

          breakdownTable->setRowCount(0);
          QSqlQuery lq("SELECT ID_PERSONNEL, NOM_PERSONNEL, ROLE, SALAIRE_BRUT FROM PERSONNEL");
          int row = 0;
          while (lq.next()) {
              breakdownTable->insertRow(row);
              breakdownTable->setItem(row, 0, new QTableWidgetItem(lq.value(0).toString()));
              breakdownTable->setItem(row, 1, new QTableWidgetItem(lq.value(1).toString()));
              breakdownTable->setItem(row, 2, new QTableWidgetItem(lq.value(2).toString()));

              QTableWidgetItem *moneyItem = new QTableWidgetItem(QString("%1 TND").arg(lq.value(3).toDouble(), 0, 'f', 2));
              moneyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
              breakdownTable->setItem(row, 3, moneyItem);

              row++;
          }
      };

      QObject::connect(btnRefreshPayroll, &QPushButton::clicked, refreshPayrollData);
      refreshPayrollData();

      cLayout->addWidget(payrollWidget);
    } else if (name == "Analytics") {
      QScrollArea *scrollArea = new QScrollArea();
      scrollArea->setWidgetResizable(true);
      scrollArea->setFrameShape(QFrame::NoFrame);
      scrollArea->setStyleSheet("QScrollArea { background-color: transparent; } QWidget#StatsContainer { background-color: transparent; }");

      QWidget *statsContainer = new QWidget();
      statsContainer->setObjectName("StatsContainer");
      QVBoxLayout *statsLayout = new QVBoxLayout(statsContainer);
      statsLayout->setSpacing(30);

      QPushButton *btnRefreshAnalytics = new QPushButton("Refresh Analytics");
      btnRefreshAnalytics->setStyleSheet(getButtonStyle());
      btnRefreshAnalytics->setCursor(Qt::PointingHandCursor);
      btnRefreshAnalytics->setFixedWidth(200);
      btnRefreshAnalytics->setFixedHeight(45);

      QHBoxLayout *btnLayout = new QHBoxLayout();
      btnLayout->addStretch();
      btnLayout->addWidget(btnRefreshAnalytics);
      statsLayout->addLayout(btnLayout);

      QVBoxLayout *chartsLayout = new QVBoxLayout();
      statsLayout->addLayout(chartsLayout);

      auto refreshAnalytics = [chartsLayout]() {
          // Clear old charts cleanly
          QLayoutItem *child;
          while ((child = chartsLayout->takeAt(0)) != nullptr) {
              if (child->widget()) {
                  child->widget()->deleteLater();
              }
              delete child;
          }

          GenericBarChart *chart1 = new GenericBarChart("Employee Headcount by Role");
          QSqlQuery q1("SELECT ROLE, COUNT(*) FROM PERSONNEL GROUP BY ROLE");
          QList<QColor> colors = {QColor(52, 152, 219), QColor(46, 204, 113), QColor(241, 196, 15), QColor(155, 89, 182), QColor(231, 76, 60), QColor(26, 188, 156)};
          int cIdx = 0;
          while (q1.next()) {
              chart1->addBar(q1.value(0).toString(), q1.value(1).toInt(), colors[cIdx % colors.size()]);
              cIdx++;
          }
          QWidget *c1Wrapper = new QWidget();
          c1Wrapper->setStyleSheet(getCardStyle());
          QVBoxLayout *v1 = new QVBoxLayout(c1Wrapper);
          v1->addWidget(chart1);
          chartsLayout->addWidget(c1Wrapper);

          GenericBarChart *chart2 = new GenericBarChart("Average Salary by Role");
          QSqlQuery q2("SELECT ROLE, AVG(SALAIRE_BRUT) FROM PERSONNEL GROUP BY ROLE");
          cIdx = 0;
          while (q2.next()) {
              chart2->addBar(q2.value(0).toString(), q2.value(1).toInt(), colors[(cIdx+3) % colors.size()]);
              cIdx++;
          }
          QWidget *c2Wrapper = new QWidget();
          c2Wrapper->setStyleSheet(getCardStyle());
          QVBoxLayout *v2 = new QVBoxLayout(c2Wrapper);
          v2->addWidget(chart2);
          chartsLayout->addWidget(c2Wrapper);
      };

      QObject::connect(btnRefreshAnalytics, &QPushButton::clicked, refreshAnalytics);
      refreshAnalytics();

      scrollArea->setWidget(statsContainer);
      cLayout->addWidget(scrollArea);

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

  // Initialize System Tray
  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setIcon(QIcon(":/logo.png"));
  trayIcon->setToolTip("Oil Press Manager Alerts");
  trayIcon->show();

  m_lastEmailTime = QDateTime(); // Initialise à vide

  // --- Initialize Serial Port for Arduino ---
  serial = new QSerialPort(this);
  
  bool arduino_is_available = false;
  QString arduino_port_name = "";

  for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
      if (!info.isNull() && !info.portName().isEmpty()) {
          arduino_is_available = true;
          arduino_port_name = info.portName();
          // Prioritize USB-connected devices (e.g. Arduino)
          if (info.hasVendorIdentifier() && info.hasProductIdentifier()) {
              break;
          }
      }
  }

  if (arduino_is_available) {
      serial->setPortName(arduino_port_name);
      qDebug() << "[SERIAL] Automatically selecting port:" << arduino_port_name;
  } else {
      qDebug() << "[SERIAL] CRITICAL: No COM ports found connected to the PC.";
  }

  serial->setBaudRate(QSerialPort::Baud9600);
  serial->setDataBits(QSerialPort::Data8);
  serial->setParity(QSerialPort::NoParity);
  serial->setStopBits(QSerialPort::OneStop);
  serial->setFlowControl(QSerialPort::NoFlowControl);
  
  if (serial->open(QIODevice::ReadWrite)) {
      qDebug() << "Serial Port Opened Successfully on" << serial->portName();
      connect(serial, &QSerialPort::readyRead, this, &MainWindow::handleSerialDataReady);
  } else {
      qDebug() << "Failed to open Serial Port.";
  }

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

  // Initial Stock Alert Check
  QTimer::singleShot(2000, this, [this]() { checkStockAlerts(true); });
}

void MainWindow::checkStockAlerts(bool silent) {
    QSqlQuery query("SELECT NOM_ARTICLE, QUANTITE FROM ARTICLE WHERE QUANTITE <= 10");
    QStringList lowStockItems;
    while (query.next()) {
        lowStockItems << QString("%1 (%2 left)").arg(query.value(0).toString(), query.value(1).toString());
    }

    if (!lowStockItems.isEmpty()) {
        QString msg = "The following items have reached the low stock threshold (10):\n\n" + lowStockItems.join("\n");
        trayIcon->showMessage("Low Stock Alert",
                              QString("%1 items require restocking.").arg(lowStockItems.size()),
                              QSystemTrayIcon::Warning, 3000);

        if (!silent) {
            QMessageBox::warning(this, "Inventory Alert", msg);
        }
    } else if (!silent) {
        QMessageBox::information(this, "Inventory Check", "All items are above the minimum threshold.");
    }
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

void MainWindow::handleSerialDataReady()
{
    if (serial->canReadLine()) {
        QByteArray incomingData = serial->readLine().trimmed();
        QString incomingText = QString::fromUtf8(incomingData);
        
        bool isGas = incomingText.startsWith("ALERT_GAS:");
        bool isSmoke = incomingText.startsWith("ALERT_SMOKE:");

        if (isGas || isSmoke) {
            if (m_isAlertShowing) return; // Prevent stacking multiple alerts
            m_isAlertShowing = true;

            QString machineIdStr = incomingText.mid(isGas ? 10 : 12);
            int machineId = machineIdStr.toInt();
            // User requested Smoke as default even for generic Gas messages
            QString typeStr = "Smoke"; 
            qDebug() << "[SERIAL] " << typeStr << " Alert for Machine ID:" << machineId;

            // 1. Update MACHINE state
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE MACHINE SET ETAT_MACHINE = 'En danger' WHERE ID_MACHINE = :id");
            updateQuery.bindValue(":id", machineId);
            updateQuery.exec();

            // 2. Insert ALERT record
            QSqlQuery insertQuery;
            insertQuery.prepare("INSERT INTO ALERT (ALERT_ID, ID_MACHINE, ALERT_DATETIME) VALUES (ALERT_SEQ.NEXTVAL, :id, CURRENT_TIMESTAMP)");
            insertQuery.bindValue(":id", machineId);
            insertQuery.exec();

            // 3. Email Logic (with cooldown)
            bool canSendEmail = !m_lastEmailTime.isValid() || m_lastEmailTime.secsTo(QDateTime::currentDateTime()) > 60;
            if (canSendEmail) {
                QSqlQuery emailQuery("SELECT EMAIL FROM PERSONNEL WHERE LOWER(ROLE) LIKE '%admin%' AND EMAIL IS NOT NULL AND ROWNUM = 1");
                if (emailQuery.next()) {
                    QString adminEmail = emailQuery.value(0).toString();
                    EmailAPI* emailApi = new EmailAPI(this);
                    emailApi->setCredentials(ConfigManager::getInstance().getBrevoKey());
                    
                    QString subject = QString("URGENT: %1 Detected on Machine %2").arg(typeStr).arg(machineId);
                    QString body = QString("A %1 leak has been detected on Machine ID %2.\n\nThe machine status is set to 'En danger'.").arg(typeStr.toLower()).arg(machineId);
                    
                    connect(emailApi, &EmailAPI::finished, emailApi, &QObject::deleteLater);
                    emailApi->sendEmail(adminEmail, subject, body);
                    m_lastEmailTime = QDateTime::currentDateTime();
                }
            }

            // 4. Show Custom Premium Popup (Defaulting to Smoke as requested)
            GasAlertWidget *alertDlg = new GasAlertWidget(machineId, GasAlertWidget::Smoke, this);
            alertDlg->exec(); 
            delete alertDlg;
            m_isAlertShowing = false; // Allow next alert to show
            
        } else {
            // New: Machine Breakdown Reporting Logic
            bool isNumericId;
            int mId = incomingText.toInt(&isNumericId);
            bool breakdownProcessed = false;

            if (isNumericId) {
                QSqlQuery mQuery;
                mQuery.prepare("SELECT LOCALISATION FROM MACHINE WHERE ID_MACHINE = :id");
                mQuery.bindValue(":id", mId);

                if (mQuery.exec() && mQuery.next()) {
                    QString location = mQuery.value(0).toString();
                    
                    // Update machine state to 'En panne'
                    QSqlQuery uQuery;
                    uQuery.prepare("UPDATE MACHINE SET ETAT_MACHINE = 'En panne' WHERE ID_MACHINE = :id");
                    uQuery.bindValue(":id", mId);
                    
                    if (uQuery.exec()) {
                        qDebug() << "[SERIAL] Machine Breakdown Reported! ID:" << mId << "Loc:" << location;
                        
                        // NEW PROTOCOL: S<ID>,<Location>\n
                        QString response = "S" + QString::number(mId) + "," + location + "\n";
                        serial->write(response.toUtf8());
                        
                        // Show confirmation in console only (no popup on PC)
                        qDebug() << "[SERIAL] Machine " << mId << " updated to 'En panne'. Response sent to OLED.";
                        
                        breakdownProcessed = true;
                    }
                } else {
                    // NEW PROTOCOL: F\n for "Not Found"
                    qDebug() << "[SERIAL] Machine ID" << mId << "not found. Sending F.";
                    serial->write("F\n");
                    breakdownProcessed = true; // Managed as a breakdown attempt
                }
            }

            if (!breakdownProcessed) {
                // Existing logic for RFID/EnterCode
                QString uid = incomingText;
                qDebug() << "[SERIAL TX/RX] Received UID from Arduino:" << uid;

                // Find personnel's name and grade using RFID
                QSqlQuery query;
                query.prepare("SELECT NOM_PERSONNEL, GRADE FROM PERSONNEL WHERE RFID = :uid");
                query.bindValue(":uid", uid);

                if (query.exec() && query.next()) {
                    QString name = query.value(0).toString();
                    QString grade = query.value(1).toString();
                    
                    if (grade.compare("Junior", Qt::CaseInsensitive) == 0) {
                        qDebug() << "[SERIAL TX/RX] Access Denied! Reason: Junior Grade.";
                        serial->write("DENIED_JUNIOR\n");
                    } else {
                        if (!activeShifts.contains(uid)) {
                            activeShifts[uid] = QDateTime::currentDateTime();
                            qDebug() << "[RFID CLOCK] Clock IN for" << name << "at" << activeShifts[uid];
                        } else {
                            QDateTime inTime = activeShifts.take(uid);
                            QDateTime outTime = QDateTime::currentDateTime();
                            double secondsWorked = inTime.secsTo(outTime);
                            double hoursWorked = secondsWorked / 3600.0;

                            QSqlQuery updateQuery;
                            updateQuery.prepare("UPDATE PERSONNEL SET HOURS = NVL(HOURS, 0) + :workedHours WHERE RFID = :uid");
                            updateQuery.bindValue(":workedHours", hoursWorked);
                            updateQuery.bindValue(":uid", uid);
                            updateQuery.exec();
                        }

                        qDebug() << "[SERIAL TX/RX] Match found! Name:" << name;
                        serial->write(name.toUtf8() + "\n");
                    }
                } else {
                    qDebug() << "[SERIAL TX/RX] Match NOT found! Sending 'NOT_FOUND'.";
                    serial->write("NOT_FOUND\n");
                }
            }
        }

    }
}

