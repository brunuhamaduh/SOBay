default: all

all: backend frontend

backend: backend.o ./Header/backlib.o ./Header/users_lib.o ./Header/sharedlib.h
	gcc -Wall backend.o ./Header/backlib.o ./Header/users_lib.o -o backend

frontend: frontend.o 
	gcc -Wall frontend.o -o frontend

backlib.o: ./Header/backlib.c ./Header/backlib.h
	gcc -Wall -c ./Header/backlib.c

backend.o: backend.c ./Header/backlib.c ./Header/backlib.h
	gcc -Wall -c backend.c

frontend.o: frontend.c ./Header/sharedlib.h
	gcc -Wall -c frontend.c

clean:
	rm -f frontend backend ./Header/backlib ./Header/backlib.o backend.o frontend.o