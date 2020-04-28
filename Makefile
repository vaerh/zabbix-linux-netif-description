discovery: discovery.c
	gcc -fPIC -shared -o discovery.so discovery.c -I../../../include
