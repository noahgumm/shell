#ifndef SIMPLE_SHELL_H
#define SIMPLE_SHELL_H

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Doxygen comment
 * @brief A simple shell class that handles basic shell operations such as parsing and executing commands.
 */
class SimpleShell {
public:
    /**
     * @brief Starts the shell loop.
     * 
     * This method runs the shell's main loop, taking user input, parsing it, and executing commands.
     */
    void run();

private:
    /**
     * @brief Executes a command using fork and execv.
     * 
     * This function spawns a child process that replaces its image with the command
     * specified by the user. The parent process waits for the child process to finish.
     * 
     * @param argv A vector containing the command and its arguments to be executed.
     */
    void execute(const std::vector<std::string>& argv);

    /**
     * @brief Parses a line of input into tokens based on a delimiter.
     * 
     * This function takes a string and splits it into tokens, storing each token
     * into a vector. The delimiter is used to identify where the string should be split.
     * 
     * @param line The input string to be tokenized.
     * @param tokens A reference to a vector that will hold the parsed tokens.
     * @param delimiter The delimiter used to split the input string.
     */
    void parse(const std::string& line, std::vector<std::string>& tokens, const std::string& delimiter);
};

#endif // SIMPLE_SHELL_H
