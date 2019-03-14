default:
	gcc -g -o prog3_server prog3_server.c
	gcc -g -o prog3_observer prog3_observer.c
	gcc -g -o prog3_participant prog3_participant.c

s:
	./prog3_server 36799 46799

p:
	./prog3_participant localhost 36799

o:
	./prog3_observer localhost 46799

clean:
	rm prog3_participant
	rm prog3_observer
	rm prog3_server
	rm core

vs:
	vim prog3_server.c

vo:
	vim prog3_observer.c

vp:
	vim prog3_participant.c
