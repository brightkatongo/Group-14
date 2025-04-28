#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QVector>
#include <QTableWidgetItem>
#include <QChart>
#include <QBarSeries>
#include <QBarSet>
#include <QChartView>
#include <QPieSeries>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int priority;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    int responseTime;
    int remainingTime;
    int startTime;
    QString status;  // "Waiting", "Running", "Completed"
    QColor color;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addProcessButton_clicked();
    void on_removeProcessButton_clicked();
    void on_clearAllButton_clicked();
    void on_importButton_clicked();
    void on_exportButton_clicked();
    void on_startSimulationButton_clicked();
    void on_resetButton_clicked();
    void on_speedSlider_valueChanged(int value);
    void on_processTableWidget_cellClicked(int row, int column);
    void on_actionImport_triggered();
    void on_actionExport_triggered();
    void on_actionExit_triggered();
    void on_actionStart_triggered();
    void on_actionPause_triggered();
    void on_actionReset_triggered();
    void on_actionAbout_triggered();
    void on_actionDocumentation_triggered();

    void simulationStep();
    void updateGanttChart();
    void updatePerformanceCharts();

private:
    Ui::MainWindow *ui;
    QVector<Process> processes;
    QGraphicsScene *ganttChartScene;
    QTimer *simulationTimer;
    int currentTime;
    int currentProcessIndex;
    bool simulationRunning;
    QMap<int, QColor> processColors;
    
    // Chart related members
    QChart *cpuUtilizationChart;
    QChart *waitingTimeChart;
    QChart *turnaroundTimeChart;
    QChart *responseTimeChart;
    QChart *processComparisonChart;
    QChart *throughputChart;
    QChart *cpuDistributionChart;
    QChartView *cpuUtilizationView;
    QChartView *waitingTimeView;
    QChartView *turnaroundTimeView;
    QChartView *responseTimeView;
    QChartView *processComparisonView;
    QChartView *throughputView;
    QChartView *cpuDistributionView;

    void setupProcessTable();
    void setupGanttChart();
    void setupPerformanceCharts();
    void updateProcessTable();
    void updateProcessDetails(int processIndex);
    void updateSimulationStats();
    QColor getRandomColor();
    void resetSimulation();
    void initializeSimulation();
    void sortProcessesByArrivalTime();
    void startSimulation();
    void pauseSimulation();
    void finishSimulation();
    bool validateProcessInput();
};
#endif // MAINWINDOW_H