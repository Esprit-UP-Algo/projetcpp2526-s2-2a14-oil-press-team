#include "workforceplanner.h"
#include <QHeaderView>
#include <QScrollArea>
#include <QMessageBox>
#include <QDate>

// --- API CONFIGURATION ---
const QString GEMINI_API_KEY = "AIzaSyDbOIr6UUIBAn_bJCatS7b0itdMplbHJRo";

WorkforcePlanner::WorkforcePlanner(QWidget *parent) : QWidget(parent) {
    selectedShift = -1;
    selectedDay = -1;
    
    QDate today = QDate::currentDate();
    currentWeek = today.weekNumber();
    currentYear = today.year();
    
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &WorkforcePlanner::handleAiResponse);

    setupUi();
    refreshAll();
}

WorkforcePlanner::~WorkforcePlanner() {}

int WorkforcePlanner::getMinRequired(const QString &role) {
    if (role == "Press Operator") return config.minPressOperator;
    if (role == "Maintenance") return config.minMaintenance;
    if (role == "Quality Control") return config.minQualityControl;
    if (role == "Logistics") return config.minLogistics;
    return 1; // generic default
}

void WorkforcePlanner::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(20);

    // TOP BAR
    QWidget *headerWidget = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    
    QPushButton *btnPrev = new QPushButton("< Prev Week");
    btnPrev->setStyleSheet("padding: 8px 15px; border-radius: 5px; background: #e5e7eb; font-weight: bold;");
    connect(btnPrev, &QPushButton::clicked, this, &WorkforcePlanner::prevWeek);
    
    weekLabel = new QLabel();
    weekLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #111827;");
    
    QPushButton *btnNext = new QPushButton("Next Week >");
    btnNext->setStyleSheet("padding: 8px 15px; border-radius: 5px; background: #e5e7eb; font-weight: bold;");
    connect(btnNext, &QPushButton::clicked, this, &WorkforcePlanner::nextWeek);
    
    QPushButton *btnBulkAi = new QPushButton("Auto-Schedule Entire Week \u2728");
    btnBulkAi->setStyleSheet("padding: 8px 15px; border-radius: 5px; background: transparent; border: 2px solid #8b5cf6; color: #8b5cf6; font-weight: bold; font-size: 13px;");
    btnBulkAi->setCursor(Qt::PointingHandCursor);
    connect(btnBulkAi, &QPushButton::clicked, this, &WorkforcePlanner::generateAiWeekRecommendation);
    
    headerLayout->addWidget(btnPrev);
    headerLayout->addStretch();
    headerLayout->addWidget(weekLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(btnBulkAi);
    headerLayout->addWidget(btnNext);
    mainLayout->addWidget(headerWidget);

    // METRICS ROW
    QWidget *metricsWidget = new QWidget();
    QHBoxLayout *metricsLayout = new QHBoxLayout(metricsWidget);
    metricsLayout->setSpacing(15);
    metricsLayout->setContentsMargins(0, 0, 0, 0);

    auto createMetricCard = [](const QString &title, QLabel*& valLabel) -> QWidget* {
        QWidget *card = new QWidget();
        card->setStyleSheet("QWidget { background: #ffffff; border: 1px solid #e5e7eb; border-radius: 8px; }");
        QVBoxLayout *layout = new QVBoxLayout(card);
        QLabel *lblTitle = new QLabel(title);
        lblTitle->setStyleSheet("font-size: 12px; color: #6b7280; text-transform: uppercase; border: none; font-weight: bold;");
        valLabel = new QLabel("--");
        valLabel->setStyleSheet("font-size: 24px; color: #111827; font-weight: 900; border: none;");
        layout->addWidget(lblTitle);
        layout->addWidget(valLabel);
        return card;
    };

    metricsLayout->addWidget(createMetricCard("Total Active Staff", lblActiveStaff));
    metricsLayout->addWidget(createMetricCard("Coverage Gaps", lblCoverageGaps));
    metricsLayout->addWidget(createMetricCard("Overscheduled", lblOverscheduled));
    metricsLayout->addWidget(createMetricCard("Capacity Score", lblCapacityScore));
    mainLayout->addWidget(metricsWidget);

    // TABS
    QTabWidget *tabs = new QTabWidget();
    tabs->setStyleSheet("QTabWidget::pane { border: 1px solid #e5e7eb; border-radius: 8px; background: white; } "
                        "QTabBar::tab { background: #f3f4f6; color: #4b5563; padding: 12px 24px; font-weight: bold; border: 1px solid #e5e7eb; border-bottom: none; border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 4px; } "
                        "QTabBar::tab:selected { background: white; color: #1D9E75; border-bottom: 2px solid #1D9E75; }");

    // -- Sub-tab 1: Heatmap
    QWidget *tabHeatmap = new QWidget();
    QVBoxLayout *heatmapLayout = new QVBoxLayout(tabHeatmap);
    heatmapLayout->setSpacing(15);
    heatmapLayout->setContentsMargins(20, 20, 20, 20);
    
    heatmapGrid = new QTableWidget(3, 7);
    heatmapGrid->setHorizontalHeaderLabels({"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"});
    heatmapGrid->setVerticalHeaderLabels({"Morning (06-14)", "Afternoon (14-22)", "Night (22-06)"});
    heatmapGrid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    heatmapGrid->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    heatmapGrid->setSelectionMode(QAbstractItemView::SingleSelection);
    heatmapGrid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    heatmapGrid->setFixedHeight(200);
    connect(heatmapGrid, &QTableWidget::cellClicked, this, &WorkforcePlanner::onCellClicked);
    
    heatmapLayout->addWidget(heatmapGrid);
    heatmapLayout->addWidget(createHeatmapDetailPanel());
    tabs->addTab(tabHeatmap, "Weekly Heatmap");

    // -- Sub-tab 2: Dept Coverage
    QWidget *tabDept = new QWidget();
    QVBoxLayout *tabDeptLayout = new QVBoxLayout(tabDept);
    deptCoverageLayout = new QVBoxLayout();
    tabDeptLayout->addLayout(deptCoverageLayout);
    
    QLabel *lblForecast = new QLabel("4-Week Forecast");
    lblForecast->setStyleSheet("font-size: 16px; font-weight: bold; margin-top: 20px;");
    tabDeptLayout->addWidget(lblForecast);
    
    forecastLayout = new QGridLayout();
    tabDeptLayout->addLayout(forecastLayout);
    tabDeptLayout->addStretch();
    tabs->addTab(tabDept, "Dept. Coverage");

    // -- Sub-tab 3: Assignments
    QWidget *tabAssignments = new QWidget();
    QScrollArea *scrollAssignments = new QScrollArea();
    scrollAssignments->setWidgetResizable(true);
    scrollAssignments->setWidget(tabAssignments);
    scrollAssignments->setFrameShape(QFrame::NoFrame);
    
    shiftAssignmentsLayout = new QVBoxLayout(tabAssignments);
    tabs->addTab(scrollAssignments, "Shift Assignments");

    mainLayout->addWidget(tabs);
}

QWidget* WorkforcePlanner::createHeatmapDetailPanel() {
    detailPanel = new QWidget();
    detailPanel->setStyleSheet("QWidget { background: #f9fafb; border: 1px solid #e5e7eb; border-left: 3px solid #1D9E75; border-radius: 4px; }");
    detailLayout = new QVBoxLayout(detailPanel);
    
    detailTitle = new QLabel("Select a shift to view details.");
    detailTitle->setStyleSheet("font-size: 16px; font-weight: bold; border: none;");
    detailLayout->addWidget(detailTitle);
    
    chipsContainer = new QWidget();
    chipsContainer->setStyleSheet("border: none; background: transparent;");
    QVBoxLayout *chipsLayout = new QVBoxLayout(chipsContainer);
    chipsLayout->setContentsMargins(0,0,0,0);
    detailLayout->addWidget(chipsContainer);
    
    aiRecommendationLabel = new QLabel("");
    aiRecommendationLabel->setStyleSheet("font-size: 14px; color: #374151; border: none; padding: 10px; background: transparent;");
    aiRecommendationLabel->setWordWrap(true);
    
    QScrollArea *aiScroll = new QScrollArea();
    aiScroll->setWidgetResizable(true);
    aiScroll->setWidget(aiRecommendationLabel);
    aiScroll->setStyleSheet("QScrollArea { border: 1px solid #e5e7eb; border-radius: 6px; background-color: #f9fafb; margin: 5px 0; }");
    aiScroll->setMinimumHeight(150);
    detailLayout->addWidget(aiScroll);
    
    QPushButton *btnAi = new QPushButton("Generate AI Schedule Recommendation");
    btnAi->setStyleSheet("QPushButton { background-color: #1D9E75; color: white; border-radius: 5px; padding: 10px; font-weight: bold; border: none;} QPushButton:hover { background-color: #147b5b; }");
    btnAi->setCursor(Qt::PointingHandCursor);
    connect(btnAi, &QPushButton::clicked, this, &WorkforcePlanner::generateAiRecommendation);
    
    btnApplyAi = new QPushButton("Apply AI Recommendations");
    btnApplyAi->setStyleSheet("QPushButton { background-color: #EF9F27; color: white; border-radius: 5px; padding: 10px; font-weight: bold; border: none;} QPushButton:hover { background-color: #d98e1f; }");
    btnApplyAi->setCursor(Qt::PointingHandCursor);
    btnApplyAi->setVisible(false);
    connect(btnApplyAi, &QPushButton::clicked, this, &WorkforcePlanner::applyAiRecommendations);

    QPushButton *btnManual = new QPushButton("Assign Staff Manually");
    btnManual->setStyleSheet("QPushButton { background-color: #f3f4f6; color: #374151; border: 1px solid #d1d5db; border-radius: 5px; padding: 10px; font-weight: bold;} QPushButton:hover { background-color: #e5e7eb; }");
    btnManual->setCursor(Qt::PointingHandCursor);
    connect(btnManual, &QPushButton::clicked, this, &WorkforcePlanner::onManualAssignClicked);

    detailLayout->addWidget(btnAi);
    detailLayout->addWidget(btnApplyAi);
    detailLayout->addWidget(btnManual);
    detailPanel->setVisible(false);
    return detailPanel;
}

void WorkforcePlanner::updateHeader() {
    weekLabel->setText(QString("Week %1, %2").arg(currentWeek).arg(currentYear));
}

void WorkforcePlanner::nextWeek() {
    currentWeek++;
    if(currentWeek > 52) { currentWeek = 1; currentYear++; }
    refreshAll();
}

void WorkforcePlanner::prevWeek() {
    currentWeek--;
    if(currentWeek < 1) { currentWeek = 52; currentYear--; }
    refreshAll();
}

void WorkforcePlanner::refreshAll() {
    updateHeader();
    refreshKpis();
    refreshHeatmap();
    refreshDeptCoverage();
    refreshAssignments();
    detailPanel->setVisible(false);
}

void WorkforcePlanner::refreshKpis() {
    QSqlQuery q;
    q.exec("SELECT COUNT(DISTINCT ID_PERSONNEL) FROM PERSONNEL WHERE STATUS = 'Active'");
    if(q.next()) lblActiveStaff->setText(q.value(0).toString());
    
    int optimalPerShift = config.minPressOperator + config.minMaintenance + config.minQualityControl + config.minLogistics;
    int totalOptimalWeek = optimalPerShift * 21; // 3 shifts * 7 days
    
    q.prepare("SELECT SHIFT_TYPE, DAY_OF_WEEK, COUNT(*) FROM SHIFT_SCHEDULE WHERE WEEK_NUMBER = :wk AND YEAR = :yr AND LEAVE_START IS NULL GROUP BY SHIFT_TYPE, DAY_OF_WEEK");
    q.bindValue(":wk", currentWeek);
    q.bindValue(":yr", currentYear);
    q.exec();
    
    int gaps = 0;
    int over = 0;
    int totalAssignments = 0;
    int shiftsFound = 0;
    
    while(q.next()) {
        int count = q.value(2).toInt();
        totalAssignments += count;
        shiftsFound++;
        if (count < optimalPerShift) gaps++;
        if (count > optimalPerShift + 2) over++;
    }
    gaps += (21 - shiftsFound); // shifts with 0 people are gaps

    lblCoverageGaps->setText(QString::number(gaps));
    lblCoverageGaps->setStyleSheet(gaps > 0 ? "font-size: 24px; font-weight: 900; border: none; color: #E24B4A;" : "font-size: 24px; font-weight: 900; border: none; color: #1D9E75;");
    
    lblOverscheduled->setText(QString::number(over));
    lblOverscheduled->setStyleSheet(over > 0 ? "font-size: 24px; font-weight: 900; border: none; color: #EF9F27;" : "font-size: 24px; font-weight: 900; border: none; color: #1D9E75;");
    
    int capacityPct = totalOptimalWeek > 0 ? (totalAssignments * 100) / totalOptimalWeek : 0;
    lblCapacityScore->setText(QString::number(qMin(capacityPct, 100)) + "%");
    if(capacityPct < 80) lblCapacityScore->setStyleSheet("font-size: 24px; font-weight: 900; border: none; color: #E24B4A;");
    else if(capacityPct < 100) lblCapacityScore->setStyleSheet("font-size: 24px; font-weight: 900; border: none; color: #EF9F27;");
    else lblCapacityScore->setStyleSheet("font-size: 24px; font-weight: 900; border: none; color: #1D9E75;");
}

void WorkforcePlanner::refreshHeatmap() {
    heatmapGrid->clearContents();
    
    int optimalPerShift = config.minPressOperator + config.minMaintenance + config.minQualityControl + config.minLogistics;
    
    for(int s=0; s<3; s++) {
        for(int d=0; d<7; d++) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter);
            
            QSqlQuery q;
            q.prepare("SELECT COUNT(*) FROM SHIFT_SCHEDULE WHERE WEEK_NUMBER = :wk AND YEAR = :yr AND SHIFT_TYPE = :sh AND DAY_OF_WEEK = :day AND LEAVE_START IS NULL");
            q.bindValue(":wk", currentWeek);
            q.bindValue(":yr", currentYear);
            q.bindValue(":sh", s);
            q.bindValue(":day", d);
            q.exec();
            
            int c = 0;
            if(q.next()) c = q.value(0).toInt();
            
            if(c == 0) {
                item->setBackground(QColor("#E24B4A")); // Red
                item->setText("Critical");
            } else if(c < optimalPerShift) {
                item->setBackground(QColor("#EF9F27")); // Amber
                item->setText("Understaffed");
            } else {
                item->setBackground(QColor("#1D9E75")); // Green
                item->setText("Optimal");
            }
            heatmapGrid->setItem(s, d, item);
        }
    }
}

void WorkforcePlanner::onCellClicked(int row, int col) {
    selectedShift = row;
    selectedDay = col;
    
    QString shiftNames[] = {"Morning", "Afternoon", "Night"};
    QString dayNames[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    detailTitle->setText(QString("%1 Shift - %2").arg(shiftNames[row]).arg(dayNames[col]));
    aiRecommendationLabel->setText("");
    
    QLayoutItem *child;
    while ((child = chipsContainer->layout()->takeAt(0)) != nullptr) {
        if(child->widget()) child->widget()->deleteLater();
        delete child;
    }
    
    QSqlQuery q;
    q.prepare(
        "SELECT ss.ID_PERSONNEL, p.NOM_PERSONNEL, p.GRADE, "
        "  (SELECT COUNT(*) FROM SHIFT_SCHEDULE lv "
        "   WHERE lv.ID_PERSONNEL = ss.ID_PERSONNEL "
        "     AND lv.WEEK_NUMBER = :wk AND lv.YEAR = :yr "
        "     AND lv.LEAVE_START IS NOT NULL AND lv.LEAVE_APPROVED = 'Approved') as HasConflict "
        "FROM SHIFT_SCHEDULE ss "
        "JOIN PERSONNEL p ON ss.ID_PERSONNEL = p.ID_PERSONNEL "
        "WHERE ss.WEEK_NUMBER = :wk AND ss.YEAR = :yr "
        "  AND ss.LEAVE_START IS NULL AND ss.SHIFT_TYPE = :sh AND ss.DAY_OF_WEEK = :day"
    );
    q.bindValue(":wk", currentWeek);
    q.bindValue(":yr", currentYear);
    q.bindValue(":sh", row);
    q.bindValue(":day", col);
    q.exec();
    
    while(q.next()) {
        QString name = q.value(1).toString();
        QString grade = q.value(2).toString();
        int conflict = q.value(3).toInt();
        
        QWidget *chip = new QWidget();
        QHBoxLayout *chipLayout = new QHBoxLayout(chip);
        chipLayout->setContentsMargins(10, 2, 5, 2);
        chipLayout->setSpacing(5);

        QLabel *nameLabel = new QLabel(QString("%1 (%2)").arg(name).arg(grade));
        nameLabel->setStyleSheet("border: none; background: transparent; font-weight: bold;");

        QPushButton *btnRemove = new QPushButton("×");
        btnRemove->setFixedSize(18, 18);
        btnRemove->setCursor(Qt::PointingHandCursor);
        btnRemove->setStyleSheet("QPushButton { color: inherit; border: none; font-size: 14px; font-weight: bold; background: transparent; } QPushButton:hover { color: #dc2626; }");
        
        QString empId = q.value(0).toString();
        connect(btnRemove, &QPushButton::clicked, [this, empId](){
            removeStaffFromShift(empId);
        });

        if(conflict > 0) {
            chip->setStyleSheet("background-color: #fca5a5; color: #991b1b; border-radius: 12px; border: 1px solid #ef4444;");
            nameLabel->setText(nameLabel->text() + " [CONFLICT: On Leave]");
        } else {
            chip->setStyleSheet("background-color: #d1fae5; color: #065f46; border-radius: 12px; border: 1px solid #10b981;");
        }

        chipLayout->addWidget(nameLabel);
        chipLayout->addWidget(btnRemove);
        chipsContainer->layout()->addWidget(chip);
    }
    
    static_cast<QVBoxLayout*>(chipsContainer->layout())->addStretch();
    detailPanel->setVisible(true);
}

void WorkforcePlanner::refreshDeptCoverage() {
    QLayoutItem *child;
    while ((child = deptCoverageLayout->takeAt(0)) != nullptr) {
        if(child->widget()) child->widget()->deleteLater();
        delete child;
    }
    while ((child = forecastLayout->takeAt(0)) != nullptr) {
        if(child->widget()) child->widget()->deleteLater();
        delete child;
    }
    
    QStringList roles = {"Press Operator", "Maintenance", "Quality Control", "Logistics"};
    for(const QString &r : roles) {
        int req = getMinRequired(r) * 21; // min per week total
        QSqlQuery q;
        q.prepare("SELECT COUNT(*) FROM SHIFT_SCHEDULE ss JOIN PERSONNEL p ON ss.ID_PERSONNEL = p.ID_PERSONNEL WHERE ss.WEEK_NUMBER = :wk AND ss.YEAR = :yr AND ss.LEAVE_START IS NULL AND p.ROLE = :role");
        q.bindValue(":wk", currentWeek);
        q.bindValue(":yr", currentYear);
        q.bindValue(":role", r);
        q.exec();
        
        int actual = 0;
        if(q.next()) actual = q.value(0).toInt();
        
        QWidget *row = new QWidget();
        QHBoxLayout *l = new QHBoxLayout(row);
        QLabel *lbl = new QLabel(r);
        lbl->setFixedWidth(120);
        
        QProgressBar *pb = new QProgressBar();
        pb->setMaximum(req > 0 ? req : 1);
        pb->setValue(actual);
        pb->setFormat("%v / %m shifts");
        pb->setAlignment(Qt::AlignCenter);
        pb->setStyleSheet(actual >= req ? "QProgressBar::chunk { background-color: #1D9E75; }" : "QProgressBar::chunk { background-color: #EF9F27; }");
        
        l->addWidget(lbl);
        l->addWidget(pb);
        deptCoverageLayout->addWidget(row);
    }
    
    // Populate 4-Week Forecast grid
    QStringList fRoles = {"Operator", "Maint.", "QC", "Logs"};
    for (int col = 0; col < 4; col++) {
        int w = currentWeek + col + 1;
        int y = currentYear;
        if (w > 52) { w -= 52; y++; }
        
        QLabel *lblWeek = new QLabel(QString("Wk %1").arg(w));
        lblWeek->setStyleSheet("font-weight: bold; font-size: 11px; color: #6b7280;");
        lblWeek->setAlignment(Qt::AlignCenter);
        forecastLayout->addWidget(lblWeek, 0, col + 1);
    }
    
    for (int rIdx = 0; rIdx < roles.size(); rIdx++) {
        QLabel *lblRole = new QLabel(fRoles[rIdx]);
        lblRole->setStyleSheet("font-weight: bold; font-size: 11px; color: #374151;");
        forecastLayout->addWidget(lblRole, rIdx + 1, 0);
        
        int req = getMinRequired(roles[rIdx]) * 21;
        
        for (int col = 0; col < 4; col++) {
            int w = currentWeek + col + 1;
            int y = currentYear;
            if (w > 52) { w -= 52; y++; }
            
            QSqlQuery fq;
            fq.prepare("SELECT COUNT(*) FROM SHIFT_SCHEDULE ss JOIN PERSONNEL p ON ss.ID_PERSONNEL = p.ID_PERSONNEL WHERE ss.WEEK_NUMBER = :wk AND ss.YEAR = :yr AND ss.LEAVE_START IS NULL AND p.ROLE = :role");
            fq.bindValue(":wk", w);
            fq.bindValue(":yr", y);
            fq.bindValue(":role", roles[rIdx]);
            fq.exec();
            
            int actual = 0;
            if (fq.next()) actual = fq.value(0).toInt();
            
            QLabel *val = new QLabel(QString::number(actual));
            val->setAlignment(Qt::AlignCenter);
            if (actual >= req) {
                val->setStyleSheet("background-color: #d1fae5; color: #065f46; border-radius: 4px; padding: 2px; font-weight: bold;");
            } else if (actual > req / 2) {
                val->setStyleSheet("background-color: #fef3c7; color: #92400e; border-radius: 4px; padding: 2px; font-weight: bold;");
            } else {
                val->setStyleSheet("background-color: #fee2e2; color: #991b1b; border-radius: 4px; padding: 2px; font-weight: bold;");
            }
            forecastLayout->addWidget(val, rIdx + 1, col + 1);
        }
    }
}

void WorkforcePlanner::refreshAssignments() {
    QLayoutItem *child;
    while ((child = shiftAssignmentsLayout->takeAt(0)) != nullptr) {
        if(child->widget()) child->widget()->deleteLater();
        delete child;
    }
    
    QTableWidget *table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Shift Type", "Day of Week", "Employee Name", "Role", "Grade"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setStyleSheet(
        "QTableWidget { border: 1px solid #e5e7eb; border-radius: 4px; background: white; }"
        "QHeaderView::section { background-color: #f9fafb; padding: 8px; border: none; border-bottom: 2px solid #e5e7eb; font-weight: bold; color: #4b5563; }"
    );
    
    QSqlQuery q;
    q.prepare("SELECT ss.SHIFT_TYPE, ss.DAY_OF_WEEK, p.NOM_PERSONNEL, p.ROLE, p.GRADE "
              "FROM SHIFT_SCHEDULE ss "
              "JOIN PERSONNEL p ON ss.ID_PERSONNEL = p.ID_PERSONNEL "
              "WHERE ss.WEEK_NUMBER = :wk AND ss.YEAR = :yr AND ss.LEAVE_START IS NULL "
              "ORDER BY ss.SHIFT_TYPE, ss.DAY_OF_WEEK");
    q.bindValue(":wk", currentWeek);
    q.bindValue(":yr", currentYear);
    q.exec();
    
    int row = 0;
    QString shiftNames[] = {"Morning", "Afternoon", "Night"};
    QString dayNames[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    while(q.next()) {
        table->insertRow(row);
        int shiftType = q.value(0).toInt();
        int dayOfWeek = q.value(1).toInt();
        table->setItem(row, 0, new QTableWidgetItem(shiftNames[shiftType]));
        table->setItem(row, 1, new QTableWidgetItem(dayNames[dayOfWeek]));
        table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
        row++;
    }
    
    shiftAssignmentsLayout->addWidget(table);
}

QString WorkforcePlanner::generateWorkforceSnapshotJson() {
    QJsonObject root;
    root["week"] = currentWeek;
    root["year"] = currentYear;
    root["selected_shift"] = selectedShift;
    root["selected_day"] = selectedDay;
    
    QJsonArray staffArr;
    QSqlQuery q;
    q.prepare(
        "SELECT p.ID_PERSONNEL, p.NOM_PERSONNEL, p.ROLE, p.GRADE, "
        "  (SELECT COUNT(*) FROM SHIFT_SCHEDULE lv WHERE lv.ID_PERSONNEL = ss.ID_PERSONNEL AND lv.WEEK_NUMBER = :wk AND lv.YEAR = :yr AND lv.LEAVE_START IS NOT NULL) as LeaveConf "
        "FROM SHIFT_SCHEDULE ss "
        "JOIN PERSONNEL p ON ss.ID_PERSONNEL = p.ID_PERSONNEL "
        "WHERE ss.WEEK_NUMBER = :wk AND ss.YEAR = :yr AND ss.LEAVE_START IS NULL AND ss.SHIFT_TYPE = :sh AND ss.DAY_OF_WEEK = :day"
    );
    q.bindValue(":wk", currentWeek);
    q.bindValue(":yr", currentYear);
    q.bindValue(":sh", selectedShift);
    q.bindValue(":day", selectedDay);
    q.exec();
    
    while(q.next()) {
        QJsonObject s;
        s["id"] = q.value(0).toString();
        s["name"] = q.value(1).toString();
        s["role"] = q.value(2).toString();
        s["grade"] = q.value(3).toString();
        s["has_leave_conflict"] = q.value(4).toInt() > 0;
        staffArr.append(s);
    }
    root["assigned_staff"] = staffArr;

    QJsonArray rosterArr;
    QSqlQuery rq;
    rq.prepare("SELECT ID_PERSONNEL, NOM_PERSONNEL, ROLE, GRADE FROM PERSONNEL WHERE STATUS = 'Active' AND ID_PERSONNEL NOT IN (SELECT ID_PERSONNEL FROM SHIFT_SCHEDULE WHERE WEEK_NUMBER = :wk AND YEAR = :yr AND ((SHIFT_TYPE = :sh AND DAY_OF_WEEK = :day) OR LEAVE_START IS NOT NULL))");
    rq.bindValue(":wk", currentWeek);
    rq.bindValue(":yr", currentYear);
    rq.bindValue(":sh", selectedShift);
    rq.bindValue(":day", selectedDay);
    rq.exec();

    while(rq.next()) {
        QJsonObject r;
        r["id"] = rq.value(0).toString();
        r["name"] = rq.value(1).toString();
        r["role"] = rq.value(2).toString();
        r["grade"] = rq.value(3).toString();
        rosterArr.append(r);
    }
    root["available_roster"] = rosterArr;
    QJsonDocument doc(root);
    return QString(doc.toJson(QJsonDocument::Compact));
}

void WorkforcePlanner::generateAiRecommendation() {
    aiRecommendationLabel->setText("<i>Analyzing workforce data with Google Gemini...</i>");
    btnApplyAi->setVisible(false);
    pendingAiJson = "";
    
    QString snapshot = generateWorkforceSnapshotJson();
    
    QString endpoint = "https://generativelanguage.googleapis.com/v1/models/gemini-2.5-flash-latest:generateContent?key=" + GEMINI_API_KEY;
    QUrl url(endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString prompt = "You are an AI operational analyst for an oil press. Read this raw JSON snapshot of a shift schedule and identify conflicts (e.g. Leave overlaps, missing leads, headcount drops below safe operating limits based on role).\n\n"
                     "1. Produce an English operational assessment paragraph explaining the risks.\n"
                     "2. Produce a numbered list of fixes.\n"
                     "3. Produce a structured machine block at the very end of your response inside <ACTION_PLAN> tags. This block must be a valid JSON array of objects with 'action' (assign/remove), 'id' (employee id), 'shift', and 'day'.\n"
                     "CRITICAL: Use the 'available_roster' provided in the JSON to find realistic candidate IDs when issuing 'assign' actions!\n\n"
                     "JSON Snapshot:\n" + snapshot;
    
    QJsonObject textPart;
    textPart["text"] = prompt;
    
    QJsonArray parts;
    parts.append(textPart);
    
    QJsonObject contentObj;
    contentObj["role"] = "user";
    contentObj["parts"] = parts;
    
    QJsonArray contents;
    contents.append(contentObj);
    
    QJsonObject root;
    root["contents"] = contents;
    
    QJsonDocument doc(root);
    networkManager->post(request, doc.toJson());
}

QString WorkforcePlanner::generateWeeklyWorkforceSnapshotJson() {
    QJsonObject root;
    root["week"] = currentWeek;
    root["year"] = currentYear;
    
    QJsonArray weekShifts;
    for (int day = 0; day < 7; ++day) {
        for (int shift = 0; shift < 3; ++shift) {
            QJsonObject shiftObj;
            shiftObj["day"] = day; 
            shiftObj["shift"] = shift; 
            
            QJsonArray staffArr;
            QSqlQuery q;
            q.prepare("SELECT p.ID_PERSONNEL, p.ROLE "
                      "FROM SHIFT_SCHEDULE ss JOIN PERSONNEL p ON ss.ID_PERSONNEL = p.ID_PERSONNEL "
                      "WHERE ss.WEEK_NUMBER = :wk AND ss.YEAR = :yr AND ss.LEAVE_START IS NULL AND ss.SHIFT_TYPE = :sh AND ss.DAY_OF_WEEK = :day");
            q.bindValue(":wk", currentWeek);
            q.bindValue(":yr", currentYear);
            q.bindValue(":sh", shift);
            q.bindValue(":day", day);
            q.exec();
            while(q.next()) {
                QJsonObject s;
                s["id"] = q.value(0).toString();
                s["role"] = q.value(1).toString();
                staffArr.append(s); 
            }
            shiftObj["assigned_staff"] = staffArr;
            
            QJsonArray availArr;
            QSqlQuery rq;
            rq.prepare("SELECT ID_PERSONNEL, ROLE FROM PERSONNEL WHERE STATUS = 'Active' AND ID_PERSONNEL NOT IN (SELECT ID_PERSONNEL FROM SHIFT_SCHEDULE WHERE WEEK_NUMBER = :wk AND YEAR = :yr AND ((SHIFT_TYPE = :sh AND DAY_OF_WEEK = :day) OR LEAVE_START IS NOT NULL))");
            rq.bindValue(":wk", currentWeek);
            rq.bindValue(":yr", currentYear);
            rq.bindValue(":sh", shift);
            rq.bindValue(":day", day);
            rq.exec();
            while(rq.next()) {
                QJsonObject a;
                a["id"] = rq.value(0).toString();
                a["role"] = rq.value(1).toString();
                availArr.append(a);
            }
            shiftObj["available_staff"] = availArr;
            weekShifts.append(shiftObj);
        }
    }
    
    root["week_shifts"] = weekShifts;
    QJsonObject reqs;
    reqs["Press Operator"] = config.minPressOperator;
    reqs["Maintenance"] = config.minMaintenance;
    reqs["Quality Control"] = config.minQualityControl;
    reqs["Logistics"] = config.minLogistics;
    root["requirements_per_shift"] = reqs;
    
    QJsonDocument doc(root);
    return QString(doc.toJson(QJsonDocument::Compact));
}

void WorkforcePlanner::generateAiWeekRecommendation() {
    detailPanel->setVisible(true);
    aiRecommendationLabel->setText("<i>Analyzing all 21 shifts for the week globally with Google Gemini (Bulk AI). This may take a few seconds...</i>");
    btnApplyAi->setVisible(false);
    pendingAiJson = "";
    
    QString snapshot = generateWeeklyWorkforceSnapshotJson();
    QString endpoint = "https://generativelanguage.googleapis.com/v1/models/gemini-2.5-flash-latest:generateContent?key=" + GEMINI_API_KEY;
    QUrl url(endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString prompt = "You are an elite AI scheduling engine. You must analyze the 21-shift payload and assign missing roles. "
                     "RULES:\n"
                     "1. DO NOT write any explanations, greetings, or analysis text.\n"
                     "2. Output EXACTLY and ONLY the <ACTION_PLAN> block.\n"
                     "3. Use ONLY IDs explicitly listed in 'available_staff' for the corresponding timeslot.\n"
                     "4. Output format MUST be:\n"
                     "<ACTION_PLAN>\n[\n  { \"action\": \"assign\", \"id\": \"123\", \"shift\": 0, \"day\": 0 }\n]\n</ACTION_PLAN>\n\n"
                     "Data:\n" + snapshot;
                     
    QJsonObject textPart;
    textPart["text"] = prompt;
    QJsonArray parts; parts.append(textPart);
    QJsonObject contentObj;
    contentObj["role"] = "user";
    contentObj["parts"] = parts;
    QJsonArray contents; contents.append(contentObj);
    QJsonObject root; root["contents"] = contents;
    
    QJsonObject generationConfig;
    generationConfig["maxOutputTokens"] = 8192;
    generationConfig["temperature"] = 0.1;
    root["generationConfig"] = generationConfig;
    
    QJsonDocument doc(root);
    networkManager->post(request, doc.toJson());
}

void WorkforcePlanner::handleAiResponse(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseStr = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseStr);
        QJsonObject root = doc.object();
        
        if (root.contains("candidates")) {
            QJsonArray candidatesArray = root["candidates"].toArray();
            if (!candidatesArray.isEmpty()) {
                QJsonObject contentObj = candidatesArray[0].toObject()["content"].toObject();
                QJsonArray partsArray = contentObj["parts"].toArray();
                if (!partsArray.isEmpty()) {
                    QString text = partsArray[0].toObject()["text"].toString();
                    
                    // Separate the visible text from the hidden action plan
                    QString displayPart = text;
                    int startIdx = text.indexOf("<ACTION_PLAN>");
                    int endIdx = text.indexOf("</ACTION_PLAN>");
                    
                    if (startIdx != -1) {
                        displayPart = text.left(startIdx).trimmed();
                        if (displayPart.isEmpty()) displayPart = "Bulk schedule generated successfully. Click Apply.";
                        
                        if (endIdx != -1) {
                            pendingAiJson = text.mid(startIdx + 13, endIdx - (startIdx + 13)).trimmed();
                        } else {
                            // Recover truncated response by grabbing everything after <ACTION_PLAN>
                            // and capping at the last ']'
                            QString truncated = text.mid(startIdx + 13);
                            int lastBracket = truncated.lastIndexOf(']');
                            if (lastBracket != -1) {
                                pendingAiJson = truncated.left(lastBracket + 1).trimmed();
                            } else {
                                pendingAiJson = truncated.trimmed() + "\n]";
                            }
                        }
                        btnApplyAi->setVisible(true);
                    }
                    
                    aiRecommendationLabel->setText("<b>AI Analysis:</b><br><br>" + displayPart.replace("\n", "<br>"));
                    reply->deleteLater();
                    return;
                }
            }
        }
        aiRecommendationLabel->setText("Error: Unexpected API response format.");
    } else {
        QByteArray errorBody = reply->readAll();
        aiRecommendationLabel->setText("<b>API Network Error:</b> " + reply->errorString() + 
                                     "<br><b>Server Response:</b> " + QString::fromUtf8(errorBody) + 
                                     "<br>Did you replace YOUR_GEMINI_API_KEY_HERE at the top of workforceplanner.cpp?");
    }
    reply->deleteLater();
}

void WorkforcePlanner::applyAiRecommendations() {
    if (pendingAiJson.isEmpty()) return;
    
    QString cleanedJson = pendingAiJson;
    cleanedJson.replace(QRegularExpression("```json\\s*"), "");
    cleanedJson.replace(QRegularExpression("```\\s*"), "");
    cleanedJson = cleanedJson.trimmed();
    
    QJsonDocument doc = QJsonDocument::fromJson(cleanedJson.toUtf8());
    if (!doc.isArray()) {
        aiRecommendationLabel->setText(aiRecommendationLabel->text() + "<br><br><font color='red'>Error: AI returned invalid action format.</font>");
        return;
    }
    
    QJsonArray actions = doc.array();
    int successCount = 0;
    
    for (int i = 0; i < actions.size(); ++i) {
        QJsonObject obj = actions[i].toObject();
        QString action = obj["action"].toString();
        QString idPerson = obj["id"].toVariant().toString();
        int shift = obj["shift"].toInt();
        int day = obj["day"].toInt();
        
        QSqlQuery q;
        if (action == "assign") {
            // First check if already assigned to avoid constraint violation
            QSqlQuery check;
            check.prepare("SELECT COUNT(*) FROM SHIFT_SCHEDULE WHERE ID_PERSONNEL = :id AND WEEK_NUMBER = :wk AND YEAR = :yr AND SHIFT_TYPE = :sh AND DAY_OF_WEEK = :day AND LEAVE_START IS NULL");
            check.bindValue(":id", idPerson);
            check.bindValue(":wk", currentWeek);
            check.bindValue(":yr", currentYear);
            check.bindValue(":sh", shift);
            check.bindValue(":day", day);
            check.exec();
            if (check.next() && check.value(0).toInt() == 0) {
                q.prepare("INSERT INTO SHIFT_SCHEDULE (ID, ID_PERSONNEL, WEEK_NUMBER, YEAR, SHIFT_TYPE, DAY_OF_WEEK) "
                          "VALUES (NVL((SELECT MAX(ID) FROM SHIFT_SCHEDULE), 0) + 1, :id, :wk, :yr, :sh, :day)");
                q.bindValue(":id", idPerson);
                q.bindValue(":wk", currentWeek);
                q.bindValue(":yr", currentYear);
                q.bindValue(":sh", shift);
                q.bindValue(":day", day);
                if (q.exec()) successCount++;
            }
        } else if (action == "remove") {
            q.prepare("DELETE FROM SHIFT_SCHEDULE WHERE ID_PERSONNEL = :id AND WEEK_NUMBER = :wk AND YEAR = :yr AND SHIFT_TYPE = :sh AND DAY_OF_WEEK = :day AND LEAVE_START IS NULL");
            q.bindValue(":id", idPerson);
            q.bindValue(":wk", currentWeek);
            q.bindValue(":yr", currentYear);
            q.bindValue(":sh", shift);
            q.bindValue(":day", day);
            if (q.exec()) successCount++;
        }
    }
    
    QMessageBox::information(this, "AI Planner", QString("Applied %1 schedule adjustments successfully.").arg(successCount));
    btnApplyAi->setVisible(false);
    refreshAll();
}

void WorkforcePlanner::onManualAssignClicked() {
    if (selectedShift == -1) return;
    
    QDialog dialog(this);
    dialog.setWindowTitle("Assign Employee to Shift");
    dialog.setMinimumWidth(300);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    layout->addWidget(new QLabel("Select an active employee to assign:"));
    
    QTableWidget *empTable = new QTableWidget();
    empTable->setColumnCount(2);
    empTable->setHorizontalHeaderLabels({"CIN", "Employee Name"});
    empTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    empTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    empTable->setSelectionMode(QAbstractItemView::SingleSelection);
    
    QSqlQuery q("SELECT ID_PERSONNEL, NOM_PERSONNEL FROM PERSONNEL WHERE STATUS = 'Active' ORDER BY NOM_PERSONNEL");
    int row = 0;
    while(q.next()){
        empTable->insertRow(row);
        empTable->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        empTable->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        row++;
    }
    layout->addWidget(empTable);
    
    QPushButton *btnConfirm = new QPushButton("Confirm Assignment");
    btnConfirm->setStyleSheet("background: #1D9E75; color: white; padding: 8px; border-radius: 4px; font-weight: bold;");
    layout->addWidget(btnConfirm);
    
    connect(btnConfirm, &QPushButton::clicked, [&](){
        auto selected = empTable->selectedItems();
        if (selected.isEmpty()) return;
        
        QString id = selected[0]->text();
        QSqlQuery ins;
        ins.prepare("INSERT INTO SHIFT_SCHEDULE (ID, ID_PERSONNEL, WEEK_NUMBER, YEAR, SHIFT_TYPE, DAY_OF_WEEK) "
                    "VALUES (NVL((SELECT MAX(ID) FROM SHIFT_SCHEDULE), 0) + 1, :id, :wk, :yr, :sh, :day)");
        ins.bindValue(":id", id);
        ins.bindValue(":wk", currentWeek);
        ins.bindValue(":yr", currentYear);
        ins.bindValue(":sh", selectedShift);
        ins.bindValue(":day", selectedDay);
        
        if (ins.exec()) {
            dialog.accept();
            refreshAll();
        } else {
            QMessageBox::critical(&dialog, "Error", ins.lastError().text());
        }
    });
    
    dialog.exec();
}

void WorkforcePlanner::removeStaffFromShift(const QString &id) {
    QSqlQuery q;
    q.prepare("DELETE FROM SHIFT_SCHEDULE WHERE ID_PERSONNEL = :id AND WEEK_NUMBER = :wk AND YEAR = :yr AND SHIFT_TYPE = :sh AND DAY_OF_WEEK = :day AND LEAVE_START IS NULL");
    q.bindValue(":id", id);
    q.bindValue(":wk", currentWeek);
    q.bindValue(":yr", currentYear);
    q.bindValue(":sh", selectedShift);
    q.bindValue(":day", selectedDay);
    if (q.exec()) {
        refreshAll();
    }
}
