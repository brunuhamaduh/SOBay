default: all

all: backend frontend

backend: backend.o ./Header/backlib.o ./Header/users_lib.o
	gcc -Wall backend.o ./Header/backlib.o ./Header/users_lib.o -o backend

frontend: frontend.o  ./Header/backlib.o
	gcc -Wall frontend.o ./Header/backlib.o -o frontend

backlib.o: ./Header/backlib.c ./Header/backlib.h
	gcc -Wall -c ./Header/backlib.c

backend.o: backend.c ./Header/backlib.c ./Header/backlib.h
	gcc -Wall -c backend.c

frontend.o: frontend.c ./Header/backlib.h
	gcc -Wall -c frontend.c

clean:
	rm -f frontend backend ./Header/backlib ./Header/backlib.o backend.o frontend.o