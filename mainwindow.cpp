#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QValueAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QGridLayout>
#include <QHeaderView>

// Using namespace to simplify code
using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentTime(0)
    , currentProcessIndex(-1)
    , simulationRunning(false)
    , simulationSpeed(500)
    , isFirstRun(true)
{
    ui->setupUi(this);
    
    // Initialize the process table
    ui->processTableWidget->setColumnCount(9);
    ui->processTableWidget->setHorizontalHeaderLabels({
        "ID", "Arrival Time", "Burst Time", "Completion Time", 
        "Turnaround Time", "Waiting Time", "Response Time", "Status", "Progress"
    });
    
    // Adjust column widths
    ui->processTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Initialize the simulation timer
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    
    // Initialize the speed slider
    ui->speedSlider->setMinimum(50);
    ui->speedSlider->setMaximum(1000);
    ui->speedSlider->setValue(simulationSpeed);
    ui->speedSlider->setInvertedAppearance(true);
    
    // Initialize the Gantt chart view
    ui->ganttChartView->setScene(new QGraphicsScene(this));
    ui->ganttChartView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->ganttChartView->setRenderHint(QPainter::Antialiasing);
    
    // Setup additional UI components
    setupAdditionalUi();
    
    // Setup charts
    setupCharts();
    
    // Disable start button until processes are added
    ui->startSimulationButton->setEnabled(false);
    ui->exportButton->setEnabled(false);
    ui->removeProcessButton->setEnabled(false);
    
    // Set window title and size
    this->setWindowTitle("Advanced FCFS Scheduler Simulation");
    this->resize(1200, 800);
}

void MainWindow::setupAdditionalUi()
{
    // Add tooltips
    ui->processIdSpinBox->setToolTip("Unique identifier for the process");
    ui->arrivalTimeSpinBox->setToolTip("Time at which the process arrives in the ready queue");
    ui->burstTimeSpinBox->setToolTip("CPU time required by the process to complete execution");
    ui->speedSlider->setToolTip("Adjust simulation speed");
    
    // Add CPU utilization progress bar
    ui->cpuUtilizationBar->setMinimum(0);
    ui->cpuUtilizationBar->setMaximum(100);
    ui->cpuUtilizationBar->setValue(0);
    ui->cpuUtilizationBar->setFormat("CPU Utilization: %p%");
    ui->cpuUtilizationBar->setTextVisible(true);
    
    // Connect process table selection
    connect(ui->processTableWidget, &QTableWidget::cellClicked, this, &MainWindow::on_processTableWidget_cellClicked);
    
    // Add priority to process details (for future extension)
    ui->prioritySpinBox->setMinimum(1);
    ui->prioritySpinBox->setMaximum(10);
    ui->prioritySpinBox->setValue(5);
}

void MainWindow::setupCharts()
{
    // Initialize chart views
    cpuUtilizationChartView = new QChartView(ui->cpuUtilizationTab);
    cpuUtilizationChartView->setRenderHint(QPainter::Antialiasing);
    
    waitingTimeChartView = new QChartView(ui->waitingTimeTab);
    waitingTimeChartView->setRenderHint(QPainter::Antialiasing);
    
    turnaroundTimeChartView = new QChartView(ui->turnaroundTimeTab);
    turnaroundTimeChartView->setRenderHint(QPainter::Antialiasing);
    
    responseTimeChartView = new QChartView(ui->responseTimeTab);
    responseTimeChartView->setRenderHint(QPainter::Antialiasing);
    
    processComparisonChartView = new QChartView(ui->processComparisonTab);
    processComparisonChartView->setRenderHint(QPainter::Antialiasing);
    
    throughputChartView = new QChartView(ui->throughputTab);
    throughputChartView->setRenderHint(QPainter::Antialiasing);
    
    pieChartView = new QChartView(ui->pieChartTab);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    
    // Add charts to layouts
    QVBoxLayout *cpuUtilizationLayout = new QVBoxLayout(ui->cpuUtilizationTab);
    cpuUtilizationLayout->addWidget(cpuUtilizationChartView);
    
    QVBoxLayout *waitingTimeLayout = new QVBoxLayout(ui->waitingTimeTab);
    waitingTimeLayout->addWidget(waitingTimeChartView);
    
    QVBoxLayout *turnaroundTimeLayout = new QVBoxLayout(ui->turnaroundTimeTab);
    turnaroundTimeLayout->addWidget(turnaroundTimeChartView);
    
    QVBoxLayout *responseTimeLayout = new QVBoxLayout(ui->responseTimeTab);
    responseTimeLayout->addWidget(responseTimeChartView);
    
    QVBoxLayout *processComparisonLayout = new QVBoxLayout(ui->processComparisonTab);
    processComparisonLayout->addWidget(processComparisonChartView);
    
    QVBoxLayout *throughputLayout = new QVBoxLayout(ui->throughputTab);
    throughputLayout->addWidget(throughputChartView);
    
    QVBoxLayout *pieChartLayout = new QVBoxLayout(ui->pieChartTab);
    pieChartLayout->addWidget(pieChartView);
    
    // Initialize empty charts
    QChart *cpuUtilizationChart = new QChart();
    cpuUtilizationChart->setTitle("CPU Utilization Over Time");
    cpuUtilizationChart->legend()->setVisible(true);
    cpuUtilizationChart->legend()->setAlignment(Qt::AlignBottom);
    cpuUtilizationChartView->setChart(cpuUtilizationChart);
    
    QChart *waitingTimeChart = new QChart();
    waitingTimeChart->setTitle("Average Waiting Time Over Time");
    waitingTimeChart->legend()->setVisible(true);
    waitingTimeChart->legend()->setAlignment(Qt::AlignBottom);
    waitingTimeChartView->setChart(waitingTimeChart);
    
    QChart *turnaroundTimeChart = new QChart();
    turnaroundTimeChart->setTitle("Average Turnaround Time Over Time");
    turnaroundTimeChart->legend()->setVisible(true);
    turnaroundTimeChart->legend()->setAlignment(Qt::AlignBottom);
    turnaroundTimeChartView->setChart(turnaroundTimeChart);
    
    QChart *responseTimeChart = new QChart();
    responseTimeChart->setTitle("Average Response Time Over Time");
    responseTimeChart->legend()->setVisible(true);
    responseTimeChart->legend()->setAlignment(Qt::AlignBottom);
    responseTimeChartView->setChart(responseTimeChart);
    
    QChart *processComparisonChart = new QChart();
    processComparisonChart->setTitle("Process Performance Comparison");
    processComparisonChart->legend()->setVisible(true);
    processComparisonChart->legend()->setAlignment(Qt::AlignBottom);
    processComparisonChartView->setChart(processComparisonChart);
    
    QChart *throughputChart = new QChart();
    throughputChart->setTitle("Throughput Over Time");
    throughputChart->legend()->setVisible(true);
    throughputChart->legend()->setAlignment(Qt::AlignBottom);
    throughputChartView->setChart(throughputChart);
    
    QChart *pieChart = new QChart();
    pieChart->setTitle("CPU Time Distribution");
    pieChart->legend()->setVisible(true);
    pieChart->legend()->setAlignment(Qt::AlignRight);
    pieChartView->setChart(pieChart);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_addProcessButton_clicked()
{
    // Get process details from input fields
    int id = ui->processIdSpinBox->value();
    int arrivalTime = ui->arrivalTimeSpinBox->value();
    int burstTime = ui->burstTimeSpinBox->value();
    
    // Validate input
    if (burstTime <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Burst time must be greater than zero!");
        return;
    }
    
    // Check for duplicate process ID
    for (const Process& p : processes) {
        if (p.id == id) {
            QMessageBox::warning(this, "Invalid Input", "Process ID already exists!");
            return;
        }
    }
    
    // Create and add the new process
    Process newProcess;
    newProcess.id = id;
    newProcess.arrivalTime = arrivalTime;
    newProcess.burstTime = burstTime;
    newProcess.remainingTime = burstTime;
    newProcess.completionTime = 0;
    newProcess.turnaroundTime = 0;
    newProcess.waitingTime = 0;
    newProcess.responseTime = -1;  // -1 means not yet started
    newProcess.isArrived = false;
    newProcess.isCompleted = false;
    newProcess.hasStarted = false;
    generateRandomColor(newProcess);
    
    processes.append(newProcess);
    
    // Update the process table
    updateProcessTable();
    
    // Increment the process ID for the next process
    ui->processIdSpinBox->setValue(id + 1);
    
    // Enable buttons
    ui->startSimulationButton->setEnabled(true);
    ui->clearAllButton->setEnabled(true);
    
    // Update charts for the new process set
    if (!simulationRunning) {
        updateProcessComparisonChart();
        updatePieChart();
    }
}

void MainWindow::generateRandomColor(Process &process) {
    // Generate a random color with good contrast
    process.color = QColor(
        QRandomGenerator::global()->bounded(100, 240),
        QRandomGenerator::global()->bounded(100, 240),
        QRandomGenerator::global()->bounded(100, 240)
    );
}

QColor MainWindow::getRandomColor() {
    return QColor(
        QRandomGenerator::global()->bounded(100, 240),
        QRandomGenerator::global()->bounded(100, 240),
        QRandomGenerator::global()->bounded(100, 240)
    );
}

void MainWindow::updateProcessTable()
{
    ui->processTableWidget->setRowCount(processes.size());
    
    for (int i = 0; i < processes.size(); ++i) {
        const Process& p = processes[i];
        
        // Set process details in the table
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(p.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        idItem->setBackground(p.color);
        ui->processTableWidget->setItem(i, 0, idItem);
        
        QTableWidgetItem* arrivalItem = new QTableWidgetItem(QString::number(p.arrivalTime));
        arrivalItem->setTextAlignment(Qt::AlignCenter);
        ui->processTableWidget->setItem(i, 1, arrivalItem);
        
        QTableWidgetItem* burstItem = new QTableWidgetItem(QString::number(p.burstTime));
        burstItem->setTextAlignment(Qt::AlignCenter);
        ui->processTableWidget->setItem(i, 2, burstItem);
        
        // Completion Time
        QTableWidgetItem* completionItem;
        if (p.isCompleted) {
            completionItem = new QTableWidgetItem(QString::number(p.completionTime));
        } else {
            completionItem = new QTableWidgetItem("-");
        }
        completionItem->setTextAlignment(Qt::AlignCenter);
        ui->processTableWidget->setItem(i, 3, completionItem);
        
        // Turnaround Time
        QTableWidgetItem* turnaroundItem;
        if (p.isCompleted) {
            turnaroundItem = new QTableWidgetItem(QString::number(p.turnaroundTime));
        } else {
            turnaroundItem = new QTableWidgetItem("-");
        }
        turnaroundItem->setTextAlignment(Qt::AlignCenter);
        ui->processTableWidget->setItem(i, 4, turnaroundItem);
        
        // Waiting Time
        QTableWidgetItem* waitingItem;
        if (p.isCompleted) {
            waitingItem = new QTableWidgetItem(QString::number(p.waitingTime));
        } else if (p.hasStarted) {
            int currentWaiting = currentTime - p.arrivalTime - (p.burstTime - p.remainingTime);
            waitingItem = new QTableWidgetItem(QString::number(currentWaiting));
        } else {
            waitingItem = new QTableWidgetItem("-");
        }
        waitingItem->setTextAlignment(Qt::AlignCenter);
        ui->processTableWidget->setItem(i, 5, waitingItem);
        
        // Response Time
        QTableWidgetItem* responseItem;
        if (p.responseTime != -1) {
            responseItem = new QTableWidgetItem(QString::number(p.responseTime));
        } else {
            responseItem = new QTableWidgetItem("-");
        }
        responseItem->setTextAlignment(Qt::AlignCenter);
        ui->processTableWidget->setItem(i, 6, responseItem);
        
        // Status
        QTableWidgetItem* statusItem;
        if (p.isCompleted) {
            statusItem = new QTableWidgetItem("Completed");
            statusItem->setForeground(Qt::darkGreen);
        } else if (i == currentProcessIndex) {
            statusItem = new QTableWidgetItem("Running");
            statusItem->setForeground(Qt::blue);
        } else if (p.isArrived) {
            statusItem = new QTableWidgetItem("Ready");
            statusItem->setForeground(Qt::darkYellow);
        } else {
            statusItem = new QTableWidgetItem("Not Arrived");
            statusItem->setForeground(Qt::gray);
        }
        statusItem->setTextAlignment(Qt::AlignCenter);
        ui->processTableWidget->setItem(i, 7, statusItem);
        
        // Progress bar
        int progress = 0;
        if (p.isCompleted) {
            progress = 100;
        } else if (p.hasStarted) {
            progress = (int)(((double)(p.burstTime - p.remainingTime) / p.burstTime) * 100);
        }
        
        QProgressBar* progressBar = new QProgressBar();
        progressBar->setMinimum(0);
        progressBar->setMaximum(100);
        progressBar->setValue(progress);
        progressBar->setTextVisible(true);
        progressBar->setFormat("%p%");
        ui->processTableWidget->setCellWidget(i, 8, progressBar);
    }
    
    // Highlight the current running process
    highlightCurrentProcess();
}

void MainWindow::highlightCurrentProcess()
{
    // Reset background color for status column
    for (int i = 0; i < ui->processTableWidget->rowCount(); ++i) {
        for (int j = 1; j < ui->processTableWidget->columnCount(); ++j) {
            if (j != 8) { // Skip progress bar column
                QTableWidgetItem* item = ui->processTableWidget->item(i, j);
                if (item) {
                    item->setBackground(Qt::white);
                }
            }
        }
    }
    
    // Highlight the currently running process
    if (currentProcessIndex >= 0 && currentProcessIndex < processes.size()) {
        for (int j = 1; j < ui->processTableWidget->columnCount(); ++j) {
            if (j != 8) { // Skip progress bar column
                QTableWidgetItem* item = ui->processTableWidget->item(currentProcessIndex, j);
                if (item) {
                    item->setBackground(QColor(220, 240, 255)); // Light blue background
                }
            }
        }
        
        // Update the status to "Running"
        QTableWidgetItem* statusItem = ui->processTableWidget->item(currentProcessIndex, 7);
        if (statusItem) {
            statusItem->setText("Running");
            statusItem->setForeground(Qt::blue);
        }
    }
}

void MainWindow::on_startSimulationButton_clicked()
{
    if (processes.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Add at least one process before starting simulation!");
        return;
    }
    
    if (!simulationRunning) {
        // First run or after reset
        if (isFirstRun) {
            // Clear any previous simulation
            resetSimulation();
            
            // Sort processes by arrival time
            std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
                return a.arrivalTime < b.arrivalTime;
            });
            
            // Update the process table after sorting
            updateProcessTable();
            
            isFirstRun = false;
        }
        
        // Start the simulation
        simulationRunning = true;
        ui->startSimulationButton->setText("Pause");
        ui->startSimulationButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        ui->addProcessButton->setEnabled(false);
        ui->importButton->setEnabled(false);
        ui->clearAllButton->setEnabled(false);
        ui->resetButton->setEnabled(true);
        
        // Start the timer
        simulationTimer->start(simulationSpeed);
    } else {
        // Pause the simulation
        simulationRunning = false;
        ui->startSimulationButton->setText("Resume");
        ui->startSimulationButton->setIcon(QIcon::fromTheme("media-playback-start"));
        simulationTimer->stop();
    }
}

void MainWindow::on_resetButton_clicked()
{
    // Stop the simulation
    simulationTimer->stop();
    simulationRunning = false;
    
    // Reset all processes and simulation state
    resetSimulation();
    
    // Update UI
    ui->startSimulationButton->setText("Start");
    ui->startSimulationButton->setIcon(QIcon::fromTheme("media-playback-start"));
    ui->startSimulationButton->setEnabled(!processes.isEmpty());
    ui->addProcessButton->setEnabled(true);
    ui->importButton->setEnabled(true);
    ui->clearAllButton->setEnabled(!processes.isEmpty());
    ui->resetButton->setEnabled(false);
    ui->currentTimeLabel->setText("Current Time: 0");
    ui->ganttChartView->scene()->clear();
    ui->cpuUtilizationBar->setValue(0);
    
    // Reset statistics
    ui->avgTurnaroundTimeLabel->setText("Average Turnaround Time: 0.00");
    ui->avgWaitingTimeLabel->setText("Average Waiting Time: 0.00");
    ui->avgResponseTimeLabel->setText("Average Response Time: 0.00");
    ui->throughputLabel->setText("Throughput: 0.00 processes/unit time");
    
    // Reset charts
    timeHistory.clear();
    cpuUtilizationHistory.clear();
    avgWaitingTimeHistory.clear();
    avgTurnaroundTimeHistory.clear();
    avgResponseTimeHistory.clear();
    completedProcessesHistory.clear();
    
    // Reset process timings
    processTimings.clear();
    
    // Reset and redraw charts
    updatePerformanceCharts();
    updateProcessComparisonChart();
    updatePieChart();
    updateThroughputChart();
    
    isFirstRun = true;
}

void MainWindow::resetSimulation()
{
    // Reset simulation state
    currentTime = 0;
    currentProcessIndex = -1;
    readyQueue.clear();
    
    // Reset all processes
    for (Process& p : processes) {
        p.remainingTime = p.burstTime;
        p.completionTime = 0;
        p.turnaroundTime = 0;
        p.waitingTime = 0;
        p.responseTime = -1;
        p.isArrived = false;
        p.isCompleted = false;
        p.hasStarted = false;
    }
    
    updateProcessTable();
}

void MainWindow::updateSimulation()
{
    // Check for new process arrivals
    for (int i = 0; i < processes.size(); ++i) {
        if (!processes[i].isArrived && processes[i].arrivalTime <= currentTime) {
            processes[i].isArrived = true;
            readyQueue.enqueue(i);
        }
    }
    
    // If no process is currently running, get the next process from the ready queue
    if (currentProcessIndex == -1 && !readyQueue.isEmpty()) {
        currentProcessIndex = readyQueue.dequeue();
        
        // Record response time if this is the first time process is running
        if (!processes[currentProcessIndex].hasStarted) {
            processes[currentProcessIndex].hasStarted = true;
            processes[currentProcessIndex].responseTime = currentTime - processes[currentProcessIndex].arrivalTime;
        }
        
        highlightCurrentProcess();
    }
    
    // If a process is running, update its remaining time
    if (currentProcessIndex != -1) {
        Process& currentProcess = processes[currentProcessIndex];
        currentProcess.remainingTime--;
        
        // Record the process execution for the Gantt chart
        if (!processTimings.contains(currentProcess.id)) {
            processTimings[currentProcess.id] = QVector<QPair<int, int>>();
            processTimings[currentProcess.id].append(QPair<int, int>(currentTime, currentTime + 1));
        } else {
            QVector<QPair<int, int>>& times = processTimings[currentProcess.id];
            if (!times.isEmpty() && times.last().second == currentTime) {
                times.last().second = currentTime + 1;
            } else {
                times.append(QPair<int, int>(currentTime, currentTime + 1));
            }
        }
        
        // If the process completes
        if (currentProcess.remainingTime == 0) {
            currentProcess.isCompleted = true;
            currentProcess.completionTime = currentTime + 1;
            currentProcess.turnaroundTime = currentProcess.completionTime - currentProcess.arrivalTime;
            currentProcess.waitingTime = currentProcess.turnaroundTime - currentProcess.burstTime;
            
            // Update Gantt chart
            updateGanttChart();
            
            // Get the next process from the ready queue
            if (!readyQueue.isEmpty()) {
                currentProcessIndex = readyQueue.dequeue();
                
                // Record response time if this is the first time process is running
                if (!processes[currentProcessIndex].hasStarted) {
                    processes[currentProcessIndex].hasStarted = true;
                    processes[currentProcessIndex].responseTime = currentTime + 1 - processes[currentProcessIndex].arrivalTime;
                }
            } else {
                currentProcessIndex = -1;
            }
            
            // Calculate statistics
            calculateStatistics();
        }
    }
    
    // Update current time
    currentTime++;
    ui->currentTimeLabel->setText("Current Time: " + QString::number(currentTime));
    
    // Record history for charts
    timeHistory.append(currentTime);
    cpuUtilizationHistory.append(calculateCpuUtilization());
    
    // Calculate and update statistics periodically
    if (currentTime % 5 == 0 || currentProcessIndex == -1) {
        calculateStatistics();
        updateStatistics();
        updatePerformanceCharts();
        updateThroughputChart();
    }
    
    // Update simulation progress
    updateSimulationProgress();
    
    // Update the process table
    updateProcessTable();
    
    // Check if all processes are completed
    bool allCompleted = true;
    for (const Process& p : processes) {
        if (!p.isCompleted) {
            allCompleted = false;
            break;
        }
    }
    
    if (allCompleted) {
        simulationTimer->stop();
        simulationRunning = false;
        ui->startSimulationButton->setText("Start");
        ui->startSimulationButton->setIcon(QIcon::fromTheme("media-playback-start"));
        ui->startSimulationButton->setEnabled(false);
        ui->exportButton->setEnabled(true);
        
        // Final update of all charts and statistics
        calculateStatistics();
        updateStatistics();
        updatePerformanceCharts();
        updateProcessComparisonChart();
        updatePieChart();
        updateThroughputChart();
        
        QMessageBox::information(this, "Simulation Complete", "All processes have been completed!");
    }
}

double MainWindow::calculateCpuUtilization() {
    int busyTime = 0;
    
    // Count time units where CPU was busy
    for (auto it = processTimings.constBegin(); it != processTimings.constEnd(); ++it) {
        const QVector<QPair<int, int>>& timePairs = it.value();
        for (const auto& pair : timePairs) {
            busyTime += pair.second - pair.first;
        }
    }
    
    // Calculate utilization as a percentage
    return (currentTime > 0) ? ((double)busyTime / currentTime) * 100.0 : 0.0;
}

void MainWindow::updateSimulationProgress() {
    // Update CPU utilization
    double cpuUtilization = calculateCpuUtilization();
    ui->cpuUtilizationBar->setValue(static_cast<int>(cpuUtilization));
    
    // Update progress label
    int completedProcesses = 0;
    for (const Process& p : processes) {
        if (p.isCompleted) {
            completedProcesses++;
        }
    }
    
    ui->progressLabel->setText(QString("Progress: %1/%2 processes completed").arg(completedProcesses).arg(processes.size()));
}

void MainWindow::updateGanttChart()
{
    // Clear the scene
    QGraphicsScene* scene = ui->ganttChartView->scene();
    scene->clear();
    
    const int height = 40;
    const int timeScale = 30; // Pixels per time unit
    
    int maxWidth = 0;
    int verticalPosition = 0;
    
    // Create a map to store process IDs to y-positions for multi-row Gantt chart
    QMap<int, int> processYPositions;
    int nextYPosition = 0;
    
    // Create time grid
    const int gridColor = 230;
    QPen gridPen(QColor(gridColor, gridColor, gridColor), 1, Qt::DashLine);
    for (int t = 0; t <= currentTime; t += 5) {
        scene->addLine(t * timeScale, 0, t * timeScale, height * processes.size(), gridPen);
    }
    
    // Draw process execution rectangles
    QMap<int, QVector<QPair<int, int>>>::const_iterator it;
    for (it = processTimings.constBegin(); it != processTimings.constEnd(); ++it) {
        int processId = it.key();
        const QVector<QPair<int, int>>& times = it.value();
        
        // Find the process index and color
        int processIndex = -1;
        QColor processColor;
        for (int i = 0; i < processes.size(); ++i) {
            if (processes[i].id == processId) {
                processIndex = i;
                processColor = processes[i].color;
                break;
            }
        }
        
        // Assign y-position for this process
        if (!processYPositions.contains(processId)) {
            processYPositions[processId] = nextYPosition;
            nextYPosition += height;
        }
        int yPos = processYPositions[processId];
        
        // Draw each execution segment
        for (const QPair<int, int>& time : times) {
            int startTime = time.first;
            int endTime = time.second;
            int rectWidth = (endTime - startTime) * timeScale;
            
            // Create a rectangle for this process duration
            QGraphicsRectItem* rect = new QGraphicsRectItem(startTime * timeScale, yPos, rectWidth, height);
            rect->setBrush(processColor);
            rect->setPen(QPen(Qt::black));
            
            // Add tooltip to show process details
            QString tooltip = QString("Process ID: %1\nStart Time: %2\nEnd Time: %3\nDuration: %4")
                .arg(processId)
                .arg(startTime)
                .arg(endTime)
                .arg(endTime - startTime);
            rect->setToolTip(tooltip);
            
            scene->addItem(rect);
            
            // Add process ID text
            QGraphicsTextItem* text = new QGraphicsTextItem("P" + QString::number(processId));
            text->setPos(startTime * timeScale + rectWidth/2 - 10, yPos + height/4);
            scene->addItem(text);
            
            // Add time labels
            if (startTime % 5 == 0) {
                QGraphicsTextItem* startLabel = new QGraphicsTextItem(QString::number(startTime));
                startLabel->setPos(startTime * timeScale - 5, nextYPosition + 5);
                scene->addItem(startLabel);
            }
            
            if (endTime * timeScale > maxWidth) {
                maxWidth = endTime * timeScale;
            }
        }
    }
    
    // Add final time label
    QGraphicsTextItem* endLabel = new QGraphicsTextItem(QString::number(currentTime));
    endLabel->setPos(currentTime * timeScale - 5, nextYPosition + 5);
    scene->addItem(endLabel);
    
    // Add time axis
    scene->addLine(0, nextYPosition, maxWidth + 30, nextYPosition, QPen(Qt::black, 2));
    
    // Add process labels on the left
    for (auto it = processYPositions.constBegin(); it != processYPositions.constEnd(); ++it) {
        QGraphicsTextItem* label = new QGraphicsTextItem("P" + QString::number(it.key()));
        label->setPos(-25, it.value() + height/4);
        scene->addItem(label);
    }
    
    // Adjust the scene rectangle
    scene->setSceneRect(-30, 0, maxWidth + 50, nextYPosition + 30);
    
    // Adjust view to show the entire Gantt chart
    ui->ganttChartView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::calculateStatistics()
{
    int completedProcesses = 0;
    double totalTurnaroundTime = 0;
    double totalWaitingTime = 0;
    double totalResponseTime = 0;
    
    for (const Process& p : processes) {
        if (p.isCompleted) {
            completedProcesses++;
            totalTurnaroundTime += p.turnaroundTime;
            totalWaitingTime += p.waitingTime;
            totalResponseTime += p.responseTime;
        }
    }
    
    if (completedProcesses > 0) {
        double avgTurnaroundTime = totalTurnaroundTime / completedProcesses;
        double avgWaitingTime = totalWaitingTime / completedProcesses;
        double avgResponseTime = totalResponseTime / completedProcesses;
        double throughput = (currentTime > 0) ? ((double)completedProcesses / currentTime) : 0;
        
        ui->avgTurnaroundTimeLabel->setText("Average Turnaround Time: " + QString::number(avgTurnaroundTime, 'f', 2));
        ui->avgWaitingTimeLabel->setText("Average Waiting Time: " + QString::number(avgWaitingTime, 'f', 2));
        ui->avgResponseTimeLabel->setText("Average Response Time: " + QString::number(avgResponseTime, 'f', 2));
        ui->throughputLabel->setText("Throughput: " + QString::number(throughput, 'f', 2) + " processes/unit time");
        
        // Update statistics history
        if (completedProcesses > completedProcessesHistory.size()) {
            avgWaitingTimeHistory.append(avgWaitingTime);
            avgTurnaroundTimeHistory.append(avgTurnaroundTime);
            avgResponseTimeHistory.append(avgResponseTime);
            completedProcessesHistory.append(completedProcesses);
        }
    }
}

void MainWindow::updateStatistics()
{
    // This function is called to update the statistics display
    // The actual calculation is done in calculateStatistics()
    // This is a placeholder for any additional statistics updates
}

void MainWindow::updatePerformanceCharts()
{
    // Update CPU Utilization Chart
    QChart* cpuChart = cpuUtilizationChartView->chart();
    cpuChart->removeAllSeries();
    
    QLineSeries* cpuSeries = new QLineSeries();
    cpuSeries->setName("CPU Utilization %");
    
    for (int i = 0; i < timeHistory.size(); ++i) {
        cpuSeries->append(timeHistory[i], cpuUtilizationHistory[i]);
    }
    
    cpuChart->addSeries(cpuSeries);
    
    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Time");
    axisX->setLabelFormat("%d");
    axisX->setTickCount(10);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Utilization %");
    axisY->setLabelFormat("%.1f");
    axisY->setRange(0, 100);
    
    cpuChart->setAxisX(axisX, cpuSeries);
    cpuChart->setAxisY(axisY, cpuSeries);
    
    // Update Waiting Time Chart
    QChart* waitingChart = waitingTimeChartView->chart();
    waitingChart->removeAllSeries();
    
    QLineSeries* waitingSeries = new QLineSeries();
    waitingSeries->setName("Average Waiting Time");
    
    for (int i = 0; i < completedProcessesHistory.size(); ++i) {
        waitingSeries->append(completedProcessesHistory[i], avgWaitingTimeHistory[i]);
    }
    
    waitingChart->addSeries(waitingSeries);
    
    QValueAxis* waitingAxisX = new QValueAxis();
    waitingAxisX->setTitleText("Completed Processes");
    waitingAxisX->setLabelFormat("%d");
    
    QValueAxis* waitingAxisY = new QValueAxis();
    waitingAxisY->setTitleText("Time Units");
    waitingAxisY->setLabelFormat("%.1f");
    
    waitingChart->setAxisX(waitingAxisX, waitingSeries);
    waitingChart->setAxisY(waitingAxisY, waitingSeries);
    
    // Update Turnaround Time Chart
    QChart* turnaroundChart = turnaroundTimeChartView->chart();
    turnaroundChart->removeAllSeries();
    
    QLineSeries* turnaroundSeries = new QLineSeries();
    turnaroundSeries->setName("Average Turnaround Time");
    
    for (int i = 0; i < completedProcessesHistory.size(); ++i) {
        turnaroundSeries->append(completedProcessesHistory[i], avgTurnaroundTimeHistory[i]);
    }
    
    turnaroundChart->addSeries(turnaroundSeries);
    
    QValueAxis* turnaroundAxisX = new QValueAxis();
    turnaroundAxisX->setTitleText("Completed Processes");
    turnaroundAxisX->setLabelFormat("%d");
    
    QValueAxis* turnaroundAxisY = new QValueAxis();
    turnaroundAxisY->setTitleText("Time Units");
    turnaroundAxisY->setLabelFormat("%.1f");
    
    turnaroundChart->setAxisX(turnaroundAxisX, turnaroundSeries);
    turnaroundChart->setAxisY(turnaroundAxisY, turnaroundSeries);
    
    // Update Response Time Chart
    QChart* responseChart = responseTimeChartView->chart();
    responseChart->removeAllSeries();
    
    QLineSeries* responseSeries = new QLineSeries();
    responseSeries->setName("Average Response Time");
    
    for (int i = 0; i < completedProcessesHistory.size(); ++i) {
        responseSeries->append(completedProcessesHistory[i], avgResponseTimeHistory[i]);
    }
    
    responseChart->addSeries(responseSeries);
    
    QValueAxis* responseAxisX = new QValueAxis();
    responseAxisX->setTitleText("Completed Processes");
    responseAxisX->setLabelFormat("%d");
    
    QValueAxis* responseAxisY = new QValueAxis();
    responseAxisY->setTitleText("Time Units");
    responseAxisY->setLabelFormat("%.1f");
    
    responseChart->setAxisX(responseAxisX, responseSeries);
    responseChart->setAxisY(responseAxisY, responseSeries);
}

void MainWindow::updateProcessComparisonChart()
{
    // Update Process Comparison Chart
    QChart* comparisonChart = processComparisonChartView->chart();
    comparisonChart->removeAllSeries();
    
    // Create bar sets for different metrics
    QBarSet* burstSet = new QBarSet("Burst Time");
    QBarSet* waitingSet = new QBarSet("Waiting Time");
    QBarSet* turnaroundSet = new QBarSet("Turnaround Time");
    QBarSet* responseSet = new QBarSet("Response Time");
    
    // Set colors
    burstSet->setColor(QColor(100, 100, 255));
    waitingSet->setColor(QColor(255, 100, 100));
    turnaroundSet->setColor(QColor(100, 255, 100));
    responseSet->setColor(QColor(255, 200, 0));
    
    // Prepare categories (process IDs)
    QStringList categories;
    
    // Add data for each process
    for (const Process& p : processes) {
        categories << "P" + QString::number(p.id);
        
        *burstSet << p.burstTime;
        
        if (p.isCompleted) {
            *waitingSet << p.waitingTime;
            *turnaroundSet << p.turnaroundTime;
            *responseSet << p.responseTime;
        } else {
            *waitingSet << 0;
            *turnaroundSet << 0;
            *responseSet << (p.responseTime >= 0 ? p.responseTime : 0);
        }
    }
    
    // Create bar series
    QBarSeries* series = new QBarSeries();
    series->append(burstSet);
    series->append(waitingSet);
    series->append(turnaroundSet);
    series->append(responseSet);
    
    // Add series to chart
    comparisonChart->addSeries(series);
    
    // Create axes
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    comparisonChart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Time Units");
    comparisonChart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    // Set animation options
    comparisonChart->setAnimationOptions(QChart::SeriesAnimations);
}

void MainWindow::updatePieChart()
{
    // Update Pie Chart for CPU Time Distribution
    QChart* pieChart = pieChartView->chart();
    pieChart->removeAllSeries();
    
    // Create pie series
    QPieSeries* series = new QPieSeries();
    
    // Add data for each process
    int totalBurstTime = 0;
    for (const Process& p : processes) {
        totalBurstTime += p.burstTime;
    }
    
    if (totalBurstTime > 0) {
        for (const Process& p : processes) {
            double percentage = (p.burstTime * 100.0) / totalBurstTime;
            QPieSlice* slice = series->append("P" + QString::number(p.id) + " (" + QString::number(percentage, 'f', 1) + "%)", p.burstTime);
            slice->setColor(p.color);
            slice->setLabelVisible();
            slice->setLabelPosition(QPieSlice::LabelOutside);
            slice->setLabelColor(Qt::black);
            slice->setBrush(p.color);
        }
    }
    
    // Add series to chart
    pieChart->addSeries(series);
    pieChart->setTitle("CPU Time Distribution");
    
    // Set animation options
    pieChart->setAnimationOptions(QChart::SeriesAnimations);
}

void MainWindow::updateThroughputChart()
{
    // Update Throughput Chart
    QChart* chart = throughputChartView->chart();
    chart->removeAllSeries();
    
    QLineSeries* series = new QLineSeries();
    series->setName("Throughput");
    
    // Calculate throughput at each time point
    QVector<QPair<int, double>> throughputData;
    for (int t = 0; t < timeHistory.size(); t++) {
        int time = timeHistory[t];
        if (time == 0) continue;
        
        // Count completed processes up to this time
        int completed = 0;
        for (const Process& p : processes) {
            if (p.isCompleted && p.completionTime <= time) {
                completed++;
            }
        }
        
        double throughput = (double)completed / time;
        throughputData.append(QPair<int, double>(time, throughput));
        series->append(time, throughput);
    }
    
    chart->addSeries(series);
    
    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Time");
    axisX->setLabelFormat("%d");
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Processes/Unit Time");
    axisY->setLabelFormat("%.2f");
    
    chart->setAxisX(axisX, series);
    chart->setAxisY(axisY, series);
}

void MainWindow::on_speedSlider_valueChanged(int value)
{
    simulationSpeed = value;
    ui->speedValueLabel->setText(QString::number(value) + " ms");
    
    if (simulationRunning) {
        simulationTimer->stop();
        simulationTimer->start(simulationSpeed);
    }
}

void MainWindow::on_importButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, 
                                                   "Import Processes", 
                                                   "", 
                                                   "CSV Files (*.csv);;Text Files (*.txt);;All Files (*)");
    if (!filename.isEmpty()) {
        importProcesses(filename);
    }
}

void MainWindow::importProcesses(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to open file!");
        return;
    }
    
    QTextStream in(&file);
    processes.clear();
    
    // Skip header line if it exists
    QString line = in.readLine();
    if (!line.startsWith("ID,") && !line.isEmpty()) {
        // Process the first line
        QStringList values = line.split(',');
        if (values.size() >= 3) {
            bool idOk, arrivalOk, burstOk;
            int id = values[0].toInt(&idOk);
            int arrivalTime = values[1].toInt(&arrivalOk);
            int burstTime = values[2].toInt(&burstOk);
            
            if (idOk && arrivalOk && burstOk && burstTime > 0) {
                Process newProcess;
                newProcess.id = id;
                newProcess.arrivalTime = arrivalTime;
                newProcess.burstTime = burstTime;
                newProcess.remainingTime = burstTime;
                newProcess.completionTime = 0;
                newProcess.turnaroundTime = 0;
                newProcess.waitingTime = 0;
                newProcess.responseTime = -1;
                newProcess.isArrived = false;
                newProcess.isCompleted = false;
                newProcess.hasStarted = false;
                generateRandomColor(newProcess);
                
                processes.append(newProcess);
            }
        }
    }
    
    // Read remaining lines
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList values = line.split(',');
        if (values.size() >= 3) {
            bool idOk, arrivalOk, burstOk;
            int id = values[0].toInt(&idOk);
            int arrivalTime = values[1].toInt(&arrivalOk);
            int burstTime = values[2].toInt(&burstOk);
            
            if (idOk && arrivalOk && burstOk && burstTime > 0) {
                // Check for duplicate IDs
                bool duplicateId = false;
                for (const Process& p : processes) {
                    if (p.id == id) {
                        duplicateId = true;
                        break;
                    }
                }
                
                if (!duplicateId) {
                    Process newProcess;
                    newProcess.id = id;
                    newProcess.arrivalTime = arrivalTime;
                    newProcess.burstTime = burstTime;
                    newProcess.remainingTime = burstTime;
                    newProcess.completionTime = 0;
                    newProcess.turnaroundTime = 0;
                    newProcess.waitingTime = 0;
                    newProcess.responseTime = -1;
                    newProcess.isArrived = false;
                    newProcess.isCompleted = false;
                    newProcess.hasStarted = false;
                    generateRandomColor(newProcess);
                    
                    processes.append(newProcess);
                }
            }
        }
    }
    
    file.close();
    
    if (processes.isEmpty()) {
        QMessageBox::warning(this, "Import Failed", "No valid processes found in the file!");
    } else {
        updateProcessTable();
        updateProcessComparisonChart();
        updatePieChart();
        ui->startSimulationButton->setEnabled(true);
        ui->clearAllButton->setEnabled(true);
        ui->processIdSpinBox->setValue(processes.last().id + 1);
        
        QMessageBox::information(this, "Import Successful", 
                              QString("Successfully imported %1 processes.").arg(processes.size()));
    }
}

void MainWindow::on_exportButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, 
                                                   "Export Results", 
                                                   "fcfs_simulation_results.csv", 
                                                   "CSV Files (*.csv);;Text Files (*.txt);;All Files (*)");
    if (!filename.isEmpty()) {
        exportResults(filename);
    }
}

void MainWindow::exportResults(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to open file for writing!");
        return;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << "Simulation Results - FCFS Scheduler\n";
    out << "Date: " << QDateTime::currentDateTime().toString() << "\n\n";
    
    // Write process details
    out << "Process ID,Arrival Time,Burst Time,Completion Time,Turnaround Time,Waiting Time,Response Time\n";
    
    for (const Process& p : processes) {
        out << p.id << ",";
        out << p.arrivalTime << ",";
        out << p.burstTime << ",";
        
        if (p.isCompleted) {
            out << p.completionTime << ",";
            out << p.turnaroundTime << ",";
            out << p.waitingTime << ",";
            out << p.responseTime;
        } else {
            out << "-,-,-,-";
        }
        
        out << "\n";
    }
    
    out << "\nSummary Statistics\n";
    
    // Calculate statistics
    int completedProcesses = 0;
    double totalTurnaroundTime = 0;
    double totalWaitingTime = 0;
    double totalResponseTime = 0;
    
    for (const Process& p : processes) {
        if (p.isCompleted) {
            completedProcesses++;
            totalTurnaroundTime += p.turnaroundTime;
            totalWaitingTime += p.waitingTime;
            totalResponseTime += p.responseTime;
        }
    }
    
    if (completedProcesses > 0) {
        double avgTurnaroundTime = totalTurnaroundTime / completedProcesses;
        double avgWaitingTime = totalWaitingTime / completedProcesses;
        double avgResponseTime = totalResponseTime / completedProcesses;
        double throughput = (currentTime > 0) ? ((double)completedProcesses / currentTime) : 0;
        double cpuUtilization = calculateCpuUtilization();
        
        out << "Total Processes," << processes.size() << "\n";
        out << "Completed Processes," << completedProcesses << "\n";
        out << "Total Time," << currentTime << "\n";
        out << "Average Turnaround Time," << QString::number(avgTurnaroundTime, 'f', 2) << "\n";
        out << "Average Waiting Time," << QString::number(avgWaitingTime, 'f', 2) << "\n";
        out << "Average Response Time," << QString::number(avgResponseTime, 'f', 2) << "\n";
        out << "Throughput (processes/unit time)," << QString::number(throughput, 'f', 2) << "\n";
        out << "CPU Utilization (%)," << QString::number(cpuUtilization, 'f', 2) << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "Export Successful", "Results exported successfully!");
}

void MainWindow::on_removeProcessButton_clicked()
{
    QList<QTableWidgetItem*> selectedItems = ui->processTableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a process to remove!");
        return;
    }
    
    int row = selectedItems.first()->row();
    int processId = ui->processTableWidget->item(row, 0)->text().toInt();
    
    // Confirm removal
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Removal",
                                                            QString("Are you sure you want to remove Process %1?").arg(processId),
                                                            QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        processes.remove(row);
        updateProcessTable();
        updateProcessComparisonChart();
        updatePieChart();
        ui->startSimulationButton->setEnabled(!processes.isEmpty());
        ui->clearAllButton->setEnabled(!processes.isEmpty());
    }
}

void MainWindow::on_clearAllButton_clicked()
{
    // Confirm clear all
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Clear All",
                                                            "Are you sure you want to remove all processes?",
                                                            QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        processes.clear();
        resetSimulation();
        ui->ganttChartView->scene()->clear();
        ui->processTableWidget->setRowCount(0);
        ui->startSimulationButton->setEnabled(false);
        ui->clearAllButton->setEnabled(false);
        ui->removeProcessButton->setEnabled(false);
        ui->processIdSpinBox->setValue(1);
        
        // Clear charts
        updateProcessComparisonChart();
        updatePieChart();
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    // Update the current tab's content
    switch (index) {
        case 0: // Process Table tab
            updateProcessTable();
            break;
        case 1: // Gantt Chart tab
            updateGanttChart();
            break;
        case 2: // Charts tab
            // Update all charts in the charts tab
            updatePerformanceCharts();
            updateProcessComparisonChart();
            updatePieChart();
            updateThroughputChart();
            break;
    }
}

void MainWindow::on_processTableWidget_cellClicked(int row, int column)
{
    if (row >= 0 && row < processes.size()) {
        ui->removeProcessButton->setEnabled(true);
        updateDetailedProcessInfo(row);
    }
}

void MainWindow::updateDetailedProcessInfo(int row)
{
    if (row < 0 || row >= processes.size()) return;
    
    const Process& p = processes[row];
    
    ui->detailsProcessIdLabel->setText(QString::number(p.id));
    ui->detailsArrivalTimeLabel->setText(QString::number(p.arrivalTime));
    ui->detailsBurstTimeLabel->setText(QString::number(p.burstTime));
    
    if (p.isCompleted) {
        ui->detailsCompletionTimeLabel->setText(QString::number(p.completionTime));
        ui->detailsTurnaroundTimeLabel->setText(QString::number(p.turnaroundTime));
        ui->detailsWaitingTimeLabel->setText(QString::number(p.waitingTime));
        ui->detailsResponseTimeLabel->setText(QString::number(p.responseTime));
        ui->detailsStatusLabel->setText("Completed");
        ui->detailsProgressBar->setValue(100);
    } else if (p.hasStarted) {
        ui->detailsCompletionTimeLabel->setText("Not yet completed");
        ui->detailsTurnaroundTimeLabel->setText("In progress");
        ui->detailsWaitingTimeLabel->setText(QString::number(currentTime - p.arrivalTime - (p.burstTime - p.remainingTime)));
        ui->detailsResponseTimeLabel->setText(QString::number(p.responseTime));
        
        if (row == currentProcessIndex) {
            ui->detailsStatusLabel->setText("Running");
        } else {
            ui->detailsStatusLabel->setText("Ready");
        }
        
        int progress = (int)(((double)(p.burstTime - p.remainingTime) / p.burstTime) * 100);
        ui->detailsProgressBar->setValue(progress);
    } else {
        ui->detailsCompletionTimeLabel->setText("Not yet completed");
        ui->detailsTurnaroundTimeLabel->setText("Not yet started");
        ui->detailsWaitingTimeLabel->setText("Not yet started");
        ui->detailsResponseTimeLabel->setText("Not yet started");
        
        if (p.isArrived) {
            ui->detailsStatusLabel->setText("Ready");
        } else {
            ui->detailsStatusLabel->setText("Not Arrived");
        }
        
        ui->detailsProgressBar->setValue(0);
    }
}