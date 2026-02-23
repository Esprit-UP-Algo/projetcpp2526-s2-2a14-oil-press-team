#include "mainwindow.h"
#include "AuthWidgets.h"
#include "EyeSaverButton.h"
#include "article.h"
#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
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

static QWidget *createStyledForm(const QString &title,
                                 const QList<QPair<QString, QString>> &fields,
                                 const QString &submitText, int spacing = 15,
                                 bool useScroll = true) {
  // 1. Create content widget
  QWidget *formContent = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(formContent);
  int rightMargin = useScroll ? 10 : 0;
  layout->setContentsMargins(0, 0, rightMargin, 0);
  layout->setSpacing(spacing);

  if (!title.isEmpty()) {
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: "
                              "#1a1a1a; margin-bottom: 25px; border: none;");
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
  formContainer->setStyleSheet(
      ".QWidget { background-color: #ffffff; border-radius: 10px; border: 1px "
      "solid #eee; }");

  QVBoxLayout *outerLayout = new QVBoxLayout(formContainer);
  outerLayout->setContentsMargins(30, 30, 30, 30);

  if (useScroll) {
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QWidget { background: transparent; }"
        "QScrollBar:vertical { border: none; background: #f0f0f0; width: 10px; "
        "margin: 0px 0px 0px 0px; border-radius: 5px; }"
        "QScrollBar::handle:vertical { background: #cdcdcd; min-height: 20px; "
        "border-radius: 5px; }"
        "QScrollBar::handle:vertical:hover { background: #3DDC84; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { "
        "background: none; }");
    scrollArea->setWidget(formContent);
    outerLayout->addWidget(scrollArea);
  } else {
    formContent->setAttribute(Qt::WA_TranslucentBackground);
    formContent->setStyleSheet("background: transparent;");
    outerLayout->addWidget(formContent);
  }

  return formContainer;
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

  for (const auto &name : tabNames) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(getTabButtonStyle());
    actionLayout->addWidget(btn);
    tabButtons.append(btn);

    QWidget *content = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout(content);

    if (name == "New Order") {
      cLayout->addWidget(
          createStyledForm("New Order Registration",
                           {{"Company Name:", "Enter company name"},
                            {"Contact Person:", "Enter contact person name"},
                            {"Email Address:", "email@example.com"},
                            {"Phone Number:", "+1 (555) ..."},
                            {"Address:", "Street, City, Zip"}},
                           "Register Client"));
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
      cLayout->addWidget(createStyledForm(
          "Edit Details",
          {{"Company Name:", "Acme Corp"}, {"Email:", "contact@acme.com"}},
          "Update Profile"));
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

      controlLayout->addWidget(searchEdit);
      controlLayout->addSpacing(15);
      controlLayout->addWidget(lblSort);
      controlLayout->addWidget(sortCombo);
      controlLayout->addSpacing(15);
      controlLayout->addWidget(btnPrint);
      controlLayout->addStretch();

      histLayout->addWidget(controlBar);

      // 2. Table
      QStringList headers = {"Date",   "Client", "Type",
                             "Amount", "Notes",  "Actions"};
      QTableWidget *table = new QTableWidget();
      table->setColumnCount(headers.size());
      table->setHorizontalHeaderLabels(headers);
      table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      table->horizontalHeader()->setSectionResizeMode(
          headers.size() - 1, QHeaderView::ResizeToContents);
      table->verticalHeader()->setVisible(false);
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
      struct OrderItem {
        QString date;
        QString client;
        QString action;
        double amount;
        QString notes;
      };

      // Mock Data
      QList<OrderItem> allOrders = {
          {"2023-10-25", "Trattoria Luigi", "Meeting", 0.0,
           "Discussed Winter Supply"},
          {"2023-10-24", "Oileria Bella", "Support", 0.0,
           "Clarified acidity levels"},
          {"2023-10-22", "Organic Market", "Order", 4500.50,
           "Placed bulk order #992"},
          {"2023-10-20", "Gourmet Foods", "Call", 0.0, "Quality check inquiry"},
          {"2023-10-18", "Pasta Place", "Order", 1200.00,
           "Regular monthly supply"},
          {"2023-10-15", "Pizza House", "Order", 320.75, "Emergency restock"}};

      auto updateTable = [table, allOrders, searchEdit, sortCombo]() {
        QString query = searchEdit->text().toLower();
        QString sortMode = sortCombo->currentText();

        // 1. Filter
        QList<OrderItem> filtered;
        for (const auto &item : allOrders) {
          bool match = item.client.toLower().contains(query) ||
                       item.notes.toLower().contains(query) ||
                       item.action.toLower().contains(query);
          if (match)
            filtered.append(item);
        }

        // 2. Sort
        std::sort(filtered.begin(), filtered.end(),
                  [sortMode](const OrderItem &a, const OrderItem &b) {
                    if (sortMode == "Date (Newest)")
                      return a.date > b.date;
                    if (sortMode == "Date (Oldest)")
                      return a.date < b.date;
                    if (sortMode == "Amount (High-Low)")
                      return a.amount > b.amount;
                    if (sortMode == "Amount (Low-High)")
                      return a.amount < b.amount;
                    return false;
                  });

        // 3. Populate
        table->setRowCount(0);
        table->setRowCount(filtered.size());
        for (int i = 0; i < filtered.size(); ++i) {
          const auto &item = filtered[i];
          table->setItem(i, 0, new QTableWidgetItem(item.date));
          table->setItem(i, 1, new QTableWidgetItem(item.client));
          table->setItem(i, 2, new QTableWidgetItem(item.action));

          QString amtStr = (item.amount > 0)
                               ? QString::number(item.amount, 'f', 2) + " €"
                               : "-";
          table->setItem(i, 3, new QTableWidgetItem(amtStr));

          table->setItem(i, 4, new QTableWidgetItem(item.notes));

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
              "600; font-size: 13px; color: #333333; } QPushButton:hover { "
              "border-color: #aaaaaa; color: #000000; background-color: "
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
          table->setCellWidget(i, 5, actionWidget);

          // Connect Modify
          QObject::connect(
              btnModify, &QPushButton::clicked,
              [table, i, data = filtered[i]]() {
                QDialog dlg(table->window());
                dlg.setWindowTitle("Edit Order Details");
                dlg.setModal(true);
                dlg.setMinimumWidth(450);
                dlg.setStyleSheet("QDialog { background-color: #ffffff; "
                                  "border-radius: 12px; }");

                QVBoxLayout *mainV = new QVBoxLayout(&dlg);
                mainV->setContentsMargins(30, 30, 30, 30);
                mainV->setSpacing(20);

                QLabel *title = new QLabel("Update Order Information");
                title->setStyleSheet(
                    "font-size: 20px; font-weight: 700; color: #1a1a1a;");
                mainV->addWidget(title);

                QFormLayout *form = new QFormLayout();
                form->setSpacing(15);
                form->setLabelAlignment(Qt::AlignLeft);

                QLineEdit *dateEdit = new QLineEdit(table->item(i, 0)->text());
                QLineEdit *clientEdit =
                    new QLineEdit(table->item(i, 1)->text());
                QLineEdit *typeEdit = new QLineEdit(table->item(i, 2)->text());
                QLineEdit *amountEdit =
                    new QLineEdit(table->item(i, 3)->text());
                QLineEdit *notesEdit = new QLineEdit(table->item(i, 4)->text());

                auto styleField = [&](QLineEdit *le) {
                  le->setStyleSheet(
                      "QLineEdit { background-color: #f9fafb; border: 1px "
                      "solid #eaeaea; border-radius: 8px; padding: 10px; "
                      "font-size: 14px; color: #333; } QLineEdit:focus { "
                      "border-color: #3DDC84; background-color: #ffffff; }");
                  le->setFixedHeight(40);
                };

                styleField(dateEdit);
                styleField(clientEdit);
                styleField(typeEdit);
                styleField(amountEdit);
                styleField(notesEdit);

                auto addRow = [&](const QString &label, QWidget *w) {
                  QLabel *l = new QLabel(label);
                  l->setStyleSheet(
                      "font-weight: 600; color: #444; font-size: 13px;");
                  form->addRow(l, w);
                };

                addRow("Order Date:", dateEdit);
                addRow("Client Name:", clientEdit);
                addRow("Interaction Type:", typeEdit);
                addRow("Total Amount:", amountEdit);
                addRow("Internal Notes:", notesEdit);

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
                  table->item(i, 0)->setText(dateEdit->text());
                  table->item(i, 1)->setText(clientEdit->text());
                  table->item(i, 2)->setText(typeEdit->text());
                  table->item(i, 3)->setText(amountEdit->text());
                  table->item(i, 4)->setText(notesEdit->text());
                  QMessageBox::information(
                      table->window(), "Success",
                      "Order details updated successfully.");
                }
              });

          // Connect Delete
          QObject::connect(
              btnDelete, &QPushButton::clicked, [table, btnDelete]() {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(
                    table->window(), "Delete Order",
                    "Are you sure you want to delete this order?",
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                  // Find row dynamically because removal changes indices
                  QPoint btnPos = btnDelete->mapTo(table->viewport(),
                                                   btnDelete->rect().center());
                  int row = table->rowAt(btnPos.y());
                  if (row >= 0) {
                    table->removeRow(row);
                    QMessageBox::information(table->window(), "Deleted",
                                             "Order deleted successfully.");
                  }
                }
              });
        }
      };

      // Connect
      QObject::connect(searchEdit, &QLineEdit::textChanged, updateTable);
      QObject::connect(sortCombo, &QComboBox::currentTextChanged, updateTable);

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

  for (const auto &name : tabNames) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(getTabButtonStyle());
    actionLayout->addWidget(btn);
    tabButtons.append(btn);

    QWidget *content = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout(content);

    if (name == "New Invoice") {
      // Add Print PDF Button
      QPushButton *btnPrint = new QPushButton("PRINT PDF");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedWidth(150);
      cLayout->addWidget(btnPrint, 0, Qt::AlignRight);

      cLayout->addWidget(
          createStyledForm("Invoice Creation Form",
                           {{"Customer Name:", "Enter customer name"},
                            {"Amount:", "0.00"},
                            {"Date:", "YYYY-MM-DD"}},
                           "Submit Invoice"));
    } else if (name == "Transaction Hub") {
      // --- Custom Transactions View ---
      QWidget *transContainer = new QWidget();
      QVBoxLayout *transLayout = new QVBoxLayout(transContainer);
      transLayout->setContentsMargins(0, 0, 0, 0);
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
      searchType->addItems({"Status"});
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
      QStringList headers = {"Date", "Description", "Amount", "Status",
                             "Actions"};
      QTableWidget *table = new QTableWidget();
      table->setColumnCount(headers.size());
      table->setHorizontalHeaderLabels(headers);
      table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      table->verticalHeader()->setVisible(false);
      table->setAlternatingRowColors(true);
      table->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: "
          "#ffffff; gridline-color: transparent; border-radius: 8px; "
          "alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; "
          "border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; "
          "color: #666; }");

      // Data
      struct Transaction {
        QString id;
        QString date;
        QString desc;
        double amount;
        QString transactionStatus;
      };
      QList<Transaction> transactions = {
          {"TRX-1024", "2023-10-25", "Packaging Supplies", 150.00, "Completed"},
          {"TRX-1023", "2023-10-24", "Client Payment - Luigi", 1200.00,
           "Completed"},
          {"TRX-1022", "2023-10-24", "Organic Certification", 340.50,
           "Pending"},
          {"TRX-1021", "2023-10-23", "Maintenance Parts", 850.00, "Completed"},
          {"TRX-1020", "2023-10-22", "Utility Bill", 210.75, "Pending"}};

      auto populateTable = [table](const QList<Transaction> &data) {
        table->setRowCount(0);
        table->setRowCount(data.size());
        for (int i = 0; i < data.size(); ++i) {
          const auto &t = data[i];
          table->setItem(i, 0, new QTableWidgetItem(t.date));
          table->setItem(i, 1, new QTableWidgetItem(t.desc));
          // Format Amount
          QString amtStr = "$" + QString::number(t.amount, 'f', 2);
          if (t.amount >= 1000) {
            int pos = amtStr.length() - 6;
            while (pos > 1) {
              amtStr.insert(pos, ",");
              pos -= 3;
            }
          }
          table->setItem(i, 2, new QTableWidgetItem(amtStr));
          table->setItem(i, 3, new QTableWidgetItem(t.transactionStatus));

          // Action Column
          QWidget *actionWidget = new QWidget();
          QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
          actionLayout->setContentsMargins(5, 2, 5, 2);
          actionLayout->setSpacing(5);

          QPushButton *btnEdit = new QPushButton("Edit");
          btnEdit->setCursor(Qt::PointingHandCursor);
          btnEdit->setMinimumWidth(80);
          btnEdit->setFixedHeight(28);
          btnEdit->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#cccccc; border-radius: 6px; padding: 0px 8px; font-weight: "
              "600; font-size: 13px; color: #333333; } QPushButton:hover { "
              "border-color: #aaaaaa; color: #000000; background-color: "
              "#f6f6f6; }");

          QPushButton *btnRem = new QPushButton("Remove");
          btnRem->setCursor(Qt::PointingHandCursor);
          btnRem->setMinimumWidth(80);
          btnRem->setFixedHeight(28);
          btnRem->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#d32f2f; color: #d32f2f; border-radius: 6px; padding: 0px 8px; "
              "font-weight: 600; font-size: 13px; } QPushButton:hover { "
              "background-color: #ffebee; border-color: #b71c1c; color: "
              "#b71c1c; }");

          actionLayout->addWidget(btnEdit);
          actionLayout->addWidget(btnRem);
          table->setCellWidget(i, 4, actionWidget);

          QObject::connect(btnRem, &QPushButton::clicked, [table, btnRem]() {
            QPoint btnPos =
                btnRem->mapTo(table->viewport(), btnRem->rect().center());
            int row = table->rowAt(btnPos.y());
            if (row >= 0) {
              if (QMessageBox::question(table->window(), "Confirm",
                                        "Remove this transaction record?") ==
                  QMessageBox::Yes)
                table->removeRow(row);
            }
          });
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
        for (const auto &t : transactions) {
          bool match = t.transactionStatus.toLower().contains(query);
          if (match)
            filtered.append(t);
        }

        // Sort
        std::sort(filtered.begin(), filtered.end(),
                  [sort](const Transaction &a, const Transaction &b) {
                    if (sort == "Amt High-Low")
                      return a.amount > b.amount;
                    else
                      return a.amount < b.amount;
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
        QMessageBox::information(table, "Print",
                                 "Generating Financial Report PDF...");
      });

      cLayout->addWidget(transContainer);
    } else if (name == "Expense Tracking") {
      // Add Print PDF Button
      QPushButton *btnPrint = new QPushButton("PRINT PDF");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedWidth(150);
      cLayout->addWidget(btnPrint, 0, Qt::AlignRight);

      cLayout->addWidget(createStyledTable(
          "Expense Log", {"Date", "Category", "Description", "Amount"},
          {{"2023-10-25", "Travel", "Flight to NY Conference", "$450.00"},
           {"2023-10-24", "Meals", "Team Lunch", "$125.00"}},
          true));
    } else if (name == "Analytics") {
      // Financial Analytics
      QPushButton *btnPrint = new QPushButton("PRINT REPORT");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedWidth(150);
      cLayout->addWidget(btnPrint, 0, Qt::AlignRight);

      GenericBarChart *chart = new GenericBarChart("Revenue vs Expenses");
      chart->addBar("Revenue", 1250500, QColor(61, 220, 132));
      chart->addBar("Expenses", 850000, QColor(231, 76, 60));

      QWidget *chartContainer = new QWidget();
      chartContainer->setStyleSheet(getCardStyle());
      QVBoxLayout *containerLayout = new QVBoxLayout(chartContainer);
      containerLayout->setContentsMargins(30, 30, 30, 30);
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
          btnSubmit, &QPushButton::clicked, [inputNom, inputQty, inputSeuil]() {
            // Input validation
            if (inputNom->text().trimmed().isEmpty()) {
              QMessageBox::warning(nullptr, "Validation Error",
                                   "Item Name is required.");
              return;
            }

            Article a;
            a.setNom(inputNom->text().trimmed());
            a.setQuantite(inputQty->text().toInt());
            a.setSeuilMinimal(inputSeuil->text().toInt());

            if (a.ajouter()) {
              QMessageBox::information(nullptr, "Success",
                                       "Item added successfully!");
              // Clear the form
              inputNom->clear();
              inputQty->clear();
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
      QStringList headers = {"ID", "Item Name", "Current Qty", "Min Threshold",
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
          QString seuil = model->data(model->index(i, 3)).toString();

          stockTable->setItem(i, 0, new QTableWidgetItem(id));
          stockTable->setItem(i, 1, new QTableWidgetItem(nom));
          stockTable->setItem(i, 2, new QTableWidgetItem(qty));
          stockTable->setItem(i, 3, new QTableWidgetItem(seuil));

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
              [stockTable, itemId, nom, qty, seuil]() {
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
                    [dialog, editNom, editQty, editSeuil, itemId,
                     stockTable]() {
                      Article a;
                      a.setId(itemId);
                      a.setNom(editNom->text().trimmed());
                      a.setQuantite(editQty->text().toInt());
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
                            stockTable->item(r, 3)->setText(editSeuil->text());
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

          stockTable->setCellWidget(i, 4, actionWidget);
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
  // Updated tab names for Machine Management (English)
  QStringList tabNames = {"Add Asset", "Asset Hub", "Service History",
                          "Analytics"};
  QList<QPushButton *> tabButtons;

  for (const auto &name : tabNames) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(getTabButtonStyle());
    actionLayout->addWidget(btn);
    tabButtons.append(btn);

    QWidget *content = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout(content);

    if (name == "Add Asset") {
      // Manual form creation for Add Machine
      QWidget *formContainer = new QWidget();
      QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
      formLayout->setSpacing(15);
      formLayout->setContentsMargins(0, 0, 10, 0);

      // Title
      QLabel *titleLabel = new QLabel("New Machine");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: "
                                "#1a1a1a; margin-bottom: 25px; border: none;");
      formLayout->addWidget(titleLabel);

      QString labelStyle = getLabelStyle();
      QString inputStyle = getInputStyle();

      // Define fields (ID is auto-generated, so removed)
      struct FieldInput {
        QString label;
        QString placeholder;
        QLineEdit *widget;
      };
      // Use shared_ptr or similar if we were using sophisticated management,
      // but here standard pointers in lambda with capture by value of the list
      // of raw pointers is fine for this scope However, we need to capture
      // specific widgets.
      QLineEdit *nameInput = new QLineEdit();
      QLineEdit *typeInput = new QLineEdit();
      QLineEdit *statusInput = new QLineEdit();

      QList<QPair<QString, QLineEdit *>> inputs = {
          {"Machine Name:", nameInput},
          {"Machine Type:", typeInput},
          {"Machine Status:", statusInput}};

      for (auto &field : inputs) {
        QLabel *lbl = new QLabel(field.first);
        lbl->setStyleSheet(labelStyle);
        field.second->setStyleSheet(inputStyle);
        // field.second->setPlaceholderText(field.placeholder); // Simplified

        formLayout->addWidget(lbl);
        formLayout->addWidget(field.second);
      }

      formLayout->addSpacing(20);
      QPushButton *btnAdd = new QPushButton("Add Machine");
      btnAdd->setStyleSheet(getButtonStyle());
      btnAdd->setCursor(Qt::PointingHandCursor);
      btnAdd->setFixedHeight(45);
      formLayout->addWidget(btnAdd);
      formLayout->addStretch();

      // Wrap in scroll area
      cLayout->addWidget(formContainer);

      // Logic to Add Machine
      // We need to capture outNestedStack to find the table later
      QObject::connect(
          btnAdd, &QPushButton::clicked,
          [nameInput, typeInput, statusInput, outNestedStack, tabButtons]() {
            // Find the table in the "Machine List" page (which is index 1 of
            // outNestedStack)
            QTableWidget *table =
                outNestedStack->findChild<QTableWidget *>("MaintenanceTable");
            if (!table) {
              QMessageBox::warning(nullptr, "Error",
                                   "Machine List table not found!");
              return;
            }

            if (nameInput->text().isEmpty() || typeInput->text().isEmpty()) {
              QMessageBox::warning(nullptr, "Error",
                                   "Please fill in Name and Type.");
              return;
            }

            // Generate ID
            int nextIdNum = table->rowCount() + 1;
            QString newId = QString("MAC-%1").arg(nextIdNum, 3, 10, QChar('0'));

            // Add Row
            int row = table->rowCount();
            table->insertRow(row);

            table->setItem(row, 0, new QTableWidgetItem(newId));
            table->setItem(row, 1, new QTableWidgetItem(nameInput->text()));
            table->setItem(row, 2, new QTableWidgetItem(typeInput->text()));
            table->setItem(row, 3,
                           new QTableWidgetItem(statusInput->text().isEmpty()
                                                    ? "Normal"
                                                    : statusInput->text()));

            // Hours default 0
            QTableWidgetItem *hItem = new QTableWidgetItem();
            hItem->setData(Qt::DisplayRole, 0);
            table->setItem(row, 4, hItem);

            // Add Actions (Copy of Machine List logic)
            QWidget *actionWidget = new QWidget();
            QHBoxLayout *al = new QHBoxLayout(actionWidget);
            al->setContentsMargins(2, 2, 2, 2);
            QPushButton *btnMod = new QPushButton("Edit");
            btnMod->setCursor(Qt::PointingHandCursor);
            btnMod->setStyleSheet(
                "QPushButton { background-color: #ffffff; border: 1px solid "
                "#cccccc; border-radius: 6px; padding: 4px 10px; font-weight: "
                "600; font-size: 13px; color: #333333; } QPushButton:hover { "
                "background-color: #f6f6f6; border-color: #bbbbbb; }");

            QPushButton *btnDel = new QPushButton("Delete");
            btnDel->setCursor(Qt::PointingHandCursor);
            btnDel->setStyleSheet(
                "QPushButton { background-color: #ffffff; border: 1px solid "
                "#d32f2f; color: #d32f2f; border-radius: 6px; padding: 4px "
                "10px; font-weight: 600; font-size: 13px; } QPushButton:hover "
                "{ background-color: #ffebee; border-color: #b71c1c; color: "
                "#b71c1c; }");

            // Connect Delete Button
            QObject::connect(btnDel, &QPushButton::clicked, [table, btnDel]() {
              QPoint btnPos =
                  btnDel->mapTo(table->viewport(), btnDel->rect().center());
              int r = table->rowAt(btnPos.y());
              if (r >= 0) {
                int ret = QMessageBox::warning(
                    table, "Confirm Delete",
                    "Are you sure you want to delete this machine?",
                    QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes)
                  table->removeRow(r);
              }
            });

            al->addWidget(btnMod);
            al->addWidget(btnDel);
            table->setCellWidget(row, 5, actionWidget);

            // Clear inputs
            nameInput->clear();
            typeInput->clear();
            statusInput->clear();

            // Switch to list
            if (outNestedStack)
              outNestedStack->setCurrentIndex(1); // Index 1 is Machine List
            if (tabButtons.size() > 1)
              tabButtons[1]->setChecked(true);
          });
    } else if (name == "Asset Hub") {
      // --- Enhanced Machine List ---
      QWidget *listContainer = new QWidget();
      QVBoxLayout *listLayout = new QVBoxLayout(listContainer);
      listLayout->setContentsMargins(0, 0, 0, 0);
      listLayout->setSpacing(10);

      // 1. Control Bar
      QWidget *controlBar = new QWidget();
      QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
      controlLayout->setContentsMargins(0, 0, 0, 0);

      // Removed Sort Button as requested

      QPushButton *btnPrint = new QPushButton("PRINT PDF");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedWidth(120);

      // Added Search Bar
      // QLineEdit *searchEdit = new QLineEdit();
      // searchEdit->setPlaceholderText("Search...");
      // searchEdit->setStyleSheet(getInputStyle());
      // searchEdit->setFixedWidth(150);

      QComboBox *searchType = new QComboBox();
      searchType->addItems({"Type", "Status"});
      searchType->setStyleSheet(getInputStyle());
      searchType->setFixedWidth(100);

      // Added Sort Controls (Replacing Search)
      QLabel *lblSort = new QLabel("Sort by:");
      lblSort->setStyleSheet(getLabelStyle());

      QComboBox *sortType = new QComboBox();
      sortType->addItems({"Select...", "Status", "Hours"});
      sortType->setStyleSheet(getInputStyle());
      sortType->setFixedWidth(120);

      // controlLayout->addWidget(searchEdit); // Removed
      controlLayout->addWidget(searchType);
      controlLayout->addSpacing(20);
      controlLayout->addWidget(lblSort);
      controlLayout->addWidget(sortType);
      controlLayout->addStretch();
      controlLayout->addWidget(btnPrint);

      listLayout->addWidget(controlBar);

      // 2. Table
      QStringList headers = {"ID",     "Name",  "Type",
                             "Status", "Hours", "Actions"};
      QTableWidget *table = new QTableWidget();
      table->setObjectName(
          "MaintenanceTable"); // Added ObjectName for finding it later
      table->setColumnCount(headers.size());
      table->setHorizontalHeaderLabels(headers);
      table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      table->horizontalHeader()->setSectionResizeMode(
          headers.size() - 1, QHeaderView::ResizeToContents);
      table->setColumnHidden(0, true); // Hide ID column
      table->verticalHeader()->setVisible(false);
      table->setAlternatingRowColors(true);
      table->setStyleSheet(
          "QTableWidget { border: 1px solid #eaeaea; background-color: "
          "#ffffff; gridline-color: transparent; border-radius: 8px; "
          "alternate-background-color: #f9fafb; }"
          "QHeaderView::section { background-color: #ffffff; padding: 12px; "
          "border: none; border-bottom: 2px solid #f0f0f0; font-weight: 700; "
          "color: #666; }");

      // Data
      struct Machine {
        QString id;
        QString name;
        QString type;
        QString machineStatus;
        int hours;
      };
      QList<Machine> machines = {
          {"MAC-001", "Press Alpha", "Press", "Normal", 1250},
          {"MAC-002", "Centrifuge Beta", "Centrifuge", "Broken", 800},
          {"MAC-003", "Filter Gamma", "Filter", "Maintenance", 3200},
          {"MAC-004", "Press Delta", "Press", "Normal", 450},
          {"MAC-005", "Bottler Epsilon", "Bottling", "Normal", 2100}};

      auto populateTable = [table, headers](const QList<Machine> &data) {
        table->setRowCount(0);
        table->setRowCount(data.size());
        for (int i = 0; i < data.size(); ++i) {
          const auto &m = data[i];
          table->setItem(i, 0, new QTableWidgetItem(m.id));
          table->setItem(i, 1, new QTableWidgetItem(m.name));
          table->setItem(i, 2, new QTableWidgetItem(m.type));
          table->setItem(i, 3, new QTableWidgetItem(m.machineStatus));
          // Hours (sortable number)
          QTableWidgetItem *hItem = new QTableWidgetItem();
          hItem->setData(Qt::DisplayRole, m.hours);
          table->setItem(i, 4, hItem);

          // Actions
          QWidget *actionWidget = new QWidget();
          QHBoxLayout *al = new QHBoxLayout(actionWidget);
          al->setContentsMargins(5, 2, 5, 2);
          al->setSpacing(8);

          QPushButton *btnMod = new QPushButton("Edit");
          btnMod->setCursor(Qt::PointingHandCursor);
          btnMod->setMinimumWidth(80);
          btnMod->setFixedHeight(28);
          btnMod->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#cccccc; border-radius: 6px; padding: 0px 8px; font-weight: "
              "600; font-size: 13px; color: #333333; } QPushButton:hover { "
              "border-color: #aaaaaa; color: #000000; background-color: "
              "#f6f6f6; }");

          QPushButton *btnDel = new QPushButton("Remove");
          btnDel->setCursor(Qt::PointingHandCursor);
          btnDel->setMinimumWidth(80);
          btnDel->setFixedHeight(28);
          btnDel->setStyleSheet(
              "QPushButton { background-color: #ffffff; border: 1px solid "
              "#d32f2f; color: #d32f2f; border-radius: 6px; padding: 0px 8px; "
              "font-weight: 600; font-size: 13px; } QPushButton:hover { "
              "background-color: #ffebee; border-color: #b71c1c; color: "
              "#b71c1c; }");

          al->addWidget(btnMod);
          al->addWidget(btnDel);
          table->setCellWidget(i, 5, actionWidget);

          // Connect Modify Button
          QObject::connect(btnMod, &QPushButton::clicked, [table, btnMod]() {
            QPoint btnPos =
                btnMod->mapTo(table->viewport(), btnMod->rect().center());
            int row = table->rowAt(btnPos.y());
            if (row >= 0) {
              // Get current data
              QString id = table->item(row, 0)->text();
              QString name = table->item(row, 1)->text();
              QString type = table->item(row, 2)->text();
              QString currentMachineStatus = table->item(row, 3)->text();

              // Create Dialog
              QDialog dlg(table);
              dlg.setWindowTitle("Modify Machine");
              dlg.setModal(true);
              dlg.setMinimumWidth(400);
              dlg.setStyleSheet("QDialog { background-color: #ffffff; "
                                "border-radius: 12px; }");

              QVBoxLayout *mainV = new QVBoxLayout(&dlg);
              mainV->setContentsMargins(30, 30, 30, 30);
              mainV->setSpacing(20);

              QLabel *title = new QLabel("Edit Machine Details");
              title->setStyleSheet(
                  "font-size: 20px; font-weight: 700; color: #1a1a1a;");
              mainV->addWidget(title);

              QFormLayout *form = new QFormLayout();
              form->setSpacing(15);
              form->setLabelAlignment(Qt::AlignLeft);

              QLineEdit *edName = new QLineEdit(name);
              edName->setStyleSheet(getInputStyle());
              edName->setFixedHeight(40);

              QLineEdit *edType = new QLineEdit(type);
              edType->setStyleSheet(getInputStyle());
              edType->setFixedHeight(40);

              QLineEdit *edStatus = new QLineEdit(currentMachineStatus);
              edStatus->setStyleSheet(getInputStyle());
              edStatus->setFixedHeight(40);

              QLabel *lblId = new QLabel(id);
              lblId->setStyleSheet("font-weight: 600; color: #666;");

              auto addStyledRow = [&](const QString &label, QWidget *w) {
                QLabel *l = new QLabel(label);
                l->setStyleSheet(getLabelStyle());
                form->addRow(l, w);
              };

              addStyledRow("Machine ID:", lblId);
              addStyledRow("Name:", edName);
              addStyledRow("Type:", edType);
              addStyledRow("Status:", edStatus);

              mainV->addLayout(form);
              mainV->addSpacing(10);

              QDialogButtonBox *bbox = new QDialogButtonBox(
                  QDialogButtonBox::Save | QDialogButtonBox::Cancel);
              bbox->setStyleSheet(
                  "QPushButton { padding: 8px 20px; border-radius: 6px; "
                  "font-weight: 600; }"
                  "QPushButton[text='Save'] { background-color: #3DDC84; "
                  "color: white; border: none; }"
                  "QPushButton[text='Save']:hover { background-color: #34c772; "
                  "}"
                  "QPushButton[text='Cancel'] { background-color: #f5f5f5; "
                  "color: #666; border: 1px solid #ddd; }"
                  "QPushButton[text='Cancel']:hover { background-color: #eee; "
                  "}");
              mainV->addWidget(bbox);

              QObject::connect(bbox, &QDialogButtonBox::accepted, &dlg,
                               &QDialog::accept);
              QObject::connect(bbox, &QDialogButtonBox::rejected, &dlg,
                               &QDialog::reject);

              if (dlg.exec() == QDialog::Accepted) {
                table->item(row, 1)->setText(edName->text());
                table->item(row, 2)->setText(edType->text());
                table->item(row, 3)->setText(edStatus->text());
                QMessageBox::information(table, "Success",
                                         "Machine updated successfully!");
              }
            }
          });

          // Connect Delete Button
          QObject::connect(btnDel, &QPushButton::clicked, [table, btnDel]() {
            // Find the row containing this button
            // We need to map the button position to the table coordinates
            QPoint btnPos =
                btnDel->mapTo(table->viewport(), btnDel->rect().center());
            int row = table->rowAt(btnPos.y());
            if (row >= 0) {
              int ret = QMessageBox::warning(
                  table, "Confirm Delete",
                  "Are you sure you want to delete this machine?",
                  QMessageBox::Yes | QMessageBox::No);
              if (ret == QMessageBox::Yes) {
                table->removeRow(row);
              }
            }
          });

          al->addWidget(btnMod);
          al->addWidget(btnDel);
          table->setCellWidget(i, 5, actionWidget);
        }
      };

      populateTable(machines);
      listLayout->addWidget(table);

      // Connect Search Logic
      /*
      auto filterTable = [=]() {
          QString query = searchEdit->text().toLower();
          QString type = searchType->currentText();
          int colIndex = (type == "Type") ? 2 : 3; // Type=2, Status=3

          for(int i = 0; i < table->rowCount(); ++i) {
              bool match = false;
              QTableWidgetItem *item = table->item(i, colIndex);
              if (item) {
                  match = item->text().toLower().contains(query);
              }
              table->setRowHidden(i, !match);
          }
      };

      QObject::connect(searchEdit, &QLineEdit::textChanged, filterTable);
      QObject::connect(searchType, &QComboBox::currentTextChanged, filterTable);
      */

      // Connect Sort Logic
      QObject::connect(
          sortType, &QComboBox::currentTextChanged,
          [table](const QString &text) {
            table->setSortingEnabled(false); // Disable default sorting to use
                                             // ours or just set sort order
            if (text == "Status") {
              table->sortItems(3, Qt::AscendingOrder); // Column 3 = Status
            } else if (text == "Hours") {
              table->sortItems(4, Qt::AscendingOrder); // Column 4 = Hours
            }
          });

      // Print
      QObject::connect(btnPrint, &QPushButton::clicked, [=]() {
        QMessageBox::information(
            table, "Print", "Geneating PDF report for Maintenance List...");
      });

      cLayout->addWidget(listContainer);
    } else if (name == "Service History") {
      // Keep historical logs
      cLayout->addWidget(createStyledTable(
          "Intervention History", {"Date", "Machine", "Action", "Result"},
          {{"2023-09-01", "MAC-001", "Belt Replacement", "Success"}}, true));
    } else if (name == "Analytics") { // Explicitly define Analytics tab
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
  QStringList tabNames = {"Product Hub", "Add Product"};
  QList<QPushButton *> tabButtons;

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

    if (name == "Product Hub") {
      // Container for list page to hold button + table
      QWidget *listPageWidget = new QWidget();
      QVBoxLayout *listPageLayout = new QVBoxLayout(listPageWidget);
      listPageLayout->setContentsMargins(0, 0, 0, 0);
      listPageLayout->setSpacing(10);

      // Add "PRINT PDF" button aligned to right
      QPushButton *btnPrint = new QPushButton("PRINT PDF");
      btnPrint->setStyleSheet(getButtonStyle());
      btnPrint->setCursor(Qt::PointingHandCursor);
      btnPrint->setFixedWidth(150);
      listPageLayout->addWidget(btnPrint, 0, Qt::AlignRight);

      // Create table
      QWidget *tableWidgetWrapper = createStyledTable(
          "Current Product Catalog",
          {"ID Container", "Date Pressage", "Capacité", "Ref Testeur",
           "Qualité", "Viscosity", "Color", "Ref Press"},
          {{"CONT-001", "2023-10-25", "500L", "TEST-A1", "Premium", "0.85",
            "Golden", "PRESS-X1"},
           {"CONT-002", "2023-10-26", "200L", "TEST-B2", "Standard", "0.90",
            "Yellow", "PRESS-X2"},
           {"CONT-003", "2023-10-27", "1000L", "TEST-A1", "Premium", "0.84",
            "Golden", "PRESS-X1"}},
          true);

      // Capture the table for updates
      productTable = tableWidgetWrapper->findChild<QTableWidget *>();

      listPageLayout->addWidget(tableWidgetWrapper);

      // Connect print button
      // PDF Export functionality removed as per request
      // The button remains visible but inactive

      cLayout->addWidget(listPageWidget);
    } else if (name == "Add Product") {
      // Manual form creation to allow access to inputs
      QWidget *formContainer = new QWidget();
      QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
      formLayout->setSpacing(15);
      formLayout->setContentsMargins(
          0, 0, 10, 0); // Slight right margin for scrollbar if needed

      // Title
      QLabel *titleLabel = new QLabel("Add New Product");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: "
                                "#1a1a1a; margin-bottom: 25px; border: none;");
      formLayout->addWidget(titleLabel);

      QString labelStyle = getLabelStyle();
      QString inputStyle = getInputStyle();

      // Define fields (ID is auto-generated, so removed from input)
      struct FieldInput {
        QString label;
        QString placeholder;
        QLineEdit *widget;
      };
      QList<FieldInput> inputs = {
          {"Date Pressage:", "YYYY-MM-DD", new QLineEdit()},
          {"Capacité:", "e.g., 500L", new QLineEdit()},
          {"Ref Testeur:", "Enter Tester Ref", new QLineEdit()},
          {"Qualité:", "Enter Quality Grade", new QLineEdit()},
          {"Viscosity:", "Enter Viscosity", new QLineEdit()},
          {"Color:", "Enter Color", new QLineEdit()},
          {"Ref Press:", "Enter Press Ref", new QLineEdit()}};

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
      QObject::connect(
          btnAdd, &QPushButton::clicked,
          [inputs, productTable, outNestedStack, tabButtons]() {
            if (!productTable)
              return;

            // 1. Auto-generate ID (Simple Logic: "CONT-" + (RowCount + 1))
            // Note: In a real app, this should check for existing IDs or use a
            // database counter.
            int nextIdNum = productTable->rowCount() + 1;
            QString newId =
                QString("CONT-%1").arg(nextIdNum, 3, 10, QChar('0'));

            // 2. Collect Data
            int row = productTable->rowCount();
            productTable->insertRow(row);

            // Column 0: ID
            productTable->setItem(row, 0, new QTableWidgetItem(newId));

            // Other Columns
            for (int i = 0; i < inputs.size(); ++i) {
              productTable->setItem(
                  row, i + 1, new QTableWidgetItem(inputs[i].widget->text()));
              inputs[i].widget->clear(); // Clear input after adding
            }

            // Add Actions (Modify/Delete) - We need to manually replicate the
            // actions column setup here or ideally extract that logic. For now,
            // let's replicate the basic button creation from createStyledTable
            // to ensure consistency.
            QWidget *actionWidget = new QWidget();
            QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
            actionLayout->setContentsMargins(2, 2, 30, 2);
            actionLayout->setSpacing(20);
            actionLayout->setAlignment(Qt::AlignCenter);

            QPushButton *btnModify = new QPushButton("Edit");
            btnModify->setCursor(Qt::PointingHandCursor);
            btnModify->setMinimumWidth(80);
            btnModify->setFixedHeight(28);
            btnModify->setStyleSheet(
                "QPushButton { background-color: #ffffff; color: #333333; "
                "border: 1px solid #cccccc; border-radius: 6px; padding: 0px "
                "8px; font-size: 13px; font-weight: 600; } QPushButton:hover { "
                "border-color: #aaaaaa; color: #000000; background-color: "
                "#f6f6f6; } QPushButton:pressed { background-color: #e6e6e6; "
                "}");

            QPushButton *btnDelete = new QPushButton("Remove");
            btnDelete->setCursor(Qt::PointingHandCursor);
            btnDelete->setMinimumWidth(80);
            btnDelete->setFixedHeight(28);
            btnDelete->setStyleSheet(
                "QPushButton { background-color: #ffffff; color: #d32f2f; "
                "border: 1px solid #d32f2f; border-radius: 6px; padding: 0px "
                "8px; font-size: 13px; font-weight: 600; } QPushButton:hover { "
                "background-color: #ffebee; border-color: #b71c1c; color: "
                "#b71c1c; } QPushButton:pressed { background-color: #ffcdd2; "
                "}");

            actionLayout->addWidget(btnModify);
            actionLayout->addWidget(btnDelete);
            actionLayout->addStretch();

            // Connect Delete (Copying logic from createStyledTable fix)
            QObject::connect(
                btnDelete, &QPushButton::clicked, [productTable, btnDelete]() {
                  QPoint btnPos = btnDelete->mapTo(productTable->viewport(),
                                                   btnDelete->rect().center());
                  int r = productTable->rowAt(btnPos.y());
                  if (r >= 0) {
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::question(
                        productTable->window(), "Confirm Deletion",
                        "Are you sure you want to delete this item?",
                        QMessageBox::Yes | QMessageBox::No);
                    if (reply == QMessageBox::Yes)
                      productTable->removeRow(r);
                  }
                });

            productTable->setCellWidget(
                row, 8, actionWidget); // Column 8 is Actions (0-7 are data)

            // 3. Switch to List View
            if (outNestedStack)
              outNestedStack->setCurrentIndex(0);
            if (!tabButtons.isEmpty())
              tabButtons.first()->setChecked(true);
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

  for (const auto &name : tabNames) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(getTabButtonStyle());
    actionLayout->addWidget(btn);
    tabButtons.append(btn);

    QWidget *content = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout(content);

    if (name == "Staff Hub") {
      // Container for list page
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

      controlLayout->addWidget(searchEdit);
      controlLayout->addStretch();
      listPageLayout->addWidget(controlBar);

      // Table
      QWidget *tableWidgetWrapper = createStyledTable(
          "Staff Directory",
          {"ID", "Name", "Position", "Department", "Start Date"},
          {{"EMP-001", "John Doe", "Manager", "Sales", "2020-01-15"},
           {"EMP-002", "Jane Smith", "Engineer", "Product", "2021-03-22"},
           {"EMP-003", "Robert Brown", "Technician", "Maintenance",
            "2019-11-05"},
           {"EMP-004", "Emily White", "Accountant", "Finance", "2022-06-01"}},
          true, true); // Actions + QR

      QTableWidget *table = tableWidgetWrapper->findChild<QTableWidget *>();
      if (table)
        table->setObjectName("PersonnelTable");

      listPageLayout->addWidget(tableWidgetWrapper);
      cLayout->addWidget(listPageWidget);

      // Search Logic
      if (table) {
        QObject::connect(
            searchEdit, &QLineEdit::textChanged, [table](const QString &text) {
              QString query = text.toLower();
              for (int i = 0; i < table->rowCount(); ++i) {
                bool match = false;
                if (table->item(i, 1))
                  match = table->item(i, 1)->text().toLower().contains(
                      query); // Name
                table->setRowHidden(i, !match);
              }
            });
      }

    } else if (name == "Add Staff") {
      // Manual Form
      QWidget *formContainer = new QWidget();
      QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
      formLayout->setSpacing(15);
      formLayout->setContentsMargins(0, 0, 10, 0);

      QLabel *titleLabel = new QLabel("New Employee Registration");
      titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: "
                                "#1a1a1a; margin-bottom: 25px; border: none;");
      formLayout->addWidget(titleLabel);

      QString labelStyle = getLabelStyle();
      QString inputStyle = getInputStyle();

      QLineEdit *nameInput = new QLineEdit();
      QLineEdit *posInput = new QLineEdit();
      QComboBox *deptInput = new QComboBox();
      deptInput->addItems(
          {"Select...", "Sales", "Product", "Maintenance", "Finance", "HR"});
      deptInput->setStyleSheet(inputStyle);
      deptInput->setFixedHeight(45);
      QLineEdit *emailInput = new QLineEdit();
      QLineEdit *dateInput = new QLineEdit();
      dateInput->setPlaceholderText("YYYY-MM-DD");

      auto addField = [&](QString label, QWidget *w) {
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet(labelStyle);
        if (QLineEdit *le = qobject_cast<QLineEdit *>(w))
          le->setStyleSheet(inputStyle);
        formLayout->addWidget(lbl);
        formLayout->addWidget(w);
      };

      addField("Full Name:", nameInput);
      addField("Position:", posInput);
      addField("Department:", deptInput);
      addField("Email:", emailInput);
      addField("Start Date:", dateInput);

      formLayout->addSpacing(20);
      QPushButton *btnHire = new QPushButton("Hire Employee");
      btnHire->setStyleSheet(getButtonStyle());
      btnHire->setCursor(Qt::PointingHandCursor);
      btnHire->setFixedHeight(45);
      formLayout->addWidget(btnHire);
      formLayout->addStretch();

      // Logic
      QObject::connect(btnHire, &QPushButton::clicked, [=]() {
        QTableWidget *table =
            outNestedStack->findChild<QTableWidget *>("PersonnelTable");
        if (!table)
          return;

        if (nameInput->text().isEmpty()) {
          QMessageBox::warning(nullptr, "Error", "Name is required!");
          return;
        }

        int row = table->rowCount();
        table->insertRow(row);
        // Auto ID
        table->setItem(row, 0,
                       new QTableWidgetItem(
                           QString("EMP-%1").arg(row + 1, 3, 10, QChar('0'))));
        table->setItem(row, 1, new QTableWidgetItem(nameInput->text()));
        table->setItem(row, 2, new QTableWidgetItem(posInput->text()));
        table->setItem(row, 3, new QTableWidgetItem(deptInput->currentText()));
        table->setItem(row, 4, new QTableWidgetItem(dateInput->text()));

        // Clear
        nameInput->clear();
        posInput->clear();
        emailInput->clear();
        dateInput->clear();
        deptInput->setCurrentIndex(0);

        // Success & Switch
        QMessageBox::information(nullptr, "Success",
                                 "Employee Hired Successfully!");
        if (outNestedStack)
          outNestedStack->setCurrentIndex(0);
        if (!tabButtons.isEmpty())
          tabButtons.first()->setChecked(true);
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
