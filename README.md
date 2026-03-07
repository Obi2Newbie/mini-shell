# mini-shell

A simple shell implementation in C with support for built-in commands, external commands, background execution, piping, input/output redirection, and job management.

## Features

- Built-in commands: `cd`, `pwd`, `mkdir`, `jobs`, `exit`
- External command execution (prefix with `!`)
- Background job execution (suffix with `&`)
- Pipe support (`|`)
- Input/output redirection (`<`, `>`)
- Job listing and tracking
- Signal handling for child processes

## Usage

```bash
gcc -pthread -o mini-shell miniShell.c
./mini-shell
```

## Commands

- `cd [path]` - Change directory
- `pwd` - Print working directory
- `mkdir [name]` - Create directory
- `jobs` - List background jobs
- `exit` - Quit the shell
- `![command]` - Execute external command
- `[command] &` - Run in background
- `[command1] | [command2]` - Pipe commands
- `[command] > [file]` - Output redirection
- `[command] < [file]` - Input redirection
