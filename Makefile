IOSchedmake: main.c lib/thpool.c lib/thpool.h
	gcc -std=c99 -pthread -o iosched main.c lib/thpool.c 
