default:
	gcc -g -o prog3_server prog3_server.c
	gcc -g -o prog3_observer prog3_observer.c
	gcc -g -o prog3_participant prog3_participant.c

s:
	./prog3_server 36789 46789

p:
	./prog3_participant localhost 36789

o:
	./prog3_observer localhost 46789

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
