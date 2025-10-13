# PROC - Process Management, Scheduling, and Communication

Process is an application that is being executed. In atOS-RT, a process is represented by a Task Control Block (TCB) structure that contains all the necessary information for managing and scheduling the process.

## In-Depth Explanation

Each process has its own TCB, which includes:
- **U32 Process ID (PID)**: A unique identifier for the process.
- **U32 *Stack Pointer (ESP)**: Points to the current position in the process's stack.
- **U32 State**: Indicates whether the process is running, ready, or blocked.
- **U32 Focus**: Focus state of the process (foreground or background).
- **U32 Priority**: Determines the scheduling priority of the process.
- **U8 Name[32]**: The name of the process.
- **Message Queue**: A queue for inter-process communication (IPC).
- **Other fields...**: See full type definition in PROC.h.

### Talking to process manager
The process manager is responsible for managing process execution.

When creating a new process, the process manager needs to know:
- If you need a framebuffer (for graphical applications).

### Process Creation and Management
Processes can be created using the `PROC_RUN` and `PROC_RUNW` for extended options. The `PROC_EXIT` function allows a process to terminate itself, while `PROC_KILL` can be used to terminate another process by its PID.

### Scheduling

Each process has its own framebuffer if it needs one. 

By pressing ALT+(0-9), A different shell is put into focus that handles its own child processes.

By pressing ALT+TAB, the shell changes focus to the next gui process.

By pressing ALT+CTRL+BACKSPACE, the current shell and all its child processes are killed and a new shell is started.

Processes can communicate with each other using message queues. The `PROC_SEND` function allows a process
to send a message to another process, while `PROC_RECEIVE` allows a process to receive messages from its queue.

When a process wants to exit, it should call `PROC_EXIT`, which will clean up resources and notify the shell process if necessary.

When coding a process, it needs to tell kernel if it:
- needs a framebuffer (for graphical applications)
- wants to be informed of certain events (keyboard, mouse, etc)

Kernel and shell are tied together by. The shell is basically a process, but it is immortal and handles child processes. 

If a new process is created, the shell is informed of it. If a process dies, the shell is informed of it. The shell can then update its list of processes.

Processes are scheduled using a simple round-robin algorithm. Each process is given a time slice to run, and when the time slice expires, the process is moved to the back of the queue and the next process is scheduled.

