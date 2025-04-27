#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QTimer>
#include <QGraphicsScene>

namespace Ui {
class MainWindow;
}

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int waitingTime;
    int turnaroundTime;
    int completionTime;
    int remainingTime;
    
    Process(int _id, int _arrival, int _burst) : 
        id(_id), arrivalTime(_arrival), burstTime(_burst), 
        waitingTime(0), turnaroundTime(0), completionTime(0), 
        remainingTime(_burst) {}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addProcess();
    void startSimulation();
    void resetSimulation();
    void updateSimulation();

private:
    Ui::MainWindow *ui;
    
    // Simulation data
    QVector<Process> processes;
    QVector<Process> readyQueue;
    Process *currentProcess;
    bool simulationRunning;
    int currentTime;
    int nextProcessIndex;
    
    // Graphics
    QGraphicsScene *ganttChart;
    QTimer *simulationTimer;
    
    // Helper methods
    void setupConnections();
    void updateProcessTable();
    void updateResultTable();
    void updateStatistics();
    void drawGanttChart();
    QColor getProcessColor(int processId);
    void sortProcessesByArrivalTime();
};

#endif // MAINWINDOW_H