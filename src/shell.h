#ifndef __RSHELL_H_
#define __RSHELL_H_

#include "command.h"
#include <iostream>
#include <string>

using namespace std;

class rShell
{
    private:
        bool isRunning; //Checks if shell is currently running. If it is, 
                        //run() loops else, loop breaks
    public:
        rShell() { isRunning = true; }
        void run()
        {
            while(isRunning)
            {
                char input[1024];
                cout << "$ "; //Prints '$' before every user input
                cin.getline(input, 1024);
                
                string exitStr = "exit"; //checks if user inputs exit. 
                                         //If so, loop exits
                char* ex = new char[exitStr.length()];
                strcpy(ex, exitStr.c_str());

                CompositeCommand* cmds = new CompositeCommand(input);
                cmds->execute();
                if(cmds->isExitCalled())
                {
                    isRunning = false;
                }
            }
        }
};

#endif
