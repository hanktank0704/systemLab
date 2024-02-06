CC = gcc
TARGET = pa1

pa1 : main.o
	gcc -o pa1 main.o

main.o : main.c
	gcc -c -o main.o main.c
  
clean : 
	rm ./pa1
	rm *.o 