all: client server

messagepass.h: messagepass.x
	rpcgen messagepass.x

messagepass_svc.c messagepass_clnt.c client.c: messagepass.h

client: client.o messagepass_clnt.o
	cc -o client client.o messagepass_clnt.o  

server: server.o messagepass_svc.o
	cc -o server server.o messagepass_svc.o 

.PHONY: clean

clean:
	-rm *.o
	-rm client
	-rm server
	-rm messagepass.h
	-rm messagepass_clnt.c
	-rm messagepass_svc.c
	-rm secrets.out
