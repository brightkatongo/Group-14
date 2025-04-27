# FCFS Process Scheduler Simulator

This application simulates the First Come First Serve (FCFS) CPU scheduling algorithm with an advanced visualization interface.

## Features

- Add, edit, and remove processes with customizable arrival times and burst times
- Import and export process data from/to CSV files
- Real-time simulation with adjustable speed
- Gantt chart visualization of process execution
- Multiple performance analysis charts:
  - CPU Utilization
  - Average Waiting Time
  - Average Turnaround Time
  - Average Response Time
  - Process Comparison
  - Throughput Analysis
  - CPU Time Distribution (Pie Chart)
- Detailed statistics and metrics
- Process-specific information panel

## Building the Application

### Prerequisites

- Qt 5.12 or later
- Qt Charts module
- C++11 compatible compiler

### Build Instructions

1. Open the `FCFSSimulator.pro` file in Qt Creator
2. Configure the project for your desired platform
3. Build and run the application

Alternatively, use qmake from the command line:

```bash
qmake FCFSSimulator.pro
make
```

## Usage Guide

### Adding Processes

1. Enter process details in the left panel:
   - Process ID: Unique identifier for the process
   - Arrival Time: When the process arrives in the ready queue
   - Burst Time: CPU time required by the process
2. Click "Add Process" to add it to the simulation

### Running the Simulation

1. Click "Start" to begin the simulation
2. Use the speed slider to adjust simulation speed
3. Click "Pause" to temporarily halt the simulation
4. Click "Reset" to restart the simulation from the beginning

### Analyzing Results

- Process Table: Shows detailed information about each process
- Gantt Chart: Visualizes the execution timeline
- Performance Charts: Provides various metrics and performance visualizations

### Importing/Exporting Data

- Click "Import" to load process data from a CSV file
- Click "Export Results" to save simulation results to a CSV file

## CSV Format

### Import Format
```
ID,ArrivalTime,BurstTime
1,0,5
2,2,3
3,4,7
```

### Export Format
```
Simulation Results - FCFS Scheduler
Date: [Current Date and Time]

Process ID,Arrival Time,Burst Time,Completion Time,Turnaround Time,Waiting Time,Response Time
1,0,5,5,5,0,0
2,2,3,8,6,3,3
3,4,7,15,11,4,4

Summary Statistics
Total Processes,3
Completed Processes,3
Total Time,15
Average Turnaround Time,7.33
Average Waiting Time,2.33
Average Response Time,2.33
Throughput (processes/unit time),0.20
CPU Utilization (%),100.00
```

## Understanding Performance Metrics

- **Turnaround Time**: Time from arrival to completion (Completion Time - Arrival Time)
- **Waiting Time**: Time spent waiting in the ready queue (Turnaround Time - Burst Time)
- **Response Time**: Time from arrival to first CPU access
- **CPU Utilization**: Percentage of time the CPU is busy
- **Throughput**: Number of processes completed per unit time
