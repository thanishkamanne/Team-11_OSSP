# Process Termination Messages

**Operating Systems and Systems Programming (25CS2104E)**  
**2026–27, Term-I | Section 4 | Team 11**

## Team Members

- **2520030204** — K.Joshi Rushitha
- **2520030583** — M.Thanishka
- **2520030516** — M.Akshaya

**Supervisor:** Ms. Soumya Enukonda

## Abstract

The KL Hospital Priority Management System is designed to manage and organize patients based on their medical priority using Priority Scheduling. The system helps prioritize patients so that those requiring urgent attention can be handled first. It includes patient registration, priority assignment, doctor details, doctor availability, and room allocation. The system also helps identify available doctors and rooms, making the treatment and admission process more organized. Overall, the project aims to improve patient management, resource allocation, and efficient hospital operations.

## Objectives

1. Implement Priority Scheduling for hospital patient management.
2. Prioritize patients based on urgency.
3. Manage patient, doctor, and room details.
4. Calculate Waiting Time (WT) and Turnaround Time (TAT).
5. Improve hospital resource management.

## Project Structure

## Project Structure

```text
Process_Termination_Messages/
│
├── src/
│   ├── main.c
│   ├── termination_test.c
│   └── Makefile
│
├── docs/
│   ├── Project_Problem_Statement.pdf
│   ├── Project_Workflow.pdf
│   └── README.md
│
├── data/
│   └── README.md
│
├── results/
│   ├── linux_normal.txt
│   ├── linux_nonzero.txt
│   ├── linux_abnormal.txt
│   ├── chrome_normal.txt
│   └── chrome_abnormal.txt
│
├── reports/
│   ├── Project_Report.pdf
│   └── Project_Report.md
│
└── README.md
```

## Setup and Execution

Open Ubuntu/WSL and enter the `src` directory:

```bash
cd ~/Process_Termination_Messages/src
```

Compile both programs:

```bash
make clean
make
```

Run the main program:

```bash
./process_monitor
```

### Linux normal termination

Choose **1** and enter:

```text
sleep 10
```

The parent creates the child, waits using `waitpid()`, and reports normal termination and exit status 0.

### Linux non-zero exit status

Choose **1** and enter:

```text
./termination_test error
```

The child terminates normally with exit status 5, allowing the program to demonstrate that normal termination can still have a non-zero exit status.

### Linux signal-based abnormal termination

Choose **1** and enter:

```text
./termination_test signal
```

The test program raises `SIGTERM`. The parent detects this using `WIFSIGNALED()` and reports the signal number using `WTERMSIG()`.

### Chrome Demonstration

The project also contains an additional WSL/Windows Chrome process-monitoring demonstration.

- **Option 2:** Chrome normal termination — launch Chrome and close it normally.
- **Option 3:** Chrome abnormal termination — launch Chrome and press ENTER; the program uses Windows `taskkill /F` to force termination.

Chrome is a Windows process, not a Linux child process. Therefore the Chrome demonstration does **not** claim a POSIX `waitpid()` exit status. The Linux mode is the core POSIX implementation of the project.
1. Project Setup
Step 1: Create a folder named KL_Hospital_Priority_System.
Step 2: Open the folder in VS Code and create a file named hospital.c.
Step 3: Copy and paste this code into hospital.c.
2. Compile the Program
Open the VS Code terminal using Terminal → New Terminal.
If GCC is installed, run:
gcc hospital.c -o hospital


If there are no errors, the program is compiled successfully.
3. Execute the Program
On Windows, run:
.\hospital.exe


If you're using Ubuntu or WSL, run:
./hospital


4. Sample Execution
Enter these example details when prompted.
Example input
Enter number of patients: 3

Patient ID: 101
Patient Name: Ravi
Priority: 2
Arrival Time: 0
Treatment Time: 5

Patient ID: 102
Patient Name: Priya
Priority: 1
Arrival Time: 0
Treatment Time: 3

Patient ID: 103
Patient Name: Arjun
Priority: 3
Arrival Time: 0
Treatment Time: 4



Expected scheduling output
--- Patient Treatment Schedule ---
ID      Name    Priority    WT    TAT
102     Priya   1           0     3
101     Ravi    2           3     8
103     Arjun   3           8     12

Average Waiting Time: 3.67
Average Turnaround Time: 7.67



5. Important Formulas
- Waiting Time (WT) = Start Time − Arrival Time
- Turnaround Time (TAT) = Completion Time − Arrival Time
- Average WT = Total WT / Number of Patients
- Average TAT = Total TAT / Number of Patients
Priority 1 represents the highest priority in this implementation.






























