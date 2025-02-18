# Simple Shell Program

This is a simple shell program written in C++ as part of an educational exercise. The program implements basic shell functionality such as reading commands from the user, parsing the input, and executing commands by creating child processes. However, it purposely lacks advanced shell features and is left incomplete as an assignment for students to further implement.

## Files Included

- `SimpleShell.h`: Header file containing the definition of the `SimpleShell` class and its methods.
- `SimpleShell.cpp`: Source file containing the implementation of the `SimpleShell` class.
- `Makefile`: File used to compile and run the program via the `make` command.

## How to Compile and Run

```bash
make  # Compiles the program and then runs the executable
make compile  # Compiles the source code into an executable located in the bin/ directory
make run  # Executes the compiled program
make clean  # Deletes the object files and the executable from the bin/ directory
