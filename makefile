default: all

all: clean frontend.o backend.o

frontend.o: frontend.c
	gcc frontend.c -o frontend

backend.o: backend.c
	gcc backend.c -o backend

clean:
	rm -f backend frontend
