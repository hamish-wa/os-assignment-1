# Customer Queue - 20183622 - OS Assignment 1

## Running the program
Usage: ./bin/assignment <m> <t_C> <t_W> <t_D> <t_I>\
m  - The size/length of the customer queue.\
t_C - The customer arrival period.\
t_W - The time duration of a withdrawal.\
t_D - The time duration of a deposit.\
t_I - The time duration of an information query.\

Example: %s 100 5 2 2 1

## Building the program

### Regular build
A makefile is provided to build the program. \
To build the program, run the following command in the root directory of the project.
```bash
make
```
### Other make commands are also provided.
Make clean removes all object files and executables from the project *bin* and *build* directories.
```bash
make clean
```
Make debug builds the program with debug flags. All debug messages have been removed from the program. 
```bash
make DEBUG=1
```
Make valgrind runs the program with valgrind. 
```bash
make valgrind 
```
Make helgrind runs the program with helgrind.
```bash
make helgrind 
```

### Directory Structure
This is the directory structure of the assignment. Below shows the directory structure after building and running the program.
```bash
.
├── bin
│  └── assignment
├── build
│  └── assignment.o
├── Makefile
├── README.md
├── c_file
├── r_log
└── src
    ├── assignment.c
    └── standard.h
```

### Assumptions

- The PC must have the gcc compiler installed.
- The PC must have the make utility installed.
- The PC must have at least c89 installed.
- When building with valgrind or helgrind the PC must have the valgrind and helgrind tools installed. 
- The program will not run if the user provides invalid arguments. 
   - The program will not run if the user does not provide the correct number of arguments.
   - The program will not run if the user provides negative arguments.
   - The program will not run if the user provides a customer queue size of 0.
   - The program will not run if the user provides a customer arrival period of 0.
   - The program will not run if the user provides a withdrawal duration of 0.
   - The program will not run if the user provides a deposit duration of 0.
   - The program will not run if the user provides an information query duration of 0.
- The c_file must be in the root directory.
- The r_log file will be created in the root directory.

