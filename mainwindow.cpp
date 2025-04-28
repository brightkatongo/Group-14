#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QRandomGenerator>
#include <QDateTime>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QtMath>
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentTime(0)
    , currentProcessIndex(-1)
    , simulationRunning(false)
{
    ui->setupUi(this);
    
    // Initialize random seed
    QRandomGenerator::global()->seed(QDateTime::currentMSecsSinceEpoch());
    
    // Setup graphics scene for Gantt chart
    ganttChartScene = new QGraphicsScene(this);
    ui->ganttChartView->setScene(ganttChartScene);
    
    // Initialize simulation timer
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::simulationStep);
    
    // Setup process table
    setupProcessTable();
    
    // Setup Gantt chart
    setupGanttChart();
    
    // Setup performance charts
    setupPerformanceCharts();
    
    // Connect slider value changed signal
    connect(ui->speedSlider, &QSlider::valueChanged, this, &MainWindow::on_speedSlider_valueChanged);
    
    // Connect menu actions
    connect(ui->actionImport, &QAction::triggered, this, &MainWindow::on_actionImport_triggered);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::on_actionExport_triggered);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::on_actionExit_triggered);
    connect(ui->actionStart, &QAction::triggered, this, &MainWindow::on_actionStart_triggered);
    connect(ui->actionPause, &QAction::triggered, this, &MainWindow::on_actionPause_triggered);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::on_actionReset_triggered);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::on_actionAbout_triggered);
    connect(ui->actionDocumentation, &QAction::triggered, this, &MainWindow::on_actionDocumentation_triggered);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupProcessTable()
{
    ui->processTableWidget->setColumnCount(10);
    ui->processTableWidget->setHorizontalHeaderLabels({
        "ID", "Arrival Time", "Burst Time", "Priority", "Start Time",
        "Completion Time", "Turnaround Time", "Waiting Time", "Response Time", "Status"
    });
    ui->processTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->processTableWidget->verticalHeader()->setVisible(false);
    ui->processTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->processTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->processTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void MainWindow::setupGanttChart()
{
    ganttChartScene->clear();
    ui->ganttChartView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->ganttChartView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::setupPerformanceCharts()
{
    // CPU Utilization Chart
    cpuUtilizationChart = new QChart();
    cpuUtilizationChart->setTitle("CPU Utilization Over Time");
    cpuUtilizationChart->setAnimationOptions(QChart::SeriesAnimations);
    cpuUtilizationView = new QChartView(cpuUtilizationChart);
    cpuUtilizationView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *cpuUtilizationLayout = new QVBoxLayout(ui->cpuUtilizationTab);
    cpuUtilizationLayout->addWidget(cpuUtilizationView);
    
    // Waiting Time Chart
    waitingTimeChart = new QChart();
    waitingTimeChart->setTitle("Waiting Time per Process");
    waitingTimeChart->setAnimationOptions(QChart::SeriesAnimations);
    waitingTimeView = new QChartView(waitingTimeChart);
    waitingTimeView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *waitingTimeLayout = new QVBoxLayout(ui->waitingTimeTab);
    waitingTimeLayout->addWidget(waitingTimeView);
    
    // Turnaround Time Chart
    turnaroundTimeChart = new QChart();
    turnaroundTimeChart->setTitle("Turnaround Time per Process");
    turnaroundTimeChart->setAnimationOptions(QChart::SeriesAnimations);
    turnaroundTimeView = new QChartView(turnaroundTimeChart);
    turnaroundTimeView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *turnaroundTimeLayout = new QVBoxLayout(ui->turnaroundTimeTab);
    turnaroundTimeLayout->addWidget(turnaroundTimeView);
    
    // Response Time Chart
    responseTimeChart = new QChart();
    responseTimeChart->setTitle("Response Time per Process");
    responseTimeChart->setAnimationOptions(QChart::SeriesAnimations);
    responseTimeView = new QChartView(responseTimeChart);
    responseTimeView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *responseTimeLayout = new QVBoxLayout(ui->responseTimeTab);
    responseTimeLayout->addWidget(responseTimeView);
    
    // Process Comparison Chart
    processComparisonChart = new QChart();
    processComparisonChart->setTitle("Process Time Metrics Comparison");
    processComparisonChart->setAnimationOptions(QChart::SeriesAnimations);
    processComparisonView = new QChartView(processComparisonChart);
    processComparisonView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *processComparisonLayout = new QVBoxLayout(ui->processComparisonTab);
    processComparisonLayout->addWidget(processComparisonView);
    
    // Throughput Chart
    throughputChart = new QChart();
    throughputChart->setTitle("Throughput Over Time");
    throughputChart->setAnimationOptions(QChart::SeriesAnimations);
    throughputView = new QChartView(throughputChart);
    throughputView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *throughputLayout = new QVBoxLayout(ui->throughputTab);
    throughputLayout->addWidget(throughputView);
    
    // CPU Distribution Chart (Pie Chart)
    cpuDistributionChart = new QChart();
    cpuDistributionChart->setTitle("CPU Time Distribution");
    cpuDistributionChart->setAnimationOptions(QChart::SeriesAnimations);
    cpuDistributionView = new QChartView(cpuDistributionChart);
    cpuDistributionView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout *cpuDistributionLayout = new QVBoxLayout(ui->pieChartTab);
    cpuDistributionLayout->addWidget(cpuDistributionView);
}

bool MainWindow::validateProcessInput()
{
    // Check if process ID is unique
    int newId = ui->processIdSpinBox->value();
    for (const Process &process : processes) {
        if (process.id == newId) {
            QMessageBox::warning(this, "Invalid Input", "Process ID must be unique.");
            return false;
        }
    }
    return true;
}

void MainWindow::on_addProcessButton_clicked()
{
    if (!validateProcessInput()) {
        return;
    }
    
    Process newProcess;
    newProcess.id = ui->processIdSpinBox->value();
    newProcess.arrivalTime = ui->arrivalTimeSpinBox->value();
    newProcess.burstTime = ui->burstTimeSpinBox->value();
    newProcess.priority = ui->prioritySpinBox->value();
    newProcess.remainingTime = newProcess.burstTime;
    newProcess.status = "Waiting";
    newProcess.color = getRandomColor();
    newProcess.completionTime = -1;
    newProcess.turnaroundTime = -1;
    newProcess.waitingTime = -1;
    newProcess.responseTime = -1;
    newProcess.startTime = -1;
    
    processes.append(newProcess);
    processColors[newProcess.id] = newProcess.color;
    
    // Auto-increment process ID
    ui->processIdSpinBox->setValue(ui->processIdSpinBox->value() + 1);
    
    // Update the table
    updateProcessTable();
    
    // Enable remove process button if there are processes
    ui->removeProcessButton->setEnabled(!processes.isEmpty());
}

QColor MainWindow::getRandomColor()
{
    return QColor(
        QRandomGenerator::global()->bounded(50, 200),
        QRandomGenerator::global()->bounded(50, 200),
        QRandomGenerator::global()->bounded(50, 200)
    );
}

void MainWindow::updateProcessTable()
{
    ui->processTableWidget->setRowCount(processes.size());
    
    for (int i = 0; i < processes.size(); ++i) {
        const Process &process = processes[i];
        
        ui->processTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(process.id)));
        ui->processTableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(process.arrivalTime)));
        ui->processTableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(process.burstTime)));
        ui->processTableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(process.priority)));
        
        if (process.startTime != -1) {
            ui->processTableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(process.startTime)));
        } else {
            ui->processTableWidget->setItem(i, 4, new QTableWidgetItem("-"));
        }
        
        if (process.completionTime != -1) {
            ui->processTableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(process.completionTime)));
            ui->processTableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(process.turnaroundTime)));
            ui->processTableWidget->setItem(i, 7, new QTableWidgetItem(QString::number(process.waitingTime)));
            ui->processTableWidget->setItem(i, 8, new QTableWidgetItem(QString::number(process.responseTime)));
        } else {
            ui->processTableWidget->setItem(i, 5, new QTableWidgetItem("-"));
            ui->processTableWidget->setItem(i, 6, new QTableWidgetItem("-"));
            ui->processTableWidget->setItem(i, 7, new QTableWidgetItem("-"));
            ui->processTableWidget->setItem(i, 8, new QTableWidgetItem("-"));
        }
        
        ui->processTableWidget->setItem(i, 9, new QTableWidgetItem(process.status));
        
        // Color the row based on status
        for (int j = 0; j < ui->processTableWidget->columnCount(); ++j) {
            QTableWidgetItem *item = ui->processTableWidget->item(i, j);
            if (item) {
                if (process.status == "Running") {
                    item->setBackground(QBrush(Qt::green));
                } else if (process.status == "Completed") {
                    item->setBackground(QBrush(Qt::gray));
                } else if (process.status == "Waiting") {
                    item->setBackground(QBrush(Qt::white));
                }
            }
        }
    }
}

void MainWindow::on_removeProcessButton_clicked()
{
    QList<QTableWidgetItem*> selectedItems = ui->processTableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "No Selection", "Please select a process to remove.");
        return;
    }
    
    int row = selectedItems.first()->row();
    if (row >= 0 && row < processes.size()) {
        processes.removeAt(row);
        updateProcessTable();
        
        // Disable remove button if no processes left
        ui->removeProcessButton->setEnabled(!processes.isEmpty());
    }
}

void MainWindow::on_clearAllButton_clicked()
{
    processes.clear();
    updateProcessTable();
    resetSimulation();
    ui->removeProcessButton->setEnabled(false);
}

void MainWindow::on_importButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import Processes", "", "CSV Files (*.csv);;Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for reading.");
        return;
    }
    
    QTextStream in(&file);
    processes.clear();
    
    // Skip header line if it exists
    QString line = in.readLine();
    if (!line.startsWith(QRegExp("\\d+"))) {
        // First line was a header, read the next line
        line = in.readLine();
    }
    
    while (!line.isNull()) {
        QStringList fields = line.split(',');
        if (fields.size() >= 4) {
            Process process;
            process.id = fields[0].toInt();
            process.arrivalTime = fields[1].toInt();
            process.burstTime = fields[2].toInt();
            process.priority = fields[3].toInt();
            process.remainingTime = process.burstTime;
            process.status = "Waiting";
            process.color = getRandomColor();
            process.completionTime = -1;
            process.turnaroundTime = -1;
            process.waitingTime = -1;
            process.responseTime = -1;
            process.startTime = -1;
            
            processes.append(process);
            processColors[process.id] = process.color;
        }
        line = in.readLine();
    }
    
    file.close();
    updateProcessTable();
    ui->removeProcessButton->setEnabled(!processes.isEmpty());
    
    // Find the next available process ID
    int maxId = 0;
    for (const Process &process : processes) {
        maxId = qMax(maxId, process.id);
    }
    ui->processIdSpinBox->setValue(maxId + 1);
    
    QMessageBox::information(this, "Import Successful", QString("Imported %1 processes.").arg(processes.size()));
}

void MainWindow::on_exportButton_clicked()
{
    if (processes.isEmpty()) {
        QMessageBox::warning(this, "No Data", "There are no processes to export.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, "Export Results", "", "CSV Files (*.csv);;Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
        return;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << "Process ID,Arrival Time,Burst Time,Priority,Start Time,Completion Time,Turnaround Time,Waiting Time,Response Time,Status\n";
    
    // Write data
    for (const Process &process : processes) {
        out << process.id << ","
            << process.arrivalTime << ","
            << process.burstTime << ","
            << process.priority << ",";
        
        if (process.startTime != -1) {
            out << process.startTime;
        }
        out << ",";
        
        if (process.completionTime != -1) {
            out << process.completionTime << ","
                << process.turnaroundTime << ","
                << process.waitingTime << ","
                << process.responseTime;
        } else {
            out << ",,,";
        }
        
        out << "," << process.status << "\n";
    }
    
    // Write summary statistics
    out << "\nSummary Statistics\n";
    out << "Total Processes," << processes.size() << "\n";
    
    if (currentTime > 0) {
        int completedProcesses = 0;
        int totalTurnaroundTime = 0;
        int totalWaitingTime = 0;
        int totalResponseTime = 0;
        
        for (const Process &process : processes) {
            if (process.completionTime != -1) {
                completedProcesses++;
                totalTurnaroundTime += process.turnaroundTime;
                totalWaitingTime += process.waitingTime;
                totalResponseTime += process.responseTime;
            }
        }
        
        if (completedProcesses > 0) {
            out << "Completed Processes," << completedProcesses << "\n";
            out << "Average Turnaround Time," << QString::number(static_cast<double>(totalTurnaroundTime) / completedProcesses, 'f', 2) << "\n";
            out << "Average Waiting Time," << QString::number(static_cast<double>(totalWaitingTime) / completedProcesses, 'f', 2) << "\n";
            out << "Average Response Time," << QString::number(static_cast<double>(totalResponseTime) / completedProcesses, 'f', 2) << "\n";
            out << "Throughput," << QString::number(static_cast<double>(completedProcesses) / currentTime, 'f', 2) << " processes/unit time\n";
        }
    }
    
    file.close();
    QMessageBox::information(this, "Export Successful", "Results exported successfully.");
}

void MainWindow::on_startSimulationButton_clicked()
{
    if (processes.isEmpty()) {
        QMessageBox::warning(this, "No Processes", "Please add processes before starting the simulation.");
        return;
    }
    
    if (!simulationRunning) {
        startSimulation();
        ui->startSimulationButton->setText("Pause");
    } else {
        pauseSimulation();
        ui->startSimulationButton->setText("Resume");
    }
}

void MainWindow::on_resetButton_clicked()
{
    resetSimulation();
}

void MainWindow::on_speedSlider_valueChanged(int value)
{
    ui->speedValueLabel->setText(QString("%1 ms").arg(value));
    if (simulationRunning) {
        simulationTimer->setInterval(value);
    }
}

void MainWindow::on_processTableWidget_cellClicked(int row, int column)
{
    if (row >= 0 && row < processes.size()) {
        updateProcessDetails(row);
    }
}

void MainWindow::updateProcessDetails(int processIndex)
{
    if (processIndex < 0 || processIndex >= processes.size()) {
        return;
    }
    
    const Process &process = processes[processIndex];
    
    ui->detailsProcessIdLabel->setText(QString::number(process.id));
    ui->detailsArrivalTimeLabel->setText(QString::number(process.arrivalTime));
    ui->detailsBurstTimeLabel->setText(QString::number(process.burstTime));
    
    if (process.completionTime != -1) {
        ui->detailsCompletionTimeLabel->setText(QString::number(process.completionTime));
        ui->detailsTurnaroundTimeLabel->setText(QString::number(process.turnaroundTime));
        ui->detailsWaitingTimeLabel->setText(QString::number(process.waitingTime));
        ui->detailsResponseTimeLabel->setText(QString::number(process.responseTime));
    } else {
        ui->detailsCompletionTimeLabel->setText("-");
        ui->detailsTurnaroundTimeLabel->setText("-");
        ui->detailsWaitingTimeLabel->setText("-");
        ui->detailsResponseTimeLabel->setText("-");
    }
    
    ui->detailsStatusLabel->setText(process.status);
    
    if (process.status == "Running") {
        int progress = ((process.burstTime - process.remainingTime) * 100) / process.burstTime;
        ui->detailsProgressBar->setValue(progress);
    } else if (process.status == "Completed") {
        ui->detailsProgressBar->setValue(100);
    } else {
        ui->detailsProgressBar->setValue(0);
    }
}

void MainWindow::sortProcessesByArrivalTime()
{
    std::sort(processes.begin(), processes.end(), [](const Process &a, const Process &b) {
        return a.arrivalTime < b.arrivalTime;
    });
}

void MainWindow::resetSimulation()
{
    pauseSimulation();
    
    currentTime = 0;
    currentProcessIndex = -1;
    
    // Reset process stats
    for (int i = 0; i < processes.size(); ++i) {
        processes[i].remainingTime = processes[i].burstTime;
        processes[i].status = "Waiting";
        processes[i].completionTime = -1;
        processes[i].turnaroundTime = -1;
        processes[i].waitingTime = -1;
        processes[i].responseTime = -1;
        processes[i].startTime = -1;
    }
    
    // Update UI elements
    ui->currentTimeLabel->setText("Current Time: 0");
    ui->progressLabel->setText(QString("Progress: 0/%1 processes completed").arg(processes.size()));
    ui->cpuUtilizationBar->setValue(0);
    ui->avgTurnaroundTimeLabel->setText("Average Turnaround Time: 0.00");
    ui->avgWaitingTimeLabel->setText("Average Waiting Time: 0.00");
    ui->avgResponseTimeLabel->setText("Average Response Time: 0.00");
    ui->throughputLabel->setText("Throughput: 0.00 processes/unit time");
    
    updateProcessTable();
    setupGanttChart();
    ui->startSimulationButton->setText("Start");
    ui->resetButton->setEnabled(false);
}

void MainWindow::initializeSimulation()
{
    resetSimulation();
    sortProcessesByArrivalTime();
    updateProcessTable();
}

void MainWindow::startSimulation()
{
    if (currentTime == 0) {
        initializeSimulation();
    }
    
    simulationRunning = true;
    simulationTimer->start(ui->speedSlider->value());
    ui->resetButton->setEnabled(true);
}

void MainWindow::pauseSimulation()
{
    simulationRunning = false;
    simulationTimer->stop();
}

void MainWindow::simulationStep()
{
    // Increment current time
    currentTime++;
    ui->currentTimeLabel->setText(QString("Current Time: %1").arg(currentTime));
    
    // Check if any process is currently running
    bool hasRunningProcess = false;
    int runningProcessIndex = -1;
    
    for (int i = 0; i < processes.size(); ++i) {
        if (processes[i].status == "Running") {
            hasRunningProcess = true;
            runningProcessIndex = i;
            break;
        }
    }
    
    // If no process is running, find the next process to run (FCFS algorithm)
    if (!hasRunningProcess) {
        for (int i = 0; i < processes.size(); ++i) {
            if (processes[i].status == "Waiting" && processes[i].arrivalTime <= currentTime) {
                processes[i].status = "Running";
                processes[i].startTime = currentTime;
                processes[i].responseTime = currentTime - processes[i].arrivalTime;
                runningProcessIndex = i;
                hasRunningProcess = true;
                break;
            }
        }
    }
    
    // Process the running process
    if (hasRunningProcess) {
        processes[runningProcessIndex].remainingTime--;
        
        // Check if process has completed
        if (processes[runningProcessIndex].remainingTime == 0) {
            processes[runningProcessIndex].status = "Completed";
            processes[runningProcessIndex].completionTime = currentTime;
            processes[runningProcessIndex].turnaroundTime = processes[runningProcessIndex].completionTime - processes[runningProcessIndex].arrivalTime;
            processes[runningProcessIndex].waitingTime = processes[runningProcessIndex].turnaroundTime - processes[runningProcessIndex].burstTime;
        }
    }
    
    // Update UI
    updateProcessTable();
    if (runningProcessIndex != -1) {
        updateProcessDetails(runningProcessIndex);
    }
    updateGanttChart();
    updateSimulationStats();
    
    // Check if all processes are completed
    bool allCompleted = true;
    for (const Process &process : processes) {
        if (process.status != "Completed") {
            allCompleted = false;
            break;
        }
    }
    
    if (allCompleted) {
        finishSimulation();
    }
}

void MainWindow::finishSimulation()
{
    pauseSimulation();
    ui->startSimulationButton->setText("Restart");
    updatePerformanceCharts();
    QMessageBox::information(this, "Simulation Complete", "All processes have completed.");
}

void MainWindow::updateGanttChart()
{
    // Clear existing chart
    ganttChartScene->clear();
    
    int timeScale = 20; // Pixels per time unit
    int rowHeight = 30; // Height of each process row
    int startY = 10;
    int timelineY = startY + processes.size() * rowHeight + 20;
    
    // Draw time axis
    ganttChartScene->addLine(0, timelineY, currentTime * timeScale, timelineY, QPen(Qt::black, 2));
    
    // Draw time markers
    for (int t = 0; t <= currentTime; t += 5) {
        ganttChartScene->addLine(t * timeScale, timelineY - 5, t * timeScale, timelineY + 5, QPen(Qt::black, 1));
        QGraphicsTextItem *timeText = ganttChartScene->addText(QString::number(t));
        timeText->setPos(t * timeScale - 5, timelineY + 5);
    }
    
    // Draw process labels and execution blocks
    for (int i = 0; i < processes.size(); ++i) {
        const Process &process = processes[i];
        int yPos = startY + i * rowHeight;
        
        // Process label
        QGraphicsTextItem *processText = ganttChartScene->addText(QString("P%1").arg(process.id));
        processText->setPos(-30, yPos);
        
        // Draw execution blocks
        if (process.startTime != -1) {
            int xStart = process.startTime * timeScale;
            int width;
            
            if (process.status == "Completed") {
                width = (process.completionTime - process.startTime) * timeScale;
            } else {
                width = (currentTime - process.startTime) * timeScale;
            }
            QGraphicsRectItem *rectItem = ganttChartScene->addRect(xStart, yPos, width, rowHeight - 5, QPen(Qt::black), QBrush(process.color));
            
            // Add process ID text on the rectangle
            QGraphicsTextItem *idText = ganttChartScene->addText(QString("P%1").arg(process.id));
            idText->setDefaultTextColor(Qt::white);
            idText->setPos(xStart + 5, yPos + 2);
            
            // Add start and end time markers
            QGraphicsTextItem *startText = ganttChartScene->addText(QString::number(process.startTime));
            startText->setPos(xStart, yPos + rowHeight);
            
            if (process.status == "Completed") {
                QGraphicsTextItem *endText = ganttChartScene->addText(QString::number(process.completionTime));
                endText->setPos(xStart + width - 10, yPos + rowHeight);
            }
        }
    }
    
    // Adjust scene size
    ganttChartScene->setSceneRect(ganttChartScene->itemsBoundingRect().adjusted(-50, -10, 50, 50));
}

void MainWindow::updateSimulationStats()
{
    int completedProcesses = 0;
    int totalTurnaroundTime = 0;
    int totalWaitingTime = 0;
    int totalResponseTime = 0;
    int totalBurstTime = 0;
    
    for (const Process &process : processes) {
        totalBurstTime += process.burstTime;
        if (process.status == "Completed") {
            completedProcesses++;
            totalTurnaroundTime += process.turnaroundTime;
            totalWaitingTime += process.waitingTime;
            totalResponseTime += process.responseTime;
        }
    }
    
    // Update progress label
    ui->progressLabel->setText(QString("Progress: %1/%2 processes completed").arg(completedProcesses).arg(processes.size()));
    
    // Calculate CPU utilization
    int idleTime = 0;
    for (int t = 0; t < currentTime; ++t) {
        bool busy = false;
        for (const Process &process : processes) {
            if (process.startTime <= t && (process.status == "Running" || (process.status == "Completed" && process.completionTime > t))) {
                busy = true;
                break;
            }
        }
        if (!busy) {
            idleTime++;
        }
    }
    
    int cpuUtilization = currentTime > 0 ? (currentTime - idleTime) * 100 / currentTime : 0;
    ui->cpuUtilizationBar->setValue(cpuUtilization);
    
    // Update average metrics
    double avgTurnaroundTime = completedProcesses > 0 ? static_cast<double>(totalTurnaroundTime) / completedProcesses : 0.0;
    double avgWaitingTime = completedProcesses > 0 ? static_cast<double>(totalWaitingTime) / completedProcesses : 0.0;
    double avgResponseTime = completedProcesses > 0 ? static_cast<double>(totalResponseTime) / completedProcesses : 0.0;
    double throughput = currentTime > 0 ? static_cast<double>(completedProcesses) / currentTime : 0.0;
    
    ui->avgTurnaroundTimeLabel->setText(QString("Average Turnaround Time: %1").arg(avgTurnaroundTime, 0, 'f', 2));
    ui->avgWaitingTimeLabel->setText(QString("Average Waiting Time: %1").arg(avgWaitingTime, 0, 'f', 2));
    ui->avgResponseTimeLabel->setText(QString("Average Response Time: %1").arg(avgResponseTime, 0, 'f', 2));
    ui->throughputLabel->setText(QString("Throughput: %1 processes/unit time").arg(throughput, 0, 'f', 2));
}

void MainWindow::updatePerformanceCharts()
{
    // Clear all charts
    cpuUtilizationChart->removeAllSeries();
    waitingTimeChart->removeAllSeries();
    turnaroundTimeChart->removeAllSeries();
    responseTimeChart->removeAllSeries();
    processComparisonChart->removeAllSeries();
    throughputChart->removeAllSeries();
    cpuDistributionChart->removeAllSeries();
    
    // Only update charts if there are completed processes
    bool hasCompletedProcesses = false;
    for (const Process &process : processes) {
        if (process.status == "Completed") {
            hasCompletedProcesses = true;
            break;
        }
    }
    
    if (!hasCompletedProcesses) {
        return;
    }
    
    // Waiting Time Chart
    QBarSet *waitingTimeSet = new QBarSet("Waiting Time");
    QStringList processCategories;
    
    for (const Process &process : processes) {
        if (process.status == "Completed") {
            *waitingTimeSet << process.waitingTime;
            processCategories << QString("P%1").arg(process.id);
        }
    }
    
    QBarSeries *waitingSeries = new QBarSeries();
    waitingSeries->append(waitingTimeSet);
    waitingTimeChart->addSeries(waitingSeries);
    
    QBarCategoryAxis *waitingAxisX = new QBarCategoryAxis();
    waitingAxisX->append(processCategories);
    waitingTimeChart->addAxis(waitingAxisX, Qt::AlignBottom);
    waitingSeries->attachAxis(waitingAxisX);
    
    QValueAxis *waitingAxisY = new QValueAxis();
    waitingAxisY->setRange(0, 100);  // Adjust range as needed
    waitingTimeChart->addAxis(waitingAxisY, Qt::AlignLeft);
    waitingSeries->attachAxis(waitingAxisY);
    
    // Turnaround Time Chart
    QBarSet *turnaroundTimeSet = new QBarSet("Turnaround Time");
    
    for (const Process &process : processes) {
        if (process.status == "Completed") {
            *turnaroundTimeSet << process.turnaroundTime;
        }
    }
    
    QBarSeries *turnaroundSeries = new QBarSeries();
    turnaroundSeries->append(turnaroundTimeSet);
    turnaroundTimeChart->addSeries(turnaroundSeries);
    
    QBarCategoryAxis *turnaroundAxisX = new QBarCategoryAxis();
    turnaroundAxisX->append(processCategories);
    turnaroundTimeChart->addAxis(turnaroundAxisX, Qt::AlignBottom);
    turnaroundSeries->attachAxis(turnaroundAxisX);
    
    QValueAxis *turnaroundAxisY = new QValueAxis();
    turnaroundAxisY->setRange(0, 100);  // Adjust range as needed
    turnaroundTimeChart->addAxis(turnaroundAxisY, Qt::AlignLeft);
    turnaroundSeries->attachAxis(turnaroundAxisY);
    
    // Response Time Chart
    QBarSet *responseTimeSet = new QBarSet("Response Time");
    
    for (const Process &process : processes) {
        if (process.status == "Completed") {
            *responseTimeSet << process.responseTime;
        }
    }
    
    QBarSeries *responseSeries = new QBarSeries();
    responseSeries->append(responseTimeSet);
    responseTimeChart->addSeries(responseSeries);
    
    QBarCategoryAxis *responseAxisX = new QBarCategoryAxis();
    responseAxisX->append(processCategories);
    responseTimeChart->addAxis(responseAxisX, Qt::AlignBottom);
    responseSeries->attachAxis(responseAxisX);
    
    QValueAxis *responseAxisY = new QValueAxis();
    responseAxisY->setRange(0, 100);  // Adjust range as needed
    responseTimeChart->addAxis(responseAxisY, Qt::AlignLeft);
    responseSeries->attachAxis(responseAxisY);
    
    // Process Comparison Chart
    QBarSet *waitingSet = new QBarSet("Waiting Time");
    QBarSet *responseSet = new QBarSet("Response Time");
    QBarSet *burstSet = new QBarSet("Burst Time");
    
    for (const Process &process : processes) {
        if (process.status == "Completed") {
            *waitingSet << process.waitingTime;
            *responseSet << process.responseTime;
            *burstSet << process.burstTime;
        }
    }
    
    QBarSeries *comparisonSeries = new QBarSeries();
    comparisonSeries->append(waitingSet);
    comparisonSeries->append(responseSet);
    comparisonSeries->append(burstSet);
    processComparisonChart->addSeries(comparisonSeries);
    
    QBarCategoryAxis *comparisonAxisX = new QBarCategoryAxis();
    comparisonAxisX->append(processCategories);
    processComparisonChart->addAxis(comparisonAxisX, Qt::AlignBottom);
    comparisonSeries->attachAxis(comparisonAxisX);
    
    QValueAxis *comparisonAxisY = new QValueAxis();
    comparisonAxisY->setRange(0, 100);  // Adjust range as needed
    processComparisonChart->addAxis(comparisonAxisY, Qt::AlignLeft);
    comparisonSeries->attachAxis(comparisonAxisY);
    
    // CPU Distribution Pie Chart
    QPieSeries *pieSeries = new QPieSeries();
    
    for (const Process &process : processes) {
        if (process.status == "Completed") {
            pieSeries->append(QString("P%1 (%2%)").arg(process.id).arg((process.burstTime * 100) / currentTime), process.burstTime);
            QPieSlice *slice = pieSeries->slices().last();
            slice->setBrush(process.color);
        }
    }
    
    // Add idle time slice if any
    int totalBurstTime = 0;
    for (const Process &process : processes) {
        if (process.status == "Completed") {
            totalBurstTime += process.burstTime;
        }
    }
    
    int idleTime = currentTime - totalBurstTime;
    if (idleTime > 0) {
        pieSeries->append(QString("Idle (%1%)").arg((idleTime * 100) / currentTime), idleTime);
        QPieSlice *idleSlice = pieSeries->slices().last();
        idleSlice->setBrush(Qt::lightGray);
    }
    
    cpuDistributionChart->addSeries(pieSeries);
}

void MainWindow::on_actionImport_triggered()
{
    on_importButton_clicked();
}

void MainWindow::on_actionExport_triggered()
{
    on_exportButton_clicked();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionStart_triggered()
{
    on_startSimulationButton_clicked();
}

void MainWindow::on_actionPause_triggered()
{
    if (simulationRunning) {
        pauseSimulation();
        ui->startSimulationButton->setText("Resume");
    }
}

void MainWindow::on_actionReset_triggered()
{
    on_resetButton_clicked();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About FCFS Scheduler Simulation",
                      "First-Come, First-Served (FCFS) CPU Scheduler Simulation\n\n"
                      "This application simulates the FCFS CPU scheduling algorithm with "
                      "visual representation of the processes execution, Gantt chart, and "
                      "performance metrics.\n\n"
                      "© 2025 CPU Scheduler Simulation Project");
}

void MainWindow::on_actionDocumentation_triggered()
{
    QMessageBox::information(this, "Documentation",
                           "The FCFS Scheduler Simulation follows these steps:\n\n"
                           "1. Add processes with their arrival time and burst time\n"
                           "2. Start the simulation to see how processes are scheduled\n"
                           "3. View real-time statistics and visualizations\n"
                           "4. Export results for further analysis\n\n"
                           "First-Come, First-Served (FCFS) is a non-preemptive scheduling algorithm "
                           "where processes are executed in the order they arrive in the ready queue.");
}