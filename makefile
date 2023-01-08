default: all

all: backend frontend

backend: backend.o ./Header/lib.o ./Header/users_lib.o
	gcc -Wall backend.o ./Header/lib.o ./Header/users_lib.o -o backend

frontend: frontend.o  ./Header/lib.o
	gcc -Wall frontend.o ./Header/lib.o -o frontend

lib.o: ./Header/lib.c ./Header/lib.h
	gcc -Wall -c ./Header/lib.c

backend.o: backend.c ./Header/lib.c ./Header/lib.h ./Header/structs.h
	gcc -Wall -c backend.c

frontend.o: frontend.c ./Header/lib.h
	gcc -Wall -c frontend.c

clean:
	rm -f frontend backend ./Header/lib.o backend.o frontend.o