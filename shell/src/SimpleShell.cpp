/**
 * Base header file and functionality provided by professor
 * Functions implemented by the student will have *Student* tagged in the comments
 * Noah Gumm
 * 02/28/2025
*/

#include "../include/SimpleShell.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


using namespace std;

void SimpleShell::execute(const vector<string>& argv)
{
    int status;
    pid_t child;

    /*
        Implement the following shell commands:
        /bin/date
        cd /usr 

        mkdir directory_name
        cat file_name
        ~ reference home directory
    */

    //We need to handle changing the current working directory with cd in the parent process, ie no fork needed
    //This is because we want the children to inherit the new working directory
    //Check if first argument is cd
    if (argv[0] == "cd") {
        if (argv.size() < 2) {
            std::cout << "CD was not given an argument" << std::endl;
        } else {
            std::string path = argv[1];
    
            // Check if the path starts with ~ and replace it with the home directory
            if (path[0] == '~') {
                const char* home = getenv("HOME");  // Get the home directory
                if (home != nullptr) {
                    path.replace(0, 1, home);  // Replace the ~ with the actual home directory
                } else {
                    std::cerr << "Error: HOME environment variable not set" << std::endl;
                    return;
                }
            }
    
            // Change to the target directory
            if (chdir(path.c_str()) != 0) {
                perror("Error");
                return;
            }
        }
        return;  // Skip forking for the cd command
    }


    // Spawning a child process
    child = fork();

    // Parent process portion
    if (child > 0) {
        cout << "(" << getpid() << ") : I am a parent process waiting..." << endl;
        waitpid(child, &status, 0); // Wait for child process to finish
        cout << "Waiting complete" << endl;
    }
    // Child process portion
    else if (child == 0) {
        cout << "I am a child executing a new environment" << endl;

        // Ensure there's at least one argument
        if (argv.empty()) {
            cerr << "Error: No command to execute" << endl;
            _exit(1);
        }

        // Prepare arguments for exec (must end with nullptr)
        vector<const char*> args;
        for (const auto& arg : argv) {
            args.push_back(arg.c_str());
        }
        args.push_back(nullptr);

        // Handle commands here
        if(argv[0] == "pwd") std::cout << GetWorkingDirectory() << std::endl; //PWD (Print working directory)
        else if(argv[0] == "ls") List_Directory(argv); //ls -F <directory> (List contents of directory)
        else if(argv[0] == "mkdir") MakeDirectory(argv);
        else if (argv[0] == "cat") Concatenate(argv);
        else if (argv[0] == "/date" || argv[0] == "/bin/date") PrintDate();
        else std::cout << "Invalid command" << std::endl; //Handle unknown input
    
    }
    else {
        perror("fork failed"); // Error handling if fork fails
        exit(1);
    }
}

void SimpleShell::parse(const string& line, vector<string>& tokens, const string& delimiter)
{
    size_t start = 0;
    size_t end = 0;
    
    while ((end = line.find(delimiter, start)) != string::npos) {
        if (end != start) { // Ignore empty tokens
            tokens.push_back(line.substr(start, end - start));
        }
        start = end + delimiter.length();
    }
    
    if (start < line.length()) { // Add the last token
        tokens.push_back(line.substr(start));
    }
}

void SimpleShell::run()
{
    while (true) {
        string line;
        vector<string> tokens;

        // Print the prompt
        cout << "(" << getpid() << ") % ";

        // Get input from the keyboard
        if (!getline(cin, line)) {
            break; // Exit the shell if input fails (e.g., EOF)
        }

        // Parse the input into tokens
        parse(line, tokens, " ");

        if (tokens.empty()) {
            continue; // Skip empty input
        }

        // Execute the user command
        execute(tokens);
    }
}

/*
 *Student implemented functions below here
*/

//*Student* Get working directory
std::string SimpleShell::GetWorkingDirectory(){
    //Create a buffer for the directory
    //Assume it will be no larger than 1024 characters
    char buf[1024];
    if(getcwd(buf, sizeof(buf)) != nullptr){
        //Return current working directory as string
        return std::string(buf);
    }
    else{
        //Print an error if not succesful
        perror("Error getting directory");
    }
    
    //If there is an error we return nothing
    return "";
}

void SimpleShell::List_Directory(const std::vector<std::string>& argv){
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        // Child process: Execute the system ls command
        // If -F is provided, append it to the ls command
        if (argv.size() == 1) {
            execlp("ls", "ls", nullptr);  // Execute ls (no arguments)
        } else if (argv.size() == 2 && argv[1] == "-F") {
            execlp("ls", "ls", "-F", nullptr);  // Execute ls -F
        } else if (argv.size() >= 2) {
            execlp("ls", "ls", argv[1].c_str(), nullptr);  // Execute ls <directory>
        }
        // If execlp fails, print an error and exit
        perror("execlp failed");
        _exit(1);
    } else {
        // Parent process: Wait for the child to finish
        int status;
        waitpid(pid, &status, 0);
    }
}

void SimpleShell::Concatenate(const std::vector<std::string>& argv){
    if (argv.size() < 2) {
        std::cerr << "Cat requires a file name as an argument" << std::endl;
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) {
        // Child process: Execute the system cat command
        execlp("cat", "cat", argv[1].c_str(), nullptr);
        perror("execlp failed"); // If execlp fails, print an error
        _exit(1);
    } else {
        // Parent process: Wait for the child to finish
        int status;
        waitpid(pid, &status, 0);
    }
}

void SimpleShell::MakeDirectory(const std::vector<std::string>& argv){
    if(argv.size() < 2){
        std::cerr << "mkdir needs a directory name" << std::endl;
    }

    //Default file permissions
    mode_t mode = 0755;

    if(mkdir(argv[1].c_str(), mode) == -1){
        perror("Can not create directory");
    } else {
        std::cout << "Directory created" << std::endl;
        return;
    }
}

void SimpleShell::PrintDate(){
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
