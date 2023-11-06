INCLUDE=-I../include/ -I../353068/
FLAGS=-Wall -Wextra -Wfatal-errors -O2 -std=c11 -fPIC
NAME=-o main
IMPLEMENTATIONS=../353068/*.c

build:
	gcc $(IMPLEMENTATIONS) main.c $(NAME) $(FLAGS) $(INCLUDE) 

clean:
	rm main

run:
	./main