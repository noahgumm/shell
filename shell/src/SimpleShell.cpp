/**
 * Base header file and functionality provided by Dr. Essa Imhmed
 * Noah Gumm
 * 03/29/2025
*/

#include "../include/SimpleShell.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

void SimpleShell::execute(const vector<string>& argv) 
{
    int status;
    pid_t child;
    int inputFile = -1, outputFile = -1; // File descriptors for input/output redirection
    vector<string> commandArgs; // Stores actual command and arguments

    // Handle 'cd' separately in the parent process
    if (argv[0] == "cd") 
    {
        if (argv.size() < 2) 
        {
            cerr << "cd: missing argument" << endl;
        } 
        else 
        {
            // Expand '~' to home directory if used
            string path = (argv[1][0] == '~') ? getenv("HOME") + argv[1].substr(1) : argv[1];
            if (chdir(path.c_str()) != 0) 
            {
                perror("cd error");
            }
        }

        return; // No need to fork for 'cd'
    }

    // Check for I/O redirection symbols and set file descriptors accordingly
    for (size_t i = 0; i < argv.size(); i++) 
    {
        // Make sure that there is an argument following <
        if (argv[i] == "<" && i + 1 < argv.size()) 
        {
            // Open the file provided as read only
            inputFile = open(argv[i + 1].c_str(), O_RDONLY);
            // Failed to open file
            if (inputFile == -1) 
            {
                perror("input redirection failed");
                return;
            }
            i++; // Skip filename
        } 
        else if (argv[i] == ">" && i + 1 < argv.size()) 
        {
            // Open a file with write permissions, if it doesn't exist create it, if it does then overwrite it
            outputFile = open(argv[i + 1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

            // Failed to open file
            if (outputFile == -1) 
            {
                perror("output redirection failed");
                return;
            }

            i++; // Skip filename
        } 
        else 
        {
            commandArgs.push_back(argv[i]); // Add regular arguments to command
        }
    }

    // Missing args
    if (commandArgs.empty()) 
    { 
        cerr << "Error: No command given" << endl;
        return;
    }

    child = fork(); // Create a child process

    if (child > 0) 
    { 
        // Parent process
        waitpid(child, &status, 0); // Wait for the child process to complete      
    } 
    else if (child == 0) 
    { 
        // Child process
        // Redirect input if specified
        if (inputFile != -1) 
        {
            dup2(inputFile, STDIN_FILENO); // Redirect standard input to read from specified file
            close(inputFile); 
        }

        // Redirect output if specified
        if (outputFile != -1) 
        {
            dup2(outputFile, STDOUT_FILENO); // Redirect output to specified file
            close(outputFile);
        }

        // Handle commands 
        if (commandArgs[0] == "pwd") 
        {   
            //Print the working directory to the user
            cout << GetWorkingDirectory() << endl;
        } 
        else if (commandArgs[0] == "ls") 
        {
            // Use execlp to execute system 'ls'
            execlp("ls", "ls", nullptr);
            perror("ls failed");

        } 
        else if (commandArgs[0] == "mkdir") 
        {
            MakeDirectory(commandArgs); 
        } 
        else if (commandArgs[0] == "cat") 
        {
            Concatenate(commandArgs);
        } 
        else if (commandArgs[0] == "/bin/date" || commandArgs[0] == "/date") 
        {
            PrintDate();
        } 
        else 
        {
            cerr << "Invalid command" << endl;
        }

        _exit(0); // Exit the child process after execution
    } 
    else 
    {
        perror("fork failed"); // If fork fails
        exit(1);
    }
}

// Turn user input into tokens 
void SimpleShell::parse(const string& line, vector<string>& tokens, const string& delimiter)
{
    size_t start = 0;
    size_t end = 0;
    
    // Break up input based on the delimiter, in our case it will be " "
    while ((end = line.find(delimiter, start)) != string::npos) 
    {
        if (end != start) 
        { 
            // Ignore empty tokens
            tokens.push_back(line.substr(start, end - start));
        }
        start = end + delimiter.length();
    }
    
    if (start < line.length()) 
    { 
        // Add the last token
        tokens.push_back(line.substr(start));
    }
}

void SimpleShell::run()
{
    while (true) 
    {
        string line;
        vector<string> tokens;

        // Print the prompt
        cout << "(" << getpid() << ") % ";

        // Get input from the keyboard
        if (!getline(cin, line)) 
        {
            break; // Exit the shell if input fails (e.g., EOF)
        }

        // Parse the input into tokens
        parse(line, tokens, " ");

        if (tokens.empty()) 
        {
            continue; // Skip empty input
        }

        // Execute the user command
        execute(tokens);
    }
}

std::string SimpleShell::GetWorkingDirectory()
{
    //Create a buffer for the directory
    //Assume it will be no larger than 1024 characters
    char buf[1024];
    if(getcwd(buf, sizeof(buf)) != nullptr)
    {
        //Return current working directory as string
        return std::string(buf);
    }
    else
    {
        //Print an error if not succesful
        perror("Error getting directory");
    }
    
    //If there is an error we return nothing
    return "";
}

void SimpleShell::List_Directory(const std::vector<std::string>& argv)
{
    // Create a child process
    pid_t pid = fork();

    if (pid < 0) 
    {
        perror("fork failed");
        return;
    } 
    else if (pid == 0) 
    {
        // Child process: Execute the system ls command
        // If -F is provided, append it to the ls command
        if (argv.size() == 1) 
        {
            execlp("ls", "ls", nullptr);  // Execute ls (no arguments)
        } 
        else if (argv.size() == 2 && argv[1] == "-F") 
        {
            execlp("ls", "ls", "-F", nullptr);  // Execute ls -F
        } 
        else if (argv.size() >= 2) 
        {
            execlp("ls", "ls", argv[1].c_str(), nullptr);  // Execute ls <directory>
        }

        // If execlp fails, print an error and exit
        perror("execlp failed");
        _exit(1);
    } 
    else 
    {
        // Parent process: Wait for the child to finish
        int status;
        waitpid(pid, &status, 0);
    }
}

void SimpleShell::Concatenate(const std::vector<std::string>& argv)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        // Execute the system cat command
        execlp("cat", "cat", argv[1].c_str(), nullptr);
        // If execlp fails, print an error
        perror("execlp failed"); 
        _exit(1);
    } else {
        // Parent process: Wait for the child to finish
        int status;
        waitpid(pid, &status, 0);
    }
}

void SimpleShell::MakeDirectory(const std::vector<std::string>& argv)
{
    // Missing args
    if(argv.size() < 2)
    {
        std::cerr << "mkdir needs a directory name" << std::endl;
    }

    //Default file permissions
    mode_t mode = 0755;

    if(mkdir(argv[1].c_str(), mode) == -1)
    {
        perror("Can not create directory");
    } 
    else 
    {
        std::cout << "Directory created" << std::endl;
        return;
    }
}

void SimpleShell::PrintDate()
{
    time_t now = time(nullptr);  // Get the current time
    struct tm* localTime = localtime(&now); // Convert to local time

    // Format and print the date & time
    std::cout << (localTime->tm_year + 1900) << "-"
              << (localTime->tm_mon + 1) << "-"
              << localTime->tm_mday << " "
              << localTime->tm_hour << ":"
              << localTime->tm_min << ":"
              << localTime->tm_sec
              << std::endl;
}

int main()
{
    SimpleShell shell;
    shell.run(); // Start the shell loop
    return 0;
}
