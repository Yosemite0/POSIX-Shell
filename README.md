# Shell Project

## Authors

**Name**        - **Yash Chordia**

**Roll No.**    - **2024201029**

---

## Overview

This project is a simple command-line shell implementation written in C++. The shell supports basic commands and demonstrates the fundamentals of environment management within a shell environment.

## Directory Structure

```
/project
├── env/
│   ├── ShellEnv.cpp
│   ├── ShellEnv.h
├── execute.cpp
├── parse.cpp
├── input.cpp
├── signal.cpp
├── inbuilt.cpp
└── shell.cpp
```

- **env/ShellEnv.cpp**: Contains the implementation of the `ShellEnv` class, which handles the environment-related functionalities.
- **shell.cpp**: The main entry point for the shell, which interacts with the user and processes commands.

## Compilation

To compile the project, simply run:

```
make
```

This will compile the source files and produce an executable named `shell`.

## Running the Shell

Once compiled, you can run the shell with:

```bash
./shell
```

---

## Features

- **Dynamic Shell Prompt:**
  - Displays the current user name and system name dynamically.

- **Basic Commands and Directory Operations:**
  - `cd`: Change the current directory.
  - `echo`: Display text.
  - `pwd`: Print the working directory.
  - `ls`: List files and directories in the current directory.
  -  Support execution of system commands via `execv`

- **System Command Management:**
  - Supports running system commands with arguments, handling both background (`&`) and foreground processes.

- **Process Information and Search:**
  - `pinfo`: Displays information about the shell process.
  - `search`: Recursively searches for files or directories from the current directory.

- **I/O Redirection and Pipelines:**
  - Supports input (`<`), output (`>`), and append (`>>`) redirection.
  - Allows pipelines to pass output between commands and combines redirection with pipelines.

- **Signal Handling:**
  - `CTRL-Z`: Moves the foreground job to the background and stops it.
  - `CTRL-C`: Interrupts the foreground job with SIGINT.
  - `CTRL-D`: Logs out of the shell.

- **Autocomplete and Command History:**
  - Autocompletes filenames and directories in the current directory.
  - `history`: Displays command history with navigation using the up and down arrows.

---

## Cleaning the Project

To remove the compiled object files and executable, run:

```bash
make clean
```

This will delete the `*.o` files and the `shell` executable.

## Requirements

- **C++17** or later
- A compatible C++ compiler (e.g., `g++`)
- `make` utility
