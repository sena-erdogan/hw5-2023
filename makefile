all:
	gcc -Wall -g -o hw5 hw5.c -pthread -lm

clean:
	rm -rf *.o hw5
