#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QPen>
#include <QFont>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentProcess(nullptr),
    simulationRunning(false),
    currentTime(0),
    nextProcessIndex(0)
{
    ui->setupUi(this);
    
    // Initialize graphics scene
    ganttChart = new QGraphicsScene(this);
    ui->ganttChartView->setScene(ganttChart);
    
    // Initialize timer
    simulationTimer = new QTimer(this);
    simulationTimer->setInterval(500); // 500ms
    
    setupConnections();
}

MainWindow::~MainWindow()
{
    if (currentProcess) delete currentProcess;
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::addProcess);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startSimulation);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::resetSimulation);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulation);
}

void MainWindow::addProcess()
{
    if (simulationRunning) {
        QMessageBox::warning(this, "Warning", "Cannot add processes while simulation is running.");
        return;
    }
    
    int id = ui->processIdSpinBox->value();
    int arrival = ui->arrivalTimeSpinBox->value();
    int burst = ui->burstTimeSpinBox->value();
    
    // Check if process ID already exists
    for (const Process &p : processes) {
        if (p.id == id) {
            QMessageBox::warning(this, "Warning", "Process ID already exists. Please use a different ID.");
            return;
        }
    }
    
    processes.append(Process(id, arrival, burst));
    updateProcessTable();
    
    // Increment process ID for convenience
    ui->processIdSpinBox->setValue(id + 1);
}

void MainWindow::updateProcessTable()
{
    ui->processTable->setRowCount(processes.size());
    
    for (int i = 0; i < processes.size(); i++) {
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(processes[i].id));
        QTableWidgetItem *arrivalItem = new QTableWidgetItem(QString::number(processes[i].arrivalTime));
        QTableWidgetItem *burstItem = new QTableWidgetItem(QString::number(processes[i].burstTime));
        
        ui->processTable->setItem(i, 0, idItem);
        ui->processTable->setItem(i, 1, arrivalItem);
        ui->processTable->setItem(i, 2, burstItem);
        
        // Center align cells
        idItem->setTextAlignment(Qt::AlignCenter);
        arrivalItem->setTextAlignment(Qt::AlignCenter);
        burstItem->setTextAlignment(Qt::AlignCenter);
    }
}

void MainWindow::startSimulation()
{
    if (processes.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please add at least one process before starting simulation.");
        return;
    }
    
    if (!simulationRunning) {
        simulationRunning = true;
        ui->startButton->setText("Pause Simulation");
        ui->addButton->setEnabled(false);
        
        // Sort processes by arrival time for FCFS
        sortProcessesByArrivalTime();
        
        // Start simulation timer
        simulationTimer->start();
    } else {
        simulationRunning = false;
        ui->startButton->setText("Resume Simulation");
        simulationTimer->stop();
    }
}

void MainWindow::resetSimulation()
{
    simulationTimer->stop();
    simulationRunning = false;
    ui->startButton->setText("Start Simulation");
    ui->addButton->setEnabled(true);
    
    // Reset simulation state
    currentTime = 0;
    nextProcessIndex = 0;
    readyQueue.clear();
    if (currentProcess) {
        delete currentProcess;
        currentProcess = nullptr;
    }
    
    // Reset process data
    for (int i = 0; i < processes.size(); i++) {
        processes[i].waitingTime = 0;
        processes[i].turnaroundTime = 0;
        processes[i].completionTime = 0;
        processes[i].remainingTime = processes[i].burstTime;
    }
    
    ui->currentTimeLabel->setText("Current Time: 0");
    ui->avgWaitingTimeLabel->setText("Average Waiting Time: 0.00");
    ui->avgTurnaroundTimeLabel->setText("Average Turnaround Time: 0.00");
    
    // Clear gantt chart
    ganttChart->clear();
    
    // Clear result table
    ui->resultTable->setRowCount(0);
}

void MainWindow::updateSimulation()
{
    // Check if new processes have arrived
    while (nextProcessIndex < processes.size() && processes[nextProcessIndex].arrivalTime <= currentTime) {
        readyQueue.append(processes[nextProcessIndex]);
        nextProcessIndex++;
    }
    
    // If no current process, get the next one from ready queue
    if (!currentProcess && !readyQueue.isEmpty()) {
        currentProcess = new Process(readyQueue.first());
        readyQueue.removeFirst();
        
        // Calculate waiting time when process starts execution
        currentProcess->waitingTime = currentTime - currentProcess->arrivalTime;
    }
    
    // If a process is running, update its remaining time
    if (currentProcess) {
        currentProcess->remainingTime--;
        
        // Update gantt chart
        drawGanttChart();
        
        // If process is complete
        if (currentProcess->remainingTime <= 0) {
            currentProcess->completionTime = currentTime + 1;
            currentProcess->turnaroundTime = currentProcess->completionTime - currentProcess->arrivalTime;
            
            // Update the original process in the processes list
            for (int i = 0; i < processes.size(); i++) {
                if (processes[i].id == currentProcess->id) {
                    processes[i].waitingTime = currentProcess->waitingTime;
                    processes[i].turnaroundTime = currentProcess->turnaroundTime;
                    processes[i].completionTime = currentProcess->completionTime;
                    break;
                }
            }
            
            // Update results table
            updateResultTable();
            updateStatistics();
            
            // Delete completed process
            delete currentProcess;
            currentProcess = nullptr;
        }
    }
    
    // Update current time
    currentTime++;
    ui->currentTimeLabel->setText("Current Time: " + QString::number(currentTime));
    
    // Check if simulation is complete
    if (nextProcessIndex >= processes.size() && readyQueue.isEmpty() && !currentProcess) {
        simulationTimer->stop();
        simulationRunning = false;
        ui->startButton->setText("Start Simulation");
        QMessageBox::information(this, "Simulation Complete", "FCFS scheduling simulation has completed.");
    }
}

void MainWindow::drawGanttChart()
{
    const int unit = 30; // Width of one time unit
    const int height = 50; // Height of the gantt chart bar
    
    // Clear previous chart
    ganttChart->clear();
    
    // Draw time scale
    for (int t = 0; t <= currentTime; t++) {
        ganttChart->addLine(t * unit, height + 5, t * unit, height + 15);
        QGraphicsTextItem *timeText = ganttChart->addText(QString::number(t));
        timeText->setPos(t * unit - 5, height + 15);
    }
    
    // Track the last drawn position
    int lastProcessId = -1;
    int startTime = 0;
    
    // Draw processes that have started or completed
    for (int t = 0; t <= currentTime; t++) {
        int processId = -1;
        
        // Find which process was running at time t
        for (const Process &p : processes) {
            if (p.completionTime > 0 && p.completionTime - p.burstTime <= t && p.completionTime > t) {
                processId = p.id;
                break;
            }
        }
        
        // Also check current process
        if (currentProcess && t >= (currentTime - (currentProcess->burstTime - currentProcess->remainingTime))) {
            processId = currentProcess->id;
        }
        
        // If process changes or we're at the end
        if ((processId != lastProcessId || t == currentTime) && lastProcessId != -1) {
            QBrush brush(getProcessColor(lastProcessId));
            QGraphicsRectItem *rect = ganttChart->addRect(
                startTime * unit, 0, (t - startTime) * unit, height, QPen(Qt::black), brush);
                
            // Add process label
            QGraphicsTextItem *processText = ganttChart->addText("P" + QString::number(lastProcessId));
            processText->setDefaultTextColor(Qt::white);
            QFont font = processText->font();
            font.setBold(true);
            processText->setFont(font);
            
            // Center the text in the rectangle
            QRectF textRect = processText->boundingRect();
            qreal x = startTime * unit + ((t - startTime) * unit - textRect.width()) / 2;
            qreal y = (height - textRect.height()) / 2;
            processText->setPos(x, y);
            
            // Update for next segment
            startTime = t;
        }
        
        if (processId != lastProcessId && processId != -1) {
            startTime = t;
        }
        
        lastProcessId = processId;
    }
    
    // Adjust scene rectangle
    ganttChart->setSceneRect(ganttChart->itemsBoundingRect());
    ui->ganttChartView->fitInView(ganttChart->sceneRect(), Qt::KeepAspectRatio);
    ui->ganttChartView->update();
}

QColor MainWindow::getProcessColor(int processId)
{
    // Generate a color based on process ID
    const QVector<QColor> colors = {
        QColor(255, 99, 71),   // Tomato
        QColor(30, 144, 255),  // DodgerBlue
        QColor(50, 205, 50),   // LimeGreen
        QColor(255, 165, 0),   // Orange
        QColor(138, 43, 226),  // BlueViolet
        QColor(0, 139, 139),   // DarkCyan
        QColor(255, 20, 147),  // DeepPink
        QColor(184, 134, 11),  // DarkGoldenrod
        QColor(70, 130, 180),  // SteelBlue
        QColor(139, 69, 19),   // SaddleBrown
    };
    
    return colors[processId % colors.size()];
}

void MainWindow::updateResultTable()
{
    ui->resultTable->setRowCount(processes.size());
    
    for (int i = 0; i < processes.size(); i++) {
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(processes[i].id));
        QTableWidgetItem *waitingItem = new QTableWidgetItem(
            processes[i].completionTime > 0 ? QString::number(processes[i].waitingTime) : "-");
        QTableWidgetItem *turnaroundItem = new QTableWidgetItem(
            processes[i].completionTime > 0 ? QString::number(processes[i].turnaroundTime) : "-");
        QTableWidgetItem *completionItem = new QTableWidgetItem(
            processes[i].completionTime > 0 ? QString::number(processes[i].completionTime) : "-");
        QTableWidgetItem *responseItem = new QTableWidgetItem(
            processes[i].completionTime > 0 ? QString::number(processes[i].waitingTime) : "-");
        
        ui->resultTable->setItem(i, 0, idItem);
        ui->resultTable->setItem(i, 1, waitingItem);
        ui->resultTable->setItem(i, 2, turnaroundItem);
        ui->resultTable->setItem(i, 3, completionItem);
        ui->resultTable->setItem(i, 4, responseItem);
        
        // Center align cells
        idItem->setTextAlignment(Qt::AlignCenter);
        waitingItem->setTextAlignment(Qt::AlignCenter);
        turnaroundItem->setTextAlignment(Qt::AlignCenter);
        completionItem->setTextAlignment(Qt::AlignCenter);
        responseItem->setTextAlignment(Qt::AlignCenter);
    }
}

void MainWindow::updateStatistics()
{
    int completedProcesses = 0;
    double totalWaitingTime = 0;
    double totalTurnaroundTime = 0;
    
    for (const Process &p : processes) {
        if (p.completionTime > 0) {
            completedProcesses++;
            totalWaitingTime += p.waitingTime;
            totalTurnaroundTime += p.turnaroundTime;
        }
    }
    
    if (completedProcesses > 0) {
        double avgWaiting = totalWaitingTime / completedProcesses;
        double avgTurnaround = totalTurnaroundTime / completedProcesses;
        
        ui->avgWaitingTimeLabel->setText("Average Waiting Time: " + QString::number(avgWaiting, 'f', 2));
        ui->avgTurnaroundTimeLabel->setText("Average Turnaround Time: " + QString::number(avgTurnaround, 'f', 2));
    }
}

void MainWindow::sortProcessesByArrivalTime()
{
    // Sort by arrival time (FCFS requirement)
    for (int i = 0; i < processes.size(); i++) {
        for (int j = 0; j < processes.size() - i - 1; j++) {
            if (processes[j].arrivalTime > processes[j + 1].arrivalTime) {
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
    
    // Update process table after sorting
    updateProcessTable();
}