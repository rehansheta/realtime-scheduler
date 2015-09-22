# realtime-scheduler

This is a part of my PhD research. This is a preference oriented fixed priority scheduler for real time systems. In this work, a set of periodic tasks are considerd where some tasks are preferably executed as soon as possible (ASAP) and others as late as possible (ALAP). This code derives promotion times for ALAP tasks (only) and devises a dual-queue based ﬁxed-priority scheduling algorithm that retains ALAP tasks in the waiting queue until their promotion times to delay their executions. Both online and offline approaches are implemented along with Rate Monotonic Scheduling (RMS). The algorithms are:

1. Offline method --> Preference-Oriented RMS (PO-RMS) Approach 
2. Online method --> Slack Management with Wrapper-Tasks (PO-RMS with Slack)
                 --> Dummy Task Technique for Static Spare Capacity (PO-RMS with dummy task)

# Publication

<https://www.academia.edu/7737839/Preference-Oriented_Fixed-Priority_Scheduling_for_Real-Time_Systems>
