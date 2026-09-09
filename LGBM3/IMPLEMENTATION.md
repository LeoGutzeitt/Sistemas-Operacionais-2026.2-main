# Implementation Details

## Architecture

The scheduler implementation is divided into four main components:

### 1. Parser (parser.c/h)
- Reads and validates input files
- Checks constraints: C ≤ D ≤ P
- Stores task definitions
- Handles various error conditions

### 2. Task Manager (task.c/h)
- Manages task state during simulation
- Tracks task instances, deadlines, and arrivals
- Provides helper functions to query task status

### 3. Scheduler (scheduler.c/h)
- Implements both RM and EDF algorithms
- Handles preemption and deadline detection
- Generates execution traces and statistics

### 4. Main (main.c)
- Command-line argument processing
- Coordinates the overall execution

## Algorithm Details

### Rate-Monotonic Scheduling
Priority assignment: `priority ∝ 1/period`
- Shorter period → higher priority
- Static priorities assigned at startup
- Preemptive: higher priority tasks interrupt lower priority ones

Implementation:
```c
find_highest_priority_task_rate() {
    // Find ready task with smallest period
    // Tie-break by task order in input file
}
```

### Earliest-Deadline-First Scheduling
Priority assignment: Based on absolute deadline
- Earlier deadline → higher priority
- Dynamic priorities updated as tasks arrive
- Preemptive: task with nearest deadline interrupts others

Implementation:
```c
find_highest_priority_task_edf() {
    // Find ready task with earliest absolute deadline
    // Tie-break by task order in input file
}
```

## Execution Model

### Time Advancement
- Time advances in discrete 1-unit increments
- At each time unit:
  1. Check for new task arrivals
  2. Detect missed deadlines
  3. Select highest-priority ready task
  4. Execute selected task for 1 unit (or go idle)

### State Transitions
```
Ready → Running → Finished/Preempted
  ↓                    ↓
Not arrived        Ready for next instance
```

### Deadline Miss Handling
When current time reaches a task's absolute deadline:
- Remaining burst is discarded
- Task counts as lost deadline
- Task waits for next instance to arrive

## Task Instance Management

When a task instance completes or misses deadline:
```c
reset_task_instance(task, current_time) {
    instance++;
    remaining_burst = original_burst;
    absolute_deadline = current_time + relative_deadline;
    next_arrival = current_time + period;
}
```

## Statistics Tracking

Three categories tracked per task:

1. **Lost Deadlines**: Instances that missed their deadline
2. **Complete Execution**: Instances that finished before deadline
3. **Killed**: Instances still executing when simulation ends

## Edge Cases Handled

1. **Simultaneous arrivals**: Multiple tasks arriving at time 0
2. **Task preemption**: Higher priority task arrives during execution
3. **Deadline races**: Task completes exactly at deadline (counts as success)
4. **Idle periods**: When no task is ready to execute
5. **Tie-breaking**: Consistent ordering by input file position

## Time Complexity

For each time unit:
- Arrival check: O(n)
- Deadline check: O(n)
- Priority selection: O(n)
- Overall: O(total_time × n)

Where n = number of tasks
