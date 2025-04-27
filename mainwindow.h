#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <QQueue>
#include <QLabel>
#include <QMap>
#include <QColor>
#include <QBarSet>
#include <QBarSeries>
#include <QPieSeries>
#include <QChartView>
#include <QPushButton>
#include <QGridLayout>
#include <QTableWidget>
#include <QFileDialog>

// Forward declarations for Qt Charts
QT_BEGIN_NAMESPACE
namespace QtCharts {
    class QChart;
    class QLineSeries;
    class QBarSeries;
    class QPieSeries;
    class QChartView;
}
QT_END_NAMESPACE

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    int responseTime;
    bool isArrived;
    bool isCompleted;
    bool hasStarted;
    QColor color;  // Color for visualization
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addProcessButton_clicked();
    void on_startSimulationButton_clicked();
    void on_resetButton_clicked();
    void updateSimulation();
    void on_speedSlider_valueChanged(int value);
    void on_importButton_clicked();
    void on_exportButton_clicked();
    void on_removeProcessButton_clicked();
    void on_clearAllButton_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_processTableWidget_cellClicked(int row, int column);

private:
    Ui::MainWindow *ui;
    QVector<Process> processes;
    QQueue<int> readyQueue;
    int currentTime;
    int currentProcessIndex;
    QTimer *simulationTimer;
    bool simulationRunning;
    int simulationSpeed;
    QMap<int, QVector<QPair<int, int>>> processTimings; // For Gantt chart
    QVector<int> timeHistory;
    QVector<double> cpuUtilizationHistory;
    QVector<double> avgWaitingTimeHistory;
    QVector<double> avgTurnaroundTimeHistory;
    QVector<double> avgResponseTimeHistory;
    QVector<int> completedProcessesHistory;
    bool isFirstRun;
    QColor getRandomColor();

    // Qt Charts
    QtCharts::QChartView *cpuUtilizationChartView;
    QtCharts::QChartView *waitingTimeChartView;
    QtCharts::QChartView *turnaroundTimeChartView;
    QtCharts::QChartView *responseTimeChartView;
    QtCharts::QChartView *processComparisonChartView;
    QtCharts::QChartView *throughputChartView;
    QtCharts::QChartView *pieChartView;

    void setupCharts();
    void updateGanttChart();
    void updateProcessTable();
    void updateStatistics();
    void updatePerformanceCharts();
    void updateProcessComparisonChart();
    void updatePieChart();
    void updateThroughputChart();
    void calculateStatistics();
    void resetSimulation();
    void highlightCurrentProcess();
    void generateRandomColor(Process &process);
    void exportResults(const QString &filename);
    void importProcesses(const QString &filename);
    void setupAdditionalUi();
    void updateSimulationProgress();
    double calculateCpuUtilization();
    void updateDetailedProcessInfo(int row);
};
#endif // MAINWINDOW_H