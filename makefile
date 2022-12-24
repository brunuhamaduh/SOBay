default: all

all: backend frontend

backend: backend.o ./Header/backlib.o ./Header/users_lib.o
	gcc backend.o ./Header/backlib.o ./Header/users_lib.o -o backend

frontend: frontend.o 
	gcc frontend.o -o frontend

backlib.o: ./Header/backlib.c ./Header/backlib.h
	gcc -c ./Header/backlib.c
backend.o: backend.c ./Header/backlib.c ./Header/backlib.h
	gcc -c backend.c
frontend.o: frontend.c
	gcc -c frontend.c

clean:
	rm -f frontend backend ./Header/backlib ./Header/backlib.o backend.o frontend.o