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
    if(argv[0] == "cd") {
    
        //If size of argv is less than two than the user did not provide a directory after the cd command
        if(argv.size() < 2){
            std::cout << "CD was not given an argument" << std::endl;
        }
        else{
            std::string path = (argv[1][0] == '~') ? getenv("HOME") : argv[1];

            //If chdir() does not return successful(1) then print an error to the user
            if(chdir(path.c_str()) != 0){
                perror("Error");
                return;
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
    int size = argv.size();
    std::string path = GetWorkingDirectory(); //Default to current directory for file listing
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
                perror("stat error");
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

void SimpleShell::Concatenate(const std::vector<std::string>& argv){
    //Check to ensure user provided an argument along with cat
    if(argv.size() < 2){
        std::cerr << "Cat requires a file name as an argument" << std::endl;
    } else {
        //Open returns an integer based on the files descriptor
        //If open fails then it returns -1
        int file = open(argv[1].c_str(), O_RDONLY);

        if(file < 0){
            perror("Sys Error: Cat failed");
        } else {
            //Read the file in pieces and send to std out
            char buffer[1024];
            ssize_t contents;

            while((contents = read(file, buffer, sizeof(buffer))) > 0){
                write(STDOUT_FILENO, buffer, contents);
            }

            close(file);
        }
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
