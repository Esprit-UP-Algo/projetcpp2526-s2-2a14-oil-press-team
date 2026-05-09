#ifndef GENERICBARCHART_H
#define GENERICBARCHART_H

#include <QWidget>
#include <QPainter>
#include <QList>
#include <QString>

class GenericBarChart : public QWidget {
    Q_OBJECT
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
    painter.fillRect(rect(), Qt::white);

    if (m_bars.isEmpty())
      return;

    int leftMargin = 80;   
    int bottomMargin = 50; 
    int topMargin = 60;    
    int rightMargin = 20;

    int chartWidth = width() - leftMargin - rightMargin;
    int chartHeight = height() - topMargin - bottomMargin;

    double maxVal = 0;
    for (const auto &bar : m_bars)
      maxVal = qMax(maxVal, bar.value);
    if (maxVal == 0)
      maxVal = 1;
    double niceMax = maxVal * 1.1;

    int gridLines = qMax(1, (int)niceMax);
    if (gridLines > 8) gridLines = 5;
    painter.setFont(QFont("Segoe UI", 9));
    QPen gridPen(QColor("#e0e0e0"));
    gridPen.setStyle(Qt::DashLine);

    for (int i = 0; i <= gridLines; ++i) {
      int intValue = (int)qRound((niceMax / gridLines) * i);
      int y = topMargin + chartHeight - (int)((intValue / niceMax) * chartHeight);

      painter.setPen(gridPen);
      painter.drawLine(leftMargin, y, width() - rightMargin, y);

      painter.setPen(QColor("#666666"));
      QString label = QString::number(intValue);
      painter.drawText(QRect(0, y - 10, leftMargin - 10, 20),
                       Qt::AlignRight | Qt::AlignVCenter, label);
    }

    painter.setPen(QPen(QColor("#333333"), 2));
    painter.drawLine(leftMargin, topMargin + chartHeight, width() - rightMargin,
                     topMargin + chartHeight);

    int count = m_bars.size();
    int availableSpace = chartWidth / count;
    int barWidth = qMin(availableSpace - 20, 80);

    for (int i = 0; i < count; ++i) {
      const auto &bar = m_bars[i];
      int barH = (int)((bar.value / niceMax) * chartHeight);

      int x = leftMargin + (i * availableSpace) + (availableSpace - barWidth) / 2;
      int y = topMargin + chartHeight - barH;

      QRect barRect(x, y, barWidth, barH);
      painter.setBrush(bar.color);
      painter.setPen(Qt::NoPen);
      painter.drawRect(barRect);

      painter.setPen(QColor("#000000"));
      painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
      QString valText = QString::number(bar.value, 'f', 0);
      painter.drawText(QRect(x - 10, y - 25, barWidth + 20, 20),
                       Qt::AlignCenter, valText);

      painter.setPen(QColor("#333333"));
      painter.setFont(QFont("Segoe UI", 10)); 
      QRect labelRect(x - 20, topMargin + chartHeight + 10, barWidth + 40, 40);
      painter.drawText(labelRect, Qt::AlignCenter | Qt::TextWordWrap, bar.label);
    }

    painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
    painter.setPen(QColor("#1a1a1a"));
    painter.drawText(QRect(0, 10, width(), 30), Qt::AlignCenter, m_title);
  }

private:
  QString m_title;
  QList<BarData> m_bars;
};

#endif // GENERICBARCHART_H
