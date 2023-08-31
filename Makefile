all:
	gcc main.c -o main -lm -lpthread

clean:
	rm ./main