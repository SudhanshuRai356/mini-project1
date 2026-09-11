# Mini Project 1: C-Shell & xv6 MLFQ

## Overview
This repository contains the complete implementation for Mini Project 1, divided into two major components: a custom POSIX-compliant C-Shell and a Multi-Level Feedback Queue (MLFQ) scheduler integrated into the xv6 operating system kernel. 

The codebase is heavily modularized into separate header and source files to ensure clean logic separation and avoid monolithic structures.
## Assumptions
1. Background execution is set up such that when we get the output result, it will not print out directly and mess up the terminal. It waits patiently like in bash, and will only announce that the process has ended after the code put in the current prompt is done running or if you just hit enter.
2. Custom built-in commands will not be run with snoop.
3. For xv6, instead of implementing an actual queue data structure, I used modulo rotating pointers to manage the processes.

***

## Part 1: C-Shell Implementation Details

### Lexer and Parser
*   **`lexer` (`lexer.c`):** Reads the raw input line and produces tokens while ignoring spaces and tabs. It handles quoting and escape characters using manual loop blocks to make sure special operators lose their meaning inside quotes.
*   **`parser` (`parser.c`):** Because who wants to be opening the question page again and again, it explicitly follows the right linear grammar from the spec to parse tokens into an array of command nodes, handling pipes, semicolons, and background symbols.

### Custom Functions Explained

**Main Core and Initialization (`main.c`)**
*   **`init_shell()`**: Sets up the shell environment, gets the user and hostname, claims the terminal PGID using `tcsetpgrp`, and sets up signal handlers for signals like `SIGCHLD`, `SIGALRM`, and ignoring `SIGINT`, `SIGTSTP`, `SIGTTOU`, and `SIGQUIT`.
*   **`getpwd()`**: Fetches the current working directory and replaces the home directory prefix with a tilde.
*   **`process_cmd()`**: The main dispatcher function that tokenizes, parses, checks for redirections and pipes, and runs the appropriate built-in or external command.
*   **`main()`**: The core REPL loop that continuously prompts the user, reads input using `fgets`, handles Ctrl-D, and invokes `process_cmd()`.
*   **`prevdir()`**: Initializes the `prev` path string buffer for the `hop -` command.
*   **`str_replace()`**: Replaces occurrences of a target substring within an old string using temporary calloc allocations.

**Job Control and Signals (`main.c`)**
*   **`assign_bg()` / `assign_stopped()` / `assign_bg_multi()` / `assign_stopped_multi()`**: Adds single or multi-process jobs to the global `bg_list` array, assigning job IDs, PIDs, group IDs, and full command arguments.
*   **`announce_bg()`**: Prints whether a background process exited normally or abnormally, cleaning up its memory from the job list.
*   **`alarm_handler()`**: A dummy signal handler for `SIGALRM` so the program doesn't die when a timeout hits.
*   **`no_longer_waiting()`**: Blocks `SIGCHLD` momentarily and flushes out all completed background jobs accumulated in the wait list so they print cleanly.
*   **`plant()`**: The `SIGCHLD` signal handler that uses `waitpid` with `WNOHANG | WUNTRACED | WCONTINUED` to reap zombie processes and track stopped or continued states.
*   **`ctrld1()` / `ctrld2()`**: Check if any jobs are stopped when Ctrl-D is pressed, printing a warning or sending `SIGHUP` to all process groups before exiting.

**Built-in Commands (`bins.c`)**
*   **`lookup()`**: Maps system call numbers to their string names for the snoop command summary table, falling back to `syscall_N` for unknowns.
*   **`search()`**: Reads the `.frecency` file, calculates rank using frequency and recency time-decay, and finds matching directory substrings.
*   **`update()`**: Updates or adds directory entries and scores into the `.frecency` persistence file.
*   **`hop()`**: Changes the shell working directory, handling ~, -, .., ., and frecency lookups when direct paths fail.
*   **`reveal()`**: Lists directory contents alphabetically using `scandir` and `alphasort`, supporting recursive (`-t`) and hidden (`-a`) flags.
*   **`locate()`**: Searches through the system `PATH` and current working directory to find executable paths matching the argument.
*   **`peek()`**: Concatenates file contents or standard input, supporting line numbering (`-n`) and reverse chunked reading (`-r`) via `lseek`.
*   **`activities()`**: Prints all active background jobs and process groups tracked by the shell along with their running or stopped states.
*   **`resume()`**: Continues a stopped or backgrounded job in the `fg` or `bg`, supporting an optional `--timeout` flag with `alarm()`.
*   **`ping()`**: Sends a specified signal (modulo 64) to a target process ID or job number.
*   **`spy()`**: Inspects `/proc/[pid]/` symlinks (`cwd`, `exe`), maps file regions, and lists open file descriptors with their file types.
*   **`snoop_sigint()`**: Signal handler to stop the `snoop` trace loop gracefully on Ctrl-C.
*   **`snoop()`**: Attaches to a running process or forks a new one under `ptrace`, tracking syscall entries and exits to print a summarized timing table.

**Execution and Redirection (`command.c`)**
*   **`builtin()` / `exec_builtin()`**: Checks if a command is a shell intrinsic and executes it directly.
*   **`run_cmd()`**: Handles executing external binaries using `execv` or `execvp`, checking for local paths or `%` overrides.
*   **`redir_in()` / `redir_out()`**: Sets up input and output file redirections using `open()`, pipes, and `dup2()`.
*   **`ext_in()` / `ext_out()`**: Strips redirection operators and filenames out of the argument array.
*   **`piped()`**: Sets up multi-command pipelines by creating pipes, forking children, setting process groups, connecting file descriptors, and waiting.

### System Calls Used
Here are the exact syscalls used in the codebase alongside the notes and comments left during development:

*   **`fork()`**: Used everywhere to create child processes for background execution, redirections, and pipelines.
*   **`waitpid()`**: Used to wait for child process state changes.
    * to check for stop we need to use untraced and not 0 otherwise your code will not detect ctrl z even though you think it should ....
    * fixed one thing found another issue trying to fix that now
*   **`execv()`, `execvp()`**: Used to launch external binaries.
    * to differentiate from normal command errors 
*   **`pipe()`**: Used to create communication channels between piped commands.
    * no error message specified for this but need to put it here still so this is it
*   **`dup()`, `dup2()`**: Used for duplicating file descriptors during file redirection and piping.
    *  redireting input reader to the open file fd
*   **`open()`, `close()`**: Used for file operations and managing file descriptors.
    *  the flags for write only, create a file if does not exist and the flag to append so that we are in >> mode
*   **`kill()`, `killpg()`**: Used for sending signals to processes or process groups.
    *  i just wanna say the function name kinda ironic made me read unix history` (on `killpg(SIGCONT)`).
    * this extra check because from my previous deep dive on kill i know that the only time it will return an neg value is for either process not found or i do not have persmission to read`
*   **`signal()`, `sigaction()`**: Used for handling signals.
    * sigaction api boiler plate apparently previously they use to use signal function thats where the header name comes from
    * sa2 part is llm generated code but essentially what it is doing is it is attaching a function for internal signals which is alarm handler here so that when we get the sigalrm signal we can actually take it and handle it instead of just dying off
*   **`alarm()`**: Used for timeout handling in `resume`.
    * apparently the alarm just kills the process it is called from so i need to just be able to accept it here even if this does nothing
    * welp alarm does not take longlong and i am not changing the above long long
    * needed other wise even with an interrupt it just rings an alarm out of nowhere in some other process
*   **`lseek()`**: Used for chunked reverse file reading in `peek`.
    * lseek returns the offset of the file descriptor and SEEK_END is a macro to seek to the end of the file
*   **`stat()`**: Used to check if paths are directories or regular files.
    * directory check is first since opening a directory also gives null, then the errors output don't need to have one saying not file or dir idk
*   **`access()`**: Used to verify executable permissions.
    * saw that the file needs to be executable got his boiler plate code so access is a function which takes the string and checks if the file is executable or not, also stat with S_ISREG is a macro to check if the file is a regular file or not
*   **`readlink()`**: Used to read symbolic links in `/proc` for `spy`.
    * since the values in the proc folder are symlinks can't be reading them normally
*   **`ptrace()`**: Used to trace syscalls in `snoop`.
    * attach ptrace to a stopped process and it will keep hanging need to make my function start it again so it works as resume too
    *  now this is where the actual tracing starts tracesysgood just puts a flag that lets the ptrace know if the program has been blocked or been put on hold due to some other syscall//it asks for a void type hence had to typecast
*   **`clock_gettime()`**: Used to measure execution time in `snoop`.
    *  this is for the the amount of time ran thingy
*   **`setpgid()`, `tcsetpgrp()`**: Used for job control and terminal foreground/background switching.
    * setting up the group pid so that we can implement it in the activites part

***

## Part 2: xv6 MLFQ Scheduler

The second half of the project implements a Multi-Level Feedback Queue replacing the default Round Robin scheduler in xv6.

### Architectural Changes
*   **`proc.h` & `proc.c`:** The `proc` structure was modified to include `queue` level, `inq` (in-queue status), and `tick` (time consumed in the current slice). 
*   **Queues:** Four priority queues (0 to 3) were implemented using rotating modulo pointers for efficiency. Time slices are distributed exponentially (1, 4, 8, 16 ticks).
*   **Syscalls:** Added `sys_getqueue` to allow user processes to query their current queue level.

### Scheduling Rules Implemented
1.  **Rule 1 & 2 (Priority):** The scheduler strictly selects the runnable process from the highest priority non-empty queue. 
2.  **Rule 3 (New Jobs):** Processes are always pushed to the tail of Queue 0 upon creation.
3.  **Rule 4 (Demotion):** If a process consumes its full allotted time slice for its current queue, it is preempted and pushed to the next lower queue (or back into Queue 3 if it is already at the bottom).
4.  **Rule 5 (Yielding):** If a process yields the CPU voluntarily before its time slice expires (e.g., waiting for I/O), it retains its priority level and is pushed to the head of its current queue when it becomes runnable again.
5.  **Priority Boost:** To prevent starvation of CPU-bound processes, a global priority boost is triggered every 48 ticks, moving all processes back to Queue 0.

### Debugging & Visualization
*   The `procdump` function (triggered via `Ctrl+P`) was expanded to print the current queue and tick consumption for each process.
*   A custom user-space binary, `schedulertest`, was written to spawn processes that simulate varying CPU-bound and I/O-bound workloads. The results were logged to evaluate turnaround time, waiting time, and response time across FCFS, Round Robin, and MLFQ.