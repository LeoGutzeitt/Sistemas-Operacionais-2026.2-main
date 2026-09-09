# Task Scheduler - Rate-Monotonic and EDF Implementation

## Overview
This project implements a real-time task scheduler simulator that compares two classical scheduling algorithms:
- **Rate-Monotonic (RM)**: Assigns priorities based on task period (shorter period = higher priority)
- **Earliest-Deadline-First (EDF)**: Assigns priorities based on absolute deadline (earlier deadline = higher priority)

Both algorithms are preemptive and support deadline detection.

## Project Structure

```
LGBM3/
├── include/              # Header files
│   ├── parser.h         # Input file parsing
│   ├── scheduler.h      # Scheduler interface
│   └── task.h           # Task definitions
├── src/                 # Source files
│   ├── main.c          # Main program entry point
│   ├── parser.c        # Input file parser implementation
│   ├── scheduler.c     # Scheduler algorithms implementation
│   └── task.c          # Task management functions
├── tests/              # Test files
│   └── voo.txt        # Example input file
├── Makefile            # Build configuration
└── README.md           # This file
```

## Building

To compile the project, simply run:

```bash
make
```

This will produce an executable named `scheduler` in the current directory.

To clean up compiled objects and executables:

```bash
make clean
```

## Usage

### Basic Syntax
```bash
./scheduler <algorithm> <input_file>
```

### Algorithms
- `rate`: Rate-monotonic scheduling
- `edf`: Earliest-deadline-first scheduling

### Example
```bash
./scheduler rate tests/voo.txt
./scheduler edf tests/voo.txt
```

## Input File Format

The input file must contain:
- **Line 1**: Total simulation time (integer > 0)
- **Following lines**: One task per line with format:
  ```
  NAME PERIOD DEADLINE BURST
  ```

Where:
- `NAME`: Task identifier (string)
- `PERIOD` (P): Time between task arrivals
- `DEADLINE` (D): Maximum time allowed from arrival to completion
- `BURST` (C): CPU time required

**Constraint**: C ≤ D ≤ P (must be satisfied)

### Example (voo.txt)
```
100
ATT 20 12 8
NAV 50 30 15
```

This defines:
- Simulation for 100 time units
- ATT: arrives every 20 units, needs 8 units of CPU, must complete within 12 units
- NAV: arrives every 50 units, needs 15 units of CPU, must complete within 30 units

## Output Format

The program generates an output file named `<algorithm>_<login>.out` containing:

1. **Execution trace**: Shows each task execution or idle period
   - `[TASK_NAME] for X units - F`: Task finished
   - `[TASK_NAME] for X units - H`: Task still has work pending
   - `idle for X units`: CPU was idle

2. **Statistics**:
   - **LOST DEADLINES**: Count of missed deadlines per task
   - **COMPLETE EXECUTION**: Count of successfully completed instances per task
   - **KILLED**: Count of incomplete tasks when simulation ends

## Error Handling

The program validates input and reports errors to stderr:
- Incorrect number of arguments
- Invalid algorithm (must be 'rate' or 'edf')
- Input file not found or unreadable
- Malformed input (missing fields, non-numeric values)
- Invalid constraints (C > D or D > P)
- Non-positive values

## Testing

A test file is provided in `tests/voo.txt`. Run:

```bash
./scheduler rate tests/voo.txt
./scheduler edf tests/voo.txt
```

Then examine `rate_mla.out` and `edf_mla.out` to see the results.

## Implementation Notes

- All tasks arrive at time 0
- Preemption is allowed: a higher-priority task arriving will interrupt the current task
- Deadline misses cause the current instance to be discarded; the next instance arrives as scheduled
- Tie-breaking: when priorities are equal, the task appearing first in the input file wins
- Time advances in 1-unit increments

## System Requirements

- GCC compiler (or compatible C99 compiler)
- Linux-based operating system
- Standard C library

## Author Notes

This implementation follows the project specification for Operating Systems course.
The scheduler simulation supports both preemptive algorithms with proper deadline and instance tracking.
