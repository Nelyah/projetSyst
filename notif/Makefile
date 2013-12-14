CC=gcc
CFLAGS=-c -Wall -g  
LDFLAGS=
SOURCES1=serveur.c 
SOURCES2=client.c
OBJECTS1=$(SOURCES1:.c=.o)
OBJECTS2=$(SOURCES2:.c=.o)
EXECUTABLE1=serveur
EXECUTABLE2=client

all: $(EXECUTABLE1) $(EXECUTABLE2)

$(EXECUTABLE1): $(OBJECTS1)
	$(CC) $(LDFLAGS) $(OBJECTS1) -o $@

$(EXECUTABLE2): $(OBJECTS2)
	$(CC) $(LDFLAGS) $(OBJECTS2) -o $@

.c.o: 
	$(CC) $< $(LDFLAGS) $(CFLAGS) -o $@

clean :
	rm -f *.o client serveur
