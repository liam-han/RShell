#ifndef	__COMMAND_H__
#define __COMMAND_H__

#include <cstring>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <stack>

using namespace std;

class BaseCommand
{
protected:
    char** in; // stores command input by user
    vector<char*> commands; // vector variable used to allow easier parsing
    string output; // stores messages to be printed, usually error messages
    bool valid; /* determines whether or not current command is valid,
                   dependent usually on other commands */
    int type; /* Variable created to hold type of command. 0 indicates a
                 regular command, 1 indicates an or (||) command, 2 indicates
                 an and (&&) command */
    bool exitCalled; // Set to true only if exit is called
public:
    BaseCommand() {}
    // Parse prepares input to be executed in execute()
    virtual void parse(char* input) = 0;
    // Execute runs commands in input accordingly
    virtual void execute() = 0;
    // Exits shell with error of 1
    void exitShell()
    {
        exit(1);
    }
    // Prints output, which usually contains an error code
    void print()
    {
        cout << output << endl;
    }
    bool isValid() {return valid;}
    void setValid(bool val) {valid = val;}
    int getType() {return type;}
    void setType(int t) { type = t;}
    bool isExitCalled() {return exitCalled;}
    void setExitCalled(bool ec) {exitCalled = ec;}

};

class Command : public BaseCommand
{
public:
    Command() {}
    Command(char* input)
    {
        valid = true;
        type = 0;
        exitCalled = false;
        this->parse(input);
    }
    // Constructor if type is specific
    Command(char* input, int t)
    {
        valid = true;
        type = t;
        exitCalled = false;
        this->parse(input);
    }
    // Constructor if validity is specific
    Command(char* input, bool val)
    {
        valid = val;
        type = 0;
        exitCalled = false;
        this->parse(input);
    }
    void parse(char* input) 
    {
        // end token for single command, used to parse with strok
        char* end_sing;
        // Parses input, removing spaces
        char* token = strtok_r(input," ", &end_sing);
 
        
        while (token != NULL)
        {
            if(strchr(token, '\"') != NULL)
            {
                // Makes variable quote to store all of text within 
                // quotation marks. Makes copy of initial token, which is 
                // the word immediately after a quotation mark, removes 
                // quotation mark, and copies to quote. Then, rest of text
                // is concatenated to quote, which is then pushed as a single
                // command
                char quote[1024];
                strcpy(quote, strchr(token, '\"') + 1);
                token = strtok_r(NULL, "\"", &end_sing);
                if(token != NULL)
                {
                    strcat(quote, " ");
                    strcat(quote, token);
                    commands.push_back(quote);
                }
                else
                {
                    char copy[1024];
                    strncpy(copy, quote, static_cast<int>(strlen(quote)) - 1);
                    commands.push_back(copy);
                }
                token = strtok_r(NULL, " ", &end_sing);
            }
            else
            {
                commands.push_back(token);
                token = strtok_r(NULL, " ", &end_sing);
            }
        }
        // 100 is the magic number here for some reason
        char** inp = new char*[commands.size() + 100];
        for(int i = 0; i < commands.size(); i++)
        {
            inp[i] = new char[static_cast<int>(strlen(commands[i]))];
            inp[i] = commands[i];
        }
        in = inp;
    }
    void execute()
    {
        // Checks if test command was input. If so, test tests succeeding input
        string testString = "test";
        string testSymbol = "[";

        // Checks if exit command was input. If so, exits shell
        string exitString = "exit";

        if(strcmp(in[0], testString.c_str()) == 0 && valid)
        {
            test();
        }
        // Checks if symbolic version of test was input
        else if(strcmp(in[0], testSymbol.c_str()) == 0 && valid) 
        {
            test();
        }
        else if(strcmp(in[0], exitString.c_str()) == 0)
        {
            setExitCalled(true);
        }
        else if(valid)
        {
            pid_t pid;
            int status;
           
            bool doPipe = false; 
            // Checks for piping symbol 
            for(int i = 0; in[i] != NULL; i++)
            {
                if(strcmp(in[i], "|") == 0)
                {
                    doPipe = true;
                }
            }

            if(doPipe)
            {
                // Count keeps track of the number of total commands
                int count = 0;
                
                // fd1 and fd2 refer to file descriptors. Two are needed for
                // piping multiple times, to feed one into the other
                int fd1[2];
                int fd2[2];

                // Counts the number of commands
                for(int i = 0; in[i] != NULL; i++)
                {
                    if(strcmp(in[i], "|") == 0)
                    {
                        count++;
                    }
                }
                // inputIndex keeps track of in, total input input by user
                // This is kept track so program iterates through in while loop
                int inputIndex = 0;
                // i keeps track of home many commands has been run, - 1
                int i = 0;
                // isDone is true when the end of in is reached
                bool isDone = false;
                // runs until end of in is reached
                while(in[inputIndex] != NULL && !isDone)
                {
                    // temporarily stores single command
                    char *pipeCommand[256];
                    // k is used to store commands from in into pipeCommand
                    int k = 0;
                    // this while loop removes commands seperated by |
                    while(strcmp(in[inputIndex], "|") != 0)
                    {
                        pipeCommand[k] = in[inputIndex];
                        inputIndex++;
                        if(in[inputIndex] == NULL)
                        {
                            k++;
                            isDone = true;
                            break;
                        }
                        k++;	
                    }
                    pipeCommand[k] = NULL;
                    inputIndex++;
                    // checks if command is even numbered or odd, in order
                    // to assign to correct file descriptor, as file 
                    // descriptors can't be duped to themselves
                    if(i % 2 != 0)
                    {
                        pipe(fd1);
                    }
                    else
                    {
                        pipe(fd2);
                    }
                    pid = fork();
                    if(pid == 0)
                    {	
                        // if first comand
                        if(i == 0)
                        {
                            dup2(fd2[1], STDOUT_FILENO);
                        }
                        // if last command
                        else if(i == count)
                        {
                            if((count + 1) % 2 != 0)
                            {
                                dup2(fd1[0], STDIN_FILENO);
                            }
                            else
                            {
                                dup2(fd2[0], STDIN_FILENO);
                            }
                        }
                        else
                        {
                            if(i % 2 != 0)
                            {
                                dup2(fd2[0], STDIN_FILENO);
                                dup2(fd1[1], STDOUT_FILENO);
                            }
                            else
                            {
                                dup2(fd1[0], STDIN_FILENO);
                                dup2(fd2[1], STDOUT_FILENO);
                            }
                        }
                        for(int m = 0; pipeCommand[m] != NULL; m++)
                        {
                            if(strcmp(pipeCommand[m], ">") == 0 || strcmp(pipeCommand[m], ">>") == 0)
                            {
                                // Checks if there is valid input after >
                                if(pipeCommand[m+1] == NULL)
                                {
                                    printError("ERROR: Missing arguments for output redirection"); 
                                }
                                else
                                {
                                    // Create/Open file
                                    int fd = creat(pipeCommand[m+1], 0666);
                                    dup2(fd, STDOUT_FILENO);
                                    close(fd);
                                }
                                // character is replaced by NULL so exec won't go further
                                pipeCommand[m] = NULL;
                            }
                            else if(strcmp(pipeCommand[m], "<") == 0 || strcmp(pipeCommand[m], "<<") == 0)
                            {
                                // Check for valid input before and after <
                                if(pipeCommand[m+1] == NULL)
                                {
                                    printError("ERROR: Missing arguments for input redirection"); 
                                }
                                else
                                {
                                    int fd = open(pipeCommand[m+1], O_RDONLY);
                                    dup2(fd, STDIN_FILENO);
                                    close(fd); 
                                }
                                pipeCommand[m] = NULL;
                            }
                        }
                        execvp(*pipeCommand, pipeCommand);
                    }
                    else if(pid < 0)
                    {
                        // Prints error if pid doesn't exist, or for returns error
                       printError("ERROR: pid < 0");
                    }
                    // closes relevant file descriptors
                    if(i == 0)
                    {
                        close(fd2[1]);
                    }
                    else if(i == count)
                    {
                        if((count + 1) % 2 != 0)
                        {
                            close(fd1[0]);
                        }
                        else
                        {
                            close(fd2[0]);
                        }
                    }
                    else
                    {
                        if(i % 2 != 0)
                        {
                            close(fd2[0]);
                            close(fd1[1]);
                        }
                        else
                        {
                            close(fd1[0]);
                            close(fd2[1]);
                        }
                    }
                    waitpid(pid, &status, WUNTRACED);
                    i++;
                } 
            }
            else
            {
                pid = fork();
                if(pid == 0)
                {
                    // Child

                    // Runs for loop that goes through all commands, checks if
                    // there are relevant symbols
                    for(int i = 0; in[i] != NULL; i++)
                    {
                        if(strcmp(in[i], ">") == 0 || strcmp(in[i], ">>") == 0)
                        {
                            // Checks if there is valid input after >
                            if(in[i+1] == NULL)
                            {
                                printError("ERROR: Missing arguments for output redirection"); 
                            }
                            else
                            {
                                // Create/Open file
                                int fd = creat(in[i+1], 0666);
                                dup2(fd, STDOUT_FILENO);
                                close(fd);
                            }
                            // character is replaced by NULL so exec won't go further
                            in[i] = NULL;
                        }
                        else if(strcmp(in[i], "<") == 0 || strcmp(in[i], "<<") == 0)
                        {
                            // Check for valid input before and after <
                            if(in[i+1] == NULL)
                            {
                                printError("ERROR: Missing arguments for input redirection"); 
                            }
                            else
                            {
                                int fd = open(in[i+1], O_RDONLY);
                                dup2(fd, STDIN_FILENO);
                                close(fd); 
                            }
                            in[i] = NULL;
                        }
                    }
                    if(execvp(*in, in) == -1)
                    {
                        // Prints error if input is invalid
                        printError("ERROR: Invalid input");
                    }
                } else if(pid < 0)
                {
                    // Prints error if pid doesn't exist, or for returns error
                    printError("ERROR: pid < 0");
                } else
                {
                    // Parent
                    do
                    {
                        waitpid(pid, &status, WUNTRACED);
                    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
                }
                if(WEXITSTATUS(status) != 0) {valid = false;}
            }
        }
    }
private:
    void printError(string err)
    {
        valid = false;     
        output = err;
        print();
        exitShell();

    }
    void test()
    {
        // buf is needed for stat function
        struct stat buf;

        // flags to be checked
        string fileStr = "-f";
        string dirStr = "-d";
        string defStr = "-e";
        char* fileFlag = strdup(fileStr.c_str());
        char* dirFlag = strdup(dirStr.c_str());
        char* defFlag = strdup(defStr.c_str());
        
        if(strcmp(in[1], fileFlag) == 0)
        {
            if(stat(in[2], &buf) == 0 && S_ISREG(buf.st_mode) != 0)
            {
                cout << "(True)" << endl;
            }
            else
            {
                cout << "(False)" << endl;
            }
        }
        else if(strcmp(in[1], dirFlag) == 0)
        {
            if(stat(in[2], &buf) == 0 && S_ISDIR(buf.st_mode) != 0)
            {
                cout << "(True)" << endl;
            }
            else
            {
                cout << "(False)" << endl;
            }
        }
        else if(strcmp(in[1], defFlag) == 0)
        {
            if(stat(in[2], &buf) == 0)
            {
                cout << "(True)" << endl;
            }
            else
            {
                cout << "(False)" << endl;
            }
        }
        else
        {
            if(stat(in[1], &buf) == 0)
            {
                cout << "(True)" << endl;
            }
            else
            {
                cout << "(False)" << endl;
            }
        }
    }
};

class CompositeCommand : public BaseCommand
{
protected:
    vector<BaseCommand*> inputs;
    vector<BaseCommand*> outputs;
public:
    CompositeCommand() 
    {
        valid = true;
        type = 0;
        exitCalled = false;
    }
    CompositeCommand(char* in)
    {
        valid = true;
        type = 0;
        exitCalled = false;
        this->parse(in);
    }

    virtual void parse(char* in) 
    {
        char* end_comp;
        char* token = strtok_r(in, ";", &end_comp);
        
        while (token != NULL)
        {
            // Checks if connectors are present. If they are, relevant
            // type is then passed to constructor of command. If code is a
            // comment, command is ignored
            if(strstr(token, "(") != NULL)
            {
                if(checkParen(in))
                {
                    char* end_par;

                    token = strtok_r(token, "()", &end_par);
                    inputs.push_back(new CompositeCommand(token));
                    token = strtok_r(NULL, ";", &end_comp);
                }
                else
                {
                    inputs.push_back(new Command(token, false));
                    token = strtok_r(NULL, ";", &end_comp);
                }
            }
            else if(strstr(token, "#") != NULL)
            {
                token = strtok_r(NULL, ";", &end_comp);
            }
            else if(strstr(token, "||") != NULL || strstr(token, "&&") != NULL)
            {
                /* isOr checks if first instance of connector is || or &&. 
                 * This is done because strtok removes first connector, 
                 * causing issues determining what the connector was.
                 */
                bool isOr;
                if(strstr(token, "||") != NULL && strstr(token, "&&") != NULL)
                {
                    isOr = strchr(token, '|') < strchr(token, '&');
                }
                else if(strstr(token, "||") != NULL)
                {
                    isOr = true;
                }
                else if(strstr(token, "&&") != NULL)
                {
                    isOr = false;
                }
                bool isFirst = true;
                
                char* end_con;
                
                // Adds first command, then signals what type of command
                // the second command is. From there, loops until done          
                char* copy = strdup(token);
                char* conToken = strtok_r(token, "||&&", &end_con);
                inputs.push_back(new Command(conToken));
                conToken = strtok_r(NULL, "|&", &end_con); 
                if(isOr)
                {
                    inputs.push_back(new Command(conToken, 1));
                }
                else
                {
                    inputs.push_back(new Command(conToken, 2));
                }
                conToken = strtok_r(NULL, "|&", &end_con); 
                while(conToken != NULL)
                {
                    if(copy[conToken - token - 1] == '|')
                    {
                        inputs.push_back(new Command(conToken, 1));
                    }
                    else if(copy[conToken - token - 1] == '&')
                    {
                        inputs.push_back(new Command(conToken, 2));
                    }
                    isFirst = false;
                    conToken = strtok_r(NULL, "|&", &end_con); 
                }
                token = strtok_r(NULL, ";", &end_comp);
            }
            else
            {
                // Checks if closed braket, for test 
                if(strstr(token, "[") != NULL && !checkParen(in))
                {
                    inputs.push_back(new Command(token, false));
                    token = strtok_r(NULL, ";", &end_comp);
                }
                else
                {
                    inputs.push_back(new Command(token));
                    token = strtok_r(NULL, ";", &end_comp);
                }
            }
        }
    }
    virtual void execute() 
    {
        bool locValid = true;
        for(int i = 0; i < inputs.size(); i++)
        {
            // executes commands, depending on input and if they are valid
            switch(inputs[i]->getType())
            {
                case 0:
                    inputs[i]->execute();
                    break;
                case 1:
                    if(!locValid) 
                    {
                        inputs[i]->execute(); 
                    }
                    break;
                case 2:
                    if(locValid) 
                    {
                        inputs[i]->execute(); 
                    }
                    break;
            }

            locValid = inputs[i]->isValid();
            this->setExitCalled(inputs[i]->isExitCalled());
            if(this->isExitCalled())
            {
                break;
            }
        }
    }
    void add(BaseCommand* com)
    {
        inputs.push_back(com);
    }
    // Using stack data structure to check for balanced parentheses
    // Find matching pair of parentheses, if stack is empty return true otherwise false
    bool checkParen(char* cmdInput)
    {
        stack<char> s;
        char temp1, temp2, temp3;
        
        for (int i=0; i<strlen(cmdInput); i++)
        {
            if (cmdInput[i] == '(' || cmdInput[i] == '[' || cmdInput[i] == '{')
            {
                // Push element at index into the stack
                s.push(cmdInput[i]);
            }
            else
            {
                switch (cmdInput[i])
                {
                    case ')':
                        
                        temp1 = s.top();
                        s.pop();
                        if (temp1=='{'||temp1=='[')
                            cout << "ERROR: Missing parentheses" << endl;
                        break;
                    case '}':
                        
                        temp2 = s.top();
                        s.pop();
                        if (temp2=='('||temp2=='[')
                            cout << "ERROR: Missing brace" << endl;
                        break;
                    case ']':
                        
                        temp3=s.top();
                        s.pop();
                        if (temp3=='('||temp3=='{')
                            cout << "ERROR: Missing bracket" << endl;
                        break;
                }
            }
            
        }
        
        if (s.empty())
            return true;
        else
        {
            char tempChar;
            while(!s.empty())
            {
                tempChar = s.top();
                s.pop();
                switch (tempChar)
                {
                    case '(':
                        cout << "ERROR: Missing parentheses" << endl;
                        break;
                    case '{':
                        cout << "ERROR: Missing brace" << endl;
                        break;
                    case '[':
                        cout << "ERROR: Missing bracket" << endl;
                        break;
                }
                
            }
            return false;
        }
    }

};


#endif
