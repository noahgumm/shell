/**
 * Base header file and functionality provided by professor
 * Functions implemented by the student will have *Student* tagged in the comments
 * Noah Gumm
 * 02/23/2025
*/

#include "../include/SimpleShell.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;

void SimpleShell::execute(const vector<string>& argv)
{
    int status;
    pid_t child;
    std::string currentWorkingDirectory;

    //*Student* Print working directory before user provides input
    //May remove later, useful for testing
    std::string currentDirectory = GetWorkingDirectory();
    std::cout << currentDirectory << std::endl;

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
    if(argv[0] == "cd") {
        
        //If size of argv is less than two than the user did not provide a directory after the cd command
        if(argv.size() < 2){
            std::cout << "CD was not given an argument" << std::endl;
        }
        else{
            //If chdir() does not return successful(1) then print an error to the user
            if(chdir(argv[1].c_str()) != 0){
                perror("Error");
            }

        }
        //Skip forking on this execute
        return;
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
        if(argv[0] == "pwd") std::cout << currentDirectory << std::endl; //PWD (Print working directory)
        else if(argv[0] == "ls") List_Directory(argv, currentWorkingDirectory); //ls -F <directory> (List contents of directory)
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
string SimpleShell::GetWorkingDirectory(){
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

void SimpleShell::List_Directory(const vector<string>& argv, const string& currentDirectory){
    int size = argv.size();
    std::string path = currentDirectory; //Default to current directory for file listing
    bool showFileTypes = false;

    if (size == 2) { //Handle ls -F or ls <directory>
        //If the second command isn't -F we know it is a directory
        if(argv[1] != "-F"){
            path = argv[1];
        } else {
            showFileTypes = true;
        }
    } else if (size == 3) { //Handle ls -F <directory>
            showFileTypes = true;
            path = argv[2];
    } else if (size > 3) {
        std::cerr << "Command 'ls' was given an invalid number of arguments" << std::endl;
    }

    //Attempt to open the directory
    DIR* dir = opendir(path.c_str());
    if(!dir){
        perror("Error opening directory to list");
        return;
    }

    //Structures fore storing directories and file stats
    struct dirent* directoryEntry;
    struct stat fileStat;

    //Read all directories
    while((directoryEntry = readdir(dir)) != nullptr){
        std::string fileName = directoryEntry->d_name;
        std::string fullPath = path + "/" + fileName;
        std::string marker = "";

        if(showFileTypes){
            //Attempt to get the file stats
            if (stat(fullPath.c_str(), &fileStat) == -1) {
                perror("stat failed");
                continue;
            }

            // Determine file type and set marker to appropriate symbol
            if (S_ISDIR(fileStat.st_mode)) marker = "/"; //Directory
            else if (S_ISREG(fileStat.st_mode) && (fileStat.st_mode & S_IXUSR)) marker = "*";  // Executable
            else if (S_ISLNK(fileStat.st_mode)) marker = "@";  // Symlink
            else if (S_ISFIFO(fileStat.st_mode)) marker = "|";  // Pipe
            else if (S_ISSOCK(fileStat.st_mode)) marker = "=";  // Socket
        }

        //Handle printing the file and its type
        std::cout << fileName << marker << std::endl;
    }

    closedir(dir);
}

int main()
{
    SimpleShell shell;
    shell.run(); // Start the shell loop
    return 0;
}
