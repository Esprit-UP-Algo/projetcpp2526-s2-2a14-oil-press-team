#ifndef WORKFORCEPLANNER_H
#define WORKFORCEPLANNER_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlQuery>
#include <QDate>
#include <QSqlError>
#include <QVariant>
#include <QMap>

struct ScheduleConfig {
    int minPressOperator = 2;
    int minMaintenance = 1;
    int minQualityControl = 1;
    int minLogistics = 1;
    int maxWeeklyHours = 40;
};

class WorkforcePlanner : public QWidget {
    Q_OBJECT

public:
    explicit WorkforcePlanner(QWidget *parent = nullptr);
    ~WorkforcePlanner();

private slots:
    void nextWeek();
    void prevWeek();
    void handleAiResponse(QNetworkReply *reply);
    void generateAiRecommendation();
    void generateAiWeekRecommendation();
    void applyAiRecommendations();
    void onCellClicked(int row, int col);
    void onManualAssignClicked();
    void removeStaffFromShift(const QString &id);
    void refreshAll();

private:
    void setupUi();
    void updateHeader();
    void refreshKpis();
    void refreshHeatmap();
    void refreshDeptCoverage();
    void refreshAssignments();
    QWidget* createHeatmapDetailPanel();
    
    QString generateWorkforceSnapshotJson();
    QString generateWeeklyWorkforceSnapshotJson();

    int selectedShift;
    int selectedDay;
    int currentWeek;
    int currentYear;
    ScheduleConfig config;
    QNetworkAccessManager *networkManager;

    QLabel *weekLabel;
    QLabel *lblActiveStaff;
    QLabel *lblCoverageGaps;
    QLabel *lblOverscheduled;
    QLabel *lblCapacityScore;
    
    QTableWidget *heatmapGrid;
    QWidget *detailPanel;
    QVBoxLayout *detailLayout;
    QLabel *detailTitle;
    QWidget *chipsContainer;
    QLabel *aiRecommendationLabel;
    QPushButton *btnApplyAi;
    QString pendingAiJson;

    QVBoxLayout *deptCoverageLayout;
    QGridLayout *forecastLayout;
    QVBoxLayout *shiftAssignmentsLayout;
    
    // Hardcoded roles mapping
    int getMinRequired(const QString &role);
};

#endif // WORKFORCEPLANNER_H
