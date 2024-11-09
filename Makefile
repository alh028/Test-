chat-server: http-server.c chat-server.c
	gcc -o chat-server http-server.c chat-server.c

clean:
	rm -f *.o chat-server