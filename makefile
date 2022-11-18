default: all

all: backend frontend

backend:
	gcc backend.c ./Header/backlib.c ./Header/users_lib.c -o backend

frontend:
	gcc frontend.c -o frontend

clean:
	rm frontend backend