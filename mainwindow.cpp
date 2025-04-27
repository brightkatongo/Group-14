#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QRandomGenerator>
#include <QMessageBox>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QDebug>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentTime(0)
    , currentProcessIndex(-1)
    , simulationRunning(false)
    , simulationComplete(false)
{
    ui->setupUi(this);
    
    // Set up the process table
    ui->processTable->setColumnCount(7);
    ui->processTable->setHorizontalHeaderLabels({"Process ID", "Arrival Time", "Burst Time", 
                                              "Completion Time", "Turnaround Time", "Waiting Time", "Remaining Time"});
    
    // Set up the simulation timer
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    
    // Set up the charts
    setupCharts();
    
    // Disable some buttons initially
    ui->startSimulationButton->setEnabled(false);
    ui->resetButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupCharts()
{
    // Set up Gantt chart
    ganttChartScene = new QGraphicsScene(this);
    ui->ganttChartView->setScene(ganttChartScene);
    
    // Set up metrics chart
    metricsScene = new QGraphicsScene(this);
    ui->metricsView->setScene(metricsScene);
}

void MainWindow::on_addProcessButton_clicked()
{
    // Get process details from the UI
    bool ok;
    int processId = ui->processIdEdit->text().toInt(&ok);
    if (!ok || processId < 0) {
        QMessageBox::warning(this, "Invalid Input", "Process ID must be a positive integer.");
        return;
    }
    
    // Check if process ID already exists
    for (const Process& p : processes) {
        if (p.id == processId) {
            QMessageBox::warning(this, "Invalid Input", "Process ID already exists.");
            return;
        }
    }
    
    int arrivalTime = ui->arrivalTimeEdit->text().toInt(&ok);
    if (!ok || arrivalTime < 0) {
        QMessageBox::warning(this, "Invalid Input", "Arrival time must be a non-negative integer.");
        return;
    }
    
    int burstTime = ui->burstTimeEdit->text().toInt(&ok);
    if (!ok || burstTime <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Burst time must be a positive integer.");
        return;
    }
    
    // Create a new process
    Process newProcess;
    newProcess.id = processId;
    newProcess.arrivalTime = arrivalTime;
    newProcess.burstTime = burstTime;
    newProcess.completionTime = -1;
    newProcess.turnaroundTime = -1;
    newProcess.waitingTime = -1;
    newProcess.remainingTime = burstTime;
    newProcess.isCompleted = false;
    newProcess.color = getRandomColor();
    
    // Add the process to the list
    processes.push_back(newProcess);
    
    // Update the table
    updateProcessTable();
    
    // Clear the input fields
    ui->processIdEdit->clear();
    ui->arrivalTimeEdit->clear();
    ui->burstTimeEdit->clear();
    
    // Enable the start button if at least one process exists
    ui->startSimulationButton->setEnabled(true);
}

void MainWindow::updateProcessTable()
{
    ui->processTable->setRowCount(processes.size());
    
    for (int i = 0; i < processes.size(); i++) {
        const Process& p = processes[i];
        
        // Set the table items
        ui->processTable->setItem(i, 0, new QTableWidgetItem(QString::number(p.id)));
        ui->processTable->setItem(i, 1, new QTableWidgetItem(QString::number(p.arrivalTime)));
        ui->processTable->setItem(i, 2, new QTableWidgetItem(QString::number(p.burstTime)));
        
        // Set completion time, turnaround time, and waiting time if available
        if (p.completionTime != -1) {
            ui->processTable->setItem(i, 3, new QTableWidgetItem(QString::number(p.completionTime)));
            ui->processTable->setItem(i, 4, new QTableWidgetItem(QString::number(p.turnaroundTime)));
            ui->processTable->setItem(i, 5, new QTableWidgetItem(QString::number(p.waitingTime)));
        } else {
            ui->processTable->setItem(i, 3, new QTableWidgetItem("-"));
            ui->processTable->setItem(i, 4, new QTableWidgetItem("-"));
            ui->processTable->setItem(i, 5, new QTableWidgetItem("-"));
        }
        
        // Set remaining time
        ui->processTable->setItem(i, 6, new QTableWidgetItem(QString::number(p.remainingTime)));
    }
}

void MainWindow::on_startSimulationButton_clicked()
{
    if (processes.empty()) {
        QMessageBox::warning(this, "Error", "No processes added yet.");
        return;
    }
    
    if (!simulationRunning) {
        // If simulation is not already running, start it
        if (simulationComplete) {
            resetSimulation();
        }
        
        // Sort processes by arrival time
        sortProcessesByArrivalTime();
        
        // Update UI
        ui->statusLabel->setText("Simulation running...");
        ui->startSimulationButton->setText("Pause");
        ui->addProcessButton->setEnabled(false);
        ui->clearButton->setEnabled(false);
        ui->resetButton->setEnabled(true);
        
        // Start the timer
        simulationRunning = true;
        simulationTimer->start(1000); // Update every 1 second
    } else {
        // If simulation is running, pause it
        simulationTimer->stop();
        simulationRunning = false;
        ui->startSimulationButton->setText("Resume");
        ui->statusLabel->setText("Simulation paused.");
    }
}

void MainWindow::on_clearButton_clicked()
{
    // Clear all processes
    processes.clear();
    
    // Reset UI
    ui->processTable->setRowCount(0);
    ui->startSimulationButton->setEnabled(false);
    ui->resetButton->setEnabled(false);
    ui->statusLabel->setText("No processes added.");
    
    // Clear charts
    ganttChartScene->clear();
    metricsScene->clear();
    
    // Reset simulation variables
    currentTime = 0;
    currentProcessIndex = -1;
    simulationRunning = false;
    simulationComplete = false;
    
    ui->timeLabel->setText("Current Time: 0");
    ui->currentProcessLabel->setText("Current Process: None");
    ui->avgTurnaroundLabel->setText("Average Turnaround Time: -");
    ui->avgWaitingLabel->setText("Average Waiting Time: -");
}

void MainWindow::on_resetButton_clicked()
{
    resetSimulation();
    ui->statusLabel->setText("Simulation reset.");
    ui->startSimulationButton->setText("Start Simulation");
    ui->addProcessButton->setEnabled(true);
    ui->clearButton->setEnabled(true);
}

void MainWindow::resetSimulation()
{
    // Stop the timer
    simulationTimer->stop();
    simulationRunning = false;
    simulationComplete = false;
    
    // Reset process data
    for (auto& p : processes) {
        p.completionTime = -1;
        p.turnaroundTime = -1;
        p.waitingTime = -1;
        p.remainingTime = p.burstTime;
        p.isCompleted = false;
    }
    
    // Update the table
    updateProcessTable();
    
    // Reset simulation variables
    currentTime = 0;
    currentProcessIndex = -1;
    
    // Update UI
    ui->timeLabel->setText("Current Time: 0");
    ui->currentProcessLabel->setText("Current Process: None");
    ui->avgTurnaroundLabel->setText("Average Turnaround Time: -");
    ui->avgWaitingLabel->setText("Average Waiting Time: -");
    
    // Clear charts
    ganttChartScene->clear();
    metricsScene->clear();
}

void MainWindow::updateSimulation()
{
    if (simulationComplete) {
        simulationTimer->stop();
        simulationRunning = false;
        ui->startSimulationButton->setText("Start Simulation");
        ui->statusLabel->setText("Simulation complete.");
        return;
    }
    
    // Update current time
    currentTime++;
    ui->timeLabel->setText("Current Time: " + QString::number(currentTime));
    
    // Find the next process to execute if none is currently running
    if (currentProcessIndex == -1) {
        for (int i = 0; i < processes.size(); i++) {
            if (!processes[i].isCompleted && processes[i].arrivalTime <= currentTime) {
                currentProcessIndex = i;
                ui->currentProcessLabel->setText("Current Process: P" + QString::number(processes[i].id));
                break;
            }
        }
    }
    
    // Process execution
    if (currentProcessIndex != -1) {
        Process& currentProcess = processes[currentProcessIndex];
        
        // Execute one time unit
        currentProcess.remainingTime--;
        
        // Check if process is completed
        if (currentProcess.remainingTime == 0) {
            currentProcess.isCompleted = true;
            currentProcess.completionTime = currentTime;
            currentProcess.turnaroundTime = currentProcess.completionTime - currentProcess.arrivalTime;
            currentProcess.waitingTime = currentProcess.turnaroundTime - currentProcess.burstTime;
            
            // Find the next process
            currentProcessIndex = -1;
            for (int i = 0; i < processes.size(); i++) {
                if (!processes[i].isCompleted && processes[i].arrivalTime <= currentTime) {
                    currentProcessIndex = i;
                    ui->currentProcessLabel->setText("Current Process: P" + QString::number(processes[i].id));
                    break;
                }
            }
            
            if (currentProcessIndex == -1) {
                ui->currentProcessLabel->setText("Current Process: None");
            }
        }
    } else {
        ui->currentProcessLabel->setText("Current Process: None (CPU Idle)");
    }
    
    // Update the process table
    updateProcessTable();
    
    // Draw Gantt chart
    drawGanttChart();
    
    // Check if all processes are completed
    bool allCompleted = true;
    for (const Process& p : processes) {
        if (!p.isCompleted) {
            allCompleted = false;
            break;
        }
    }
    
    if (allCompleted) {
        simulationComplete = true;
        calculateMetrics();
        drawMetricsChart();
    }
}

void MainWindow::calculateMetrics()
{
    float totalTurnaround = 0.0f;
    float totalWaiting = 0.0f;
    
    for (const Process& p : processes) {
        totalTurnaround += p.turnaroundTime;
        totalWaiting += p.waitingTime;
    }
    
    float avgTurnaround = totalTurnaround / processes.size();
    float avgWaiting = totalWaiting / processes.size();
    
    ui->avgTurnaroundLabel->setText("Average Turnaround Time: " + QString::number(avgTurnaround, 'f', 2));
    ui->avgWaitingLabel->setText("Average Waiting Time: " + QString::number(avgWaiting, 'f', 2));
}

void MainWindow::drawGanttChart()
{
    ganttChartScene->clear();
    
    int rectWidth = 30;
    int rectHeight = 40;
    int xOffset = 10;
    int yOffset = 20;
    int textOffset = 10;
    
    // Draw time axis
    ganttChartScene->addLine(xOffset, yOffset + rectHeight + 10, xOffset + currentTime * rectWidth, yOffset + rectHeight + 10);
    
    // Draw time markers
    for (int t = 0; t <= currentTime; t++) {
        int x = xOffset + t * rectWidth;
        ganttChartScene->addLine(x, yOffset + rectHeight + 5, x, yOffset + rectHeight + 15);
        QGraphicsTextItem* timeText = ganttChartScene->addText(QString::number(t));
        timeText->setPos(x - timeText->boundingRect().width() / 2, yOffset + rectHeight + 15);
    }
    
    // Track the current process for each time unit
    QMap<int, int> timeToProcessMap;
    for (int t = 0; t <= currentTime; t++) {
        timeToProcessMap[t] = -1; // -1 means idle
    }
    
    // Fill the map based on process execution
    int lastCompletedTime = 0;
    
    for (int i = 0; i < processes.size(); i++) {
        const Process& p = processes[i];
        if (p.isCompleted) {
            int startTime = p.completionTime - p.burstTime;
            for (int t = startTime; t < p.completionTime; t++) {
                if (t >= 0) {
                    timeToProcessMap[t] = i;
                }
            }
            lastCompletedTime = std::max(lastCompletedTime, p.completionTime);
        }
    }
    
    // For the current executing process
    if (currentProcessIndex != -1) {
        const Process& currentP = processes[currentProcessIndex];
        int startTime = currentTime - (currentP.burstTime - currentP.remainingTime);
        for (int t = startTime; t < currentTime; t++) {
            if (t >= 0) {
                timeToProcessMap[t] = currentProcessIndex;
            }
        }
    }
    
    // Draw the Gantt chart based on the map
    int lastProcessIndex = -2; // Different from idle (-1)
    int startX = xOffset;
    
    for (int t = 0; t <= currentTime; t++) {
        int processIndex = timeToProcessMap[t];
        
        if (processIndex != lastProcessIndex || t == currentTime) {
            // Draw the previous block if there's a change or we reached the end
            if (lastProcessIndex != -2 && t > 0) {
                int width = (xOffset + t * rectWidth) - startX;
                
                // Draw the rectangle
                QGraphicsRectItem* rect;
                if (lastProcessIndex == -1) {
                    // Idle time
                    rect = ganttChartScene->addRect(startX, yOffset, width, rectHeight, QPen(Qt::black), QBrush(Qt::lightGray));
                    QGraphicsTextItem* idleText = ganttChartScene->addText("Idle");
                    idleText->setPos(startX + width/2 - idleText->boundingRect().width()/2, 
                                   yOffset + rectHeight/2 - idleText->boundingRect().height()/2);
                } else {
                    // Process execution
                    const Process& p = processes[lastProcessIndex];
                    rect = ganttChartScene->addRect(startX, yOffset, width, rectHeight, QPen(Qt::black), QBrush(p.color));
                    QGraphicsTextItem* processText = ganttChartScene->addText("P" + QString::number(p.id));
                    processText->setPos(startX + width/2 - processText->boundingRect().width()/2, 
                                      yOffset + rectHeight/2 - processText->boundingRect().height()/2);
                }
            }
            
            // Start a new block
            startX = xOffset + t * rectWidth;
            lastProcessIndex = processIndex;
        }
    }
    
    ui->ganttChartView->fitInView(ganttChartScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    ui->ganttChartView->centerOn(ganttChartScene->itemsBoundingRect().center());
}

void MainWindow::drawMetricsChart()
{
    if (!simulationComplete) return;
    
    metricsScene->clear();
    
    int barWidth = 40;
    int maxHeight = 200;
    int xOffset = 50;
    int yOffset = 250;
    int spacing = 80;
    
    // Find maximum values for scaling
    int maxTurnaround = 0;
    int maxWaiting = 0;
    int maxBurst = 0;
    
    for (const Process& p : processes) {
        maxTurnaround = std::max(maxTurnaround, p.turnaroundTime);
        maxWaiting = std::max(maxWaiting, p.waitingTime);
        maxBurst = std::max(maxBurst, p.burstTime);
    }
    
    int maxMetric = std::max({maxTurnaround, maxWaiting, maxBurst});
    
    // Draw axes
    metricsScene->addLine(xOffset, yOffset, xOffset + (processes.size() * spacing) + 50, yOffset); // X-axis
    metricsScene->addLine(xOffset, yOffset, xOffset, yOffset - maxHeight - 50); // Y-axis
    
    // Add axis labels
    QGraphicsTextItem* yLabel = metricsScene->addText("Time");
    yLabel->setPos(xOffset - 40, yOffset - maxHeight - 60);
    
    QGraphicsTextItem* xLabel = metricsScene->addText("Process ID");
    xLabel->setPos(xOffset + (processes.size() * spacing) / 2 - 30, yOffset + 20);
    
    // Draw scale on Y-axis
    int scaleStep = maxMetric > 10 ? maxMetric / 10 : 1;
    for (int i = 0; i <= maxMetric; i += scaleStep) {
        int y = yOffset - (i * maxHeight / maxMetric);
        metricsScene->addLine(xOffset - 5, y, xOffset, y);
        QGraphicsTextItem* scaleText = metricsScene->addText(QString::number(i));
        scaleText->setPos(xOffset - 25, y - 10);
    }
    
    // Draw legend
    int legendX = xOffset + (processes.size() * spacing) + 60;
    int legendY = yOffset - maxHeight;
    
    QGraphicsRectItem* burstRect = metricsScene->addRect(legendX, legendY, 20, 20, QPen(Qt::black), QBrush(Qt::blue));
    QGraphicsTextItem* burstText = metricsScene->addText("Burst Time");
    burstText->setPos(legendX + 30, legendY);
    
    QGraphicsRectItem* waitRect = metricsScene->addRect(legendX, legendY + 30, 20, 20, QPen(Qt::black), QBrush(Qt::green));
    QGraphicsTextItem* waitText = metricsScene->addText("Waiting Time");
    waitText->setPos(legendX + 30, legendY + 30);
    
    QGraphicsRectItem* turnRect = metricsScene->addRect(legendX, legendY + 60, 20, 20, QPen(Qt::black), QBrush(Qt::red));
    QGraphicsTextItem* turnText = metricsScene->addText("Turnaround Time");
    turnText->setPos(legendX + 30, legendY + 60);
    
    // Draw bars for each process
    for (int i = 0; i < processes.size(); i++) {
        const Process& p = processes[i];
        int x = xOffset + i * spacing;
        
        // Draw burst time bar
        int burstHeight = p.burstTime * maxHeight / maxMetric;
        metricsScene->addRect(x, yOffset - burstHeight, barWidth, burstHeight, QPen(Qt::black), QBrush(Qt::blue));
        
        // Draw waiting time bar
        int waitingHeight = p.waitingTime * maxHeight / maxMetric;
        metricsScene->addRect(x + barWidth, yOffset - waitingHeight, barWidth, waitingHeight, QPen(Qt::black), QBrush(Qt::green));
        
        // Draw turnaround time bar
        int turnaroundHeight = p.turnaroundTime * maxHeight / maxMetric;
        metricsScene->addRect(x + 2 * barWidth, yOffset - turnaroundHeight, barWidth, turnaroundHeight, QPen(Qt::black), QBrush(Qt::red));
        
        // Add process ID label
        QGraphicsTextItem* idText = metricsScene->addText("P" + QString::number(p.id));
        idText->setPos(x + barWidth, yOffset + 5);
    }
    
    ui->metricsView->fitInView(metricsScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    ui->metricsView->centerOn(metricsScene->itemsBoundingRect().center());
}

QColor MainWindow::getRandomColor()
{
    return QColor(
        QRandomGenerator::global()->bounded(100, 240),  // Red
        QRandomGenerator::global()->bounded(100, 240),  // Green
        QRandomGenerator::global()->bounded(100, 240)   // Blue
    );
}

void MainWindow::sortProcessesByArrivalTime()
{
    std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });
    
    updateProcessTable();
}