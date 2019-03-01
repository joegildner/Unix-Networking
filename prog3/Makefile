default:
	gcc -o prog3_server prog3_server.c
	gcc -o prog3_observer prog3_observer.c
	gcc -o prog3_participant prog3_participant.c

s:
	./prog3_server 36799 46799

p:
	./prog3_participant localhost 36799

o:
	./prog3_observer localhost 46799
