main:main.o ELF_process.o 
	g++ -o main ELF_process.o  main.o
main.o:main.cpp ELF_process.h
	g++ -c main.cpp -o main.o
process.o:ELF_process.cpp ELF_process.h
	g++ -c ELF_process.cpp -o ELF_process.o
