CC = arm-none-linux-gnueabi-gcc -pthread -static

20141544 : main.o input.o output.o mode.o
	$(CC) -o 20141544 main.o input.o output.o mode.o

main.o : main.c
	$(CC) -c -o main.o main.c

input.o : input.c
	$(CC) -c -o input.o input.c -lpthread

output.o : output.c
	$(CC) -c -o output.o output.c
mode.o : mode.c
	$(CC) -c -o mode.o mode.c
clean :
	rm *.o 20141544
