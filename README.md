# SIMPLE SHELL



SIMPLE SHELL is a command shell written in C++ to read a unique and unlimited set of commands. 
For example:
Single commands
```sh
$ cd ls-a
$ npm install -d
$ node app
```
Multiple commands on a single command line
```sh
$ ls -a; echo hello; mkdir test
```
Multiple commands with combination of operators
```sh
$ ls -a; echo hello && mkdir test || echo world; git status
```
 
# Features List

-  Starts command shell with ability to exit
- Rshell reads and executes single command lines
- Option to take multiple command lines with connectors
   -  (  ||, &&, ;)
- Handles command line with #comments
- Prints a command prompt


# Important family of functions used
- fork();
- execvp();
- strtok();
   ```
    virtual void parse(char* input)
    {
        char* token = strtok (input," || && ;");
        commands.reserve(1000);        
        while (token != NULL)
        {
            commands.push_back(token);
            token = strtok (NULL, " || && ;");
        }

### AUTHORS
| Name | Email |
| ------ | ------ |
| LIAM HAN | whan013@ucr.edu
| ELIJAH NICASIO | enica001@ucr.edu |

