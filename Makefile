CC = gcc
CFLAG = -Wall 
CFLAGS = -pthread -lrt
HEADERS = File.c
RM= rm -rf

normal : Demon client 
	gnome-terminal -- ./Demon 
	gnome-terminal -- ./client
			
Demon : Demon.c
	$(CC) $(CFLAG) -o Demon Demon.c $(HEADERS) $(CFLAGS)	
	  
client : client.c	
		$(CC) $(CFLAG) -o client client.c $(HEADERS) $(CFLAGS)
				
	
                
clean :
	$(RM) Demon client *.o Tube*
