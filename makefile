default: all

all: backend frontend

backend: backend.o ./Header/backend_lib.o ./Header/users_lib.o
	gcc -Wall backend.o ./Header/backend_lib.o ./Header/users_lib.o -o backend

frontend: frontend.o  ./Header/frontend_lib.o
	gcc -Wall frontend.o ./Header/frontend_lib.o -o frontend

backend_lib.o: ./Header/backend_lib.c ./Header/backend_lib.h
	gcc -Wall -c ./Header/backend_lib.c

frontend_lib.o: ./Header/frontend_lib.c ./Header/frontend_lib.h
	gcc -Wall -c ./Header/frontend_lib.c

backend.o: backend.c ./Header/backend_lib.c ./Header/backend_lib.h ./Header/backend_structs.h ./Header/shared_structs.h ./Header/users_lib.h
	gcc -Wall -c backend.c

frontend.o: frontend.c ./Header/frontend_lib.c ./Header/frontend_lib.h ./Header/frontend_structs.h ./Header/shared_structs.h
	gcc -Wall -c frontend.c

clean:
	rm -f frontend backend ./Header/backend_lib.o ./Header/frontend_lib.o backend.o frontend.o