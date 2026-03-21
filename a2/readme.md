## A2
## Group Members
- Zifan Si
- Stanislav Serbezov

## Individual Contributions
Both members worked together on the code, features, logic, tests, debugs.
## File
- `A2.c`

## Compile and Run

Compile:
`gcc -pthread A2.c -o A2`

Run:
`./A2 5`

## Output

The program prints its output directly in the terminal.  
It also generates a log file for debugging under `/logs`.
(So every printf(...) becomes log_printf(...) as well)

Example output:

```text
TA is sleeping.
Student 3 wakes up the TA and gets immediate help.
TA starts helping student 3 immediately.
TA is helping student 3 for 2 seconds.

Student 2 sits in chair 0.
Student 1 sits in chair 1.
Student 5 sits in chair 2.
Student 4 found no empty chair and will come back later.

TA finished helping student 3.
TA calls student 2. Waiting students left: 2
Student 2 goes into the office for help.

TA finished helping student 2.
TA calls student 1. Waiting students left: 1

TA finished helping student 1.
TA calls student 5. Waiting students left: 0
... continues looping (full logs are also written to `/logs`)
```

## Program Features
- **A. Setup**
  - **a1.** Read number of students from command line
  - **a2.** Initialize shared variables
  - **a3.** Initialize mutex and semaphores
  - **a4.** Allocate arrays for threads, ids, semaphores

- **B. Create Threads**
  - **b1.** Create 1 TA thread
  - **b2.** Create `n` student threads
  - **b3.** Give each student a unique id

- **C. Runtime Logic**
  - **c1.** Student programs for random time
  - **c2.** Student asks TA for help
  - **c3.** If a chair is available, the student waits
  - **c4.** If no chair is available, the student comes back later
  - **c5.** If the TA is sleeping, the student wakes the TA
  - **c6.** TA helps one student at a time
  - **c7.** TA sleeps again if nobody is waiting

- **D. Synchronization**
  - **d1.** Mutex protects shared data
  - **d2.** `students_waiting` wakes the TA
  - **d3.** `student_called[i]` calls one student
  - **d4.** `student_done[i]` tells the student help is done

## UML Diagram
![UML Diagram](image.png)


## UML Diagram
@startuml
start
```
:A. Setup;
:a1. Read number of students;
:a2. Initialize shared variables;
:a3. Initialize mutex and semaphores;
:a4. Allocate arrays for threads, ids, semaphores;

:B. Create threads;
:b1. Create 1 TA thread;
:b2. Create n student threads;
:b3. Give each student a unique id;

fork
  repeat
    :TA thread;
    if (c7. No student waiting\nand no current student?) then (yes)
      :TA sleeps;
    endif
    :d2. Wait for students_waiting;
    :Choose current student\nor next waiting student;
    :d3. Signal student_called[i];
    :c6. Help one student;
    :d4. Signal student_done[i];
  repeat while (running)

fork again
  repeat
    :Student thread;
    :c1. Program for random time;
    :c2. Ask TA for help;

    if (TA free and\nno one waiting?) then (yes)
      :c5. Wake TA if sleeping;
      :Get immediate help;
    else (no)
      if (c3. Chair available?) then (yes)
        :Wait in chair;
      else (no)
        :c4. Come back later;
      endif
    endif
  repeat while (running)

end fork

stop
@enduml
```


