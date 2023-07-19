main:   main.o
    gcc sistema.o   main.c  -o  main

sistema.o:  sistema.c   sistema.h
    gcc -c  sistema.c   -o  sistema.o

clean:
    rm  -rf *.o main.o