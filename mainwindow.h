#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QMap>
#include <QTimer>
#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    int remainingTime;
    bool isCompleted;
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
    void on_startSimulationButton_clicked();
    void on_clearButton_clicked();
    void on_resetButton_clicked();
    void updateSimulation();
    void drawGanttChart();
    void drawMetricsChart();

private:
    Ui::MainWindow *ui;
    QVector<Process> processes;
    QTimer *simulationTimer;
    int currentTime;
    int currentProcessIndex;
    QGraphicsScene *ganttChartScene;
    QGraphicsScene *metricsScene;
    bool simulationRunning;
    bool simulationComplete;
    
    void setupCharts();
    void updateProcessTable();
    void calculateMetrics();
    QColor getRandomColor();
    void sortProcessesByArrivalTime();
    void resetSimulation();
};

#endif // MAINWINDOW_H