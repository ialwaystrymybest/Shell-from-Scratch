#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <unistd.h>    //Provides access() function.
#include <sys/types.h>
#include <sys/wait.h>


// Helper to check if a file exists AND is executable
bool executable(const std::string &path)
{
    return access(path.c_str(), X_OK) == 0;            // X_OK checks for executable permission
}

// Search PATH for the command
std::string find_in_path(const std::string &cmd)
{
    const char* env_path = std::getenv("PATH");
    if(!env_path) return "";

    std::stringstream ss(env_path);
    std::string dir;

    // Split PATH by ':' and check each folder
    while(std::getline(ss, dir, ':'))
    {
        std::string full_path = dir + "/" + cmd;

        if(executable(full_path))
        {
            return full_path;
        }
    }
    return "";   //Not found
}

void run_external_prog(const std::string &command, const std::string &parameters)
{
    std::vector<std::string> args;
    args.push_back(command);
    
    if(!parameters.empty()){
        std::stringstream ss(parameters);
        std::string arg;
        while(ss >> arg){
            args.push_back(arg);
        }
    }

    std::vector<char*> c_args;
    for(const auto &a: args){
        c_args.push_back(const_cast<char*>(a.c_str()));
    }
    c_args.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0){
        execvp(command.c_str(), c_args.data());

        exit(1);
    }
    else if(pid > 0){

        int status;
        waitpid(pid, &status, 0);
    }


}

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while(true)
    {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);

        std::string command = input.substr(0, input.find(" "));
        std::string parameters;

        int parameterIndex = input.find(" ") + 1;
        if(parameterIndex != std::string::npos)
        {
            parameters = input.substr(parameterIndex);
        }

        if(command == "exit")
        {
            break;
        }

        if(command == "echo")
        {
            std::cout << parameters << std::endl;
        }
        else if(command == "type")
        {
            if(parameters == "echo" || parameters == "exit" || parameters == "type")
            {
                std::cout << parameters << " is a shell builtin" << std::endl;
            }
            else
            {
                std::string path = find_in_path(parameters);
                if(!path.empty())
                {
                    std::cout << parameters << " is " << path << std::endl;
                }
                else
                {
                    std::cout << parameters << ": not found" << std::endl;
                }
            }
        }
        else
        {
            std::string path = find_in_path(command);
            if(!path.empty())
            {
                run_external_prog(command, parameters);
            }
            else
            {
                std::cout << input << ": command not found" << std::endl;
            }
        }
    }
}
