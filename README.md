# Realtime-Scheduler

This is a part of my PhD research. This is a preference oriented fixed priority scheduler for real time systems. In this work, a set of periodic tasks are considerd where some tasks are preferably executed as soon as possible (ASAP) and others as late as possible (ALAP). This code derives promotion times for ALAP tasks (only) and devises a dual-queue based ﬁxed-priority scheduling algorithm that retains ALAP tasks in the waiting queue until their promotion times to delay their executions. Both online and offline approaches are implemented along with Rate Monotonic Scheduling (RMS). The algorithms are:

1. Preference-Oriented RMS (PO-RMS) Approach --> Offline method
2. Slack Management with Wrapper-Tasks (PO-RMS with Slack) --> Online method
3. Dummy Task Technique for Static Spare Capacity (PO-RMS with dummy task) --> Online method
4. RMS

# realtime-scheduler/

1. Constants.h -- contains all the variables
2. Utility.h -- contains all the utility functions
3. TaskSetGenerator.h -- contains the task generator class which generates the task set from a uniform distribution
4. Main.cpp -- the main file
5. RTScheduling.h -- contains the RT scheduler class
6. QComparators.h -- contains the queue comparator class that manages the priority queues
7. Task.h -- contains the task class 
8. Slack.h -- contains the slack class
9. porms -- the executable that starts the simulation

# Run

1. change the values in Constants.h
2. ./porms

# Publication

<https://www.academia.edu/7737839/Preference-Oriented_Fixed-Priority_Scheduling_for_Real-Time_Systems>
