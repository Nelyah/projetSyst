CC=gcc
CFLAGS=-c -Wall -g 
LDFLAGS=$(shell pkg-config --cflags --libs gtk+-2.0)
SOURCES=main.c ajouter_Compound.c ajouter_Dated.c ajouter_texte.c ajouter_Jpeg.c ajouter_Png.c compression.c fileSelection.c fonctionsGenerales.c lecture.c ouverture.c quitter.c suppression.c creerDazibao.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dazibao

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o: 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

clean :
	rm -f *.o 
