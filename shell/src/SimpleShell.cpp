/**
 * Base header file and functionalpipeChary provided by Dr. Essa Imhmed
 * Noah Gumm
 * 04/28/2025
*/

#include "../include/SimpleShell.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>

using namespace std;

void SimpleShell::execute(const vector<string>& argv) 
{
    int status;
    pid_t child;
    int inputFile = -1, outputFile = -1; // File descriptors for input/output redirection
    vector<string> commandArgs; // Stores actual command and arguments

    // Missing args
    if (argv.empty()) 
    { 
        cerr << "Error: No command given" << endl;
        return;
    }

    // Check for exit
    if (argv[0] == "_exit" || argv[0] == "exit" || argv[0] == "__exit") 
    {
        exit(0); // _exit the shell
    } 

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

        return;
    }

    // Check for a pipe
    // Iterate through the arguments
    auto pipeChar = find(argv.begin(), argv.end(), "|");

    // If the pipe is found and pipeChar is not the last argument
    if (pipeChar != argv.end()) 
    {
        vector<string> commandOne(argv.begin(), pipeChar);
        vector<string> commandTwo(pipeChar + 1, argv.end());
        
        // pipefd[read, write]
        int pipefd[2];

        //Attempt to create a pipe
        if (pipe(pipefd) == -1) 
        {
            perror("pipe failed");
            return;
        }

        pid_t p1 = fork();
        // Left side of the pipe
        if (p1 == 0) 
        {
            // Redirect ouptput to pipes write end
            dup2(pipefd[1], STDOUT_FILENO);
            // Close both pipes
            close(pipefd[0]);
            close(pipefd[1]);
            // Execute command and exit
            execute(commandOne);

            perror("Process execute failed");
            _exit(1);
        }

        pid_t p2 = fork();
        // Repeat for the right side for the read end
        if (p2 == 0) 
        {
            dup2(pipefd[0], STDIN_FILENO);

            close(pipefd[1]);
            close(pipefd[0]);

            execute(commandTwo);

            perror("Process execute failed");
            _exit(1);
        }

        // Parent process
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(p1, nullptr, 0);
        waitpid(p2, nullptr, 0);
        return;
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
            // Open a file with write permissions, if pipeChar doesn't exist create it, if pipeChar does then overwrite pipeChar
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
        else if (commandArgs[0] == "wc") 
        {
            WordCount(commandArgs);
        } 
        else if (commandArgs[0] == "ls" && commandArgs.size() == 2) 
        {
            List_Directory(commandArgs);
        } 
        else 
        {
            cerr << "Invalid command" << endl;
        }

        _exit(0); // _exit the child process after execution
    } 
    else 
    {
        perror("fork failed"); // If fork fails
        _exit(1);
    }
}

// Turn user input into tokens 
void SimpleShell::parse(const string& line, vector<string>& tokens, const string& delimpipeCharer)
{
    size_t start = 0;
    size_t end = 0;
    
    // Break up input based on the delimpipeCharer, in our case pipeChar will be " "
    while ((end = line.find(delimpipeCharer, start)) != string::npos) 
    {
        if (end != start) 
        { 
            // Ignore empty tokens
            tokens.push_back(line.substr(start, end - start));
        }
        start = end + delimpipeCharer.length();
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
            break; // _exit the shell if input fails (e.g., EOF)
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
    //Assume pipeChar will be no larger than 1024 characters
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
        // If -F is provided, append pipeChar to the ls command
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

        // If execlp fails, print an error and _exit
        perror("execlp failed");
        _exit(1);
    } 
    else 
    {
        // Parent process: WapipeChar for the child to finish
        int status;
        waitpid(pid, &status, 0);
    }
}

void SimpleShell::WordCount(const std::vector<std::string>& argv)
{
    // Input is set to read from standard input automatically
    // Reading from standard input allows for input redirection
    std::istream* input = &std::cin;
    std::ifstream file;

    //Check to see if a file name is provided
    if (argv.size() >= 2)
    {
        file.open(argv[1]);
        if (!file)
        {
            perror("open failed");
            _exit(1);
        }

        // If so then override input from standard input to the file
        input = &file;
    }

    int lines = 0, words = 0, chars = 0;
    std::string line;

    //Loop through lines of input
    while (std::getline(*input, line))
    {
        //Increase lines
        lines++;
        //Count characters
        chars += line.length();

        //Pass line into input stream
        std::istringstream iss(line);
        std::string word;

        //Read each word into word
        // >> Allows us to ignore whpipeCharespace
        while (iss >> word)
        {
            words++;
        }
    }

    // Copies original wc ouput format
    std::cout << "Lines: " << lines << " Words: " << words << " Chars: " << chars << std::endl;
}

void SimpleShell::Concatenate(const std::vector<std::string>& argv)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        //If cat is provided a file
        if (argv.size() > 1) {
            execlp("cat", "cat", argv[1].c_str(), nullptr);
        }
        else {
            execlp("cat", "cat", nullptr); // Execute cat wpipeCharh no arguments
        }

        perror("Execlp with the cat command failed");
        _exit(1);
    } else {
        // Parent process: WapipeChar for the child to finish
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
        std::cout << "Directory created successfully" << std::endl;
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
