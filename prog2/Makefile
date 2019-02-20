make:	
	gcc -Wall -g -o server prog2_server.c
	gcc -Wall -g -o client prog2_client.c

#default run for debugging the server quick
ds:
	./server 8080 8 30 twl06.txt

#default run for debugging the client quick
dc:
	./client localhost 8080

clean:
	rm server
	rm client
