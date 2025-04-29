# Simple Shell Program

This is a simple shell program written in C++ as part of an educational exercise. 
The program implements basic shell functionality such as reading commands from the user, parsing the input, and executing commands by creating child processes. 
However, it purposely lacks advanced shell features and is left incomplete as an assignment for students to further implement.

NOTE: We are to avoid using the exec() functions when possible. Attempt to use specific system calls.
NOTE 2: Code is a bit of a mess and could have used more structure or possible even be split into multiple files. But for the scale of the project one file seems to be fine.


Supported Commands:
- cd <path> (Change working directory.)
- pwd (print working directory.)
- ls <!optional>-F <!optional><directory_name> 
  (list the files in the provided directory, if no directory is given then it uses the current dirctory, -F appends file types to the end of file names.)
- mkdir <directory_name> (Make a new directory.)
- cat <file_name> (Print the contents of a file.)
- /date or /bin/date (Prints the current date.)
- < For For input redirection
- \> For output redirection
- wc to count words, lines, and characters
- | pipes to pass data to commands


## Files Included

- `SimpleShell.h`: Header file containing the definition of the `SimpleShell` class and its methods.
- `SimpleShell.cpp`: Source file containing the implementation of the `SimpleShell` class.
- `Makefile`: File used to compile and run the program via the `make` command.

## How to Compile and Run

You must be in the same directory as the makefile (the inner 'shell' directory) in order to run the makefile commands.
```bash
make  - Compiles the program and runs the executable
make compile  - Compiles the source code into an executable located in the bin/ directory
make run  - Executes the compiled program
make clean  - Deletes the object files and the executable from the bin/ directory
