# Unix-Shell-and-History-Feature

C program to serve as a shell interface to execute user commands as a separate background process. Creating a history feature to store the last 10 user commands and to execute them directly

To run, enter
make all
all: testtry

testtry: try.o gcc -pthread -o testtry try.o -lm

try.o: try.c gcc -pthread -c try.c -lm
