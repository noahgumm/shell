#include "../include/SimpleShell.h"
#include <iostream>

using namespace std;

void SimpleShell::execute(const vector<string>& argv)
{
    int status;
    pid_t child;

    //Print working directory before user proviedes input
    //May remove later, useful for testing
    PrintWorkingDirectory();

    /*
        Implement the following shell commands:
        /bin/date
        cd /usr
        pwd

        mkdir directory_name
        ls -F
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

        string exec_path = "/bin/" + argv[0];

        // Prepare arguments for exec (must end with nullptr)
        vector<const char*> args;
        for (const auto& arg : argv) {
            args.push_back(arg.c_str());
        }
        args.push_back(nullptr);

        // Execute the ls command. You need to change it to satisfy the assignment requirements
        if (execl("/bin/ls", "ls", "/", (char *)0) == -1)
        {
            perror("execl failed"); // Print error message
            _exit(1);
        }
    }
    else {
        perror("fork failed"); // Error handling if fork fails
        exit(1);
    }
}

//Function to print working directory
void SimpleShell::PrintWorkingDirectory(){
    //Create a buffer for the directory
    //Assume it will be no larger than 1024 characters
    char buf[1024];
    if(getcwd(buf, sizeof(buf)) != nullptr){
        //Print current working directory to the user
        cout << buf << endl;
    }
    else{
        //Print an error if not succesful
        perror("Error");
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

int main()
{
    SimpleShell shell;
    shell.run(); // Start the shell loop
    return 0;
}
