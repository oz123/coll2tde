CFLAGS+=-Wall -Werror -std=gnu99 -O2 -g 
LDFLAGS = -Wl,-rpath,$(LIBROOT)/dataextract
TDE_LIBS=-L/usr/lib/dataextract -lDataExtract
TDE_LDFLAGS=-Wl,-rpath=/usr/lib/dataextract
JSMN=-Ljsmn/ -ljsmn 

OBJS=mongo.o tde.o json.o log.o

PREFIX ?= /usr/local/bin 

.PHONY: all clean coll2tde

all: coll2tde

log.o: log.c
	$(CC) $(CFLAGS) -c log.c 

tde.o: tde.c
	$(CC) $(CFLAGS) -c tde.c $(TDE_LIBS) $(TDE_LDFLAGS) 

mongo.o: mongo.c
	$(CC) $(CFLAGS) -c mongo.c \
		$(shell pkg-config --cflags --libs libmongoc-1.0) 
json.o: json.c
	$(CC) $(CFLAGS) -c json.c $(JSMN)

coll2tde: $(OBJS)
	$(CC) $(CFLAGS) -o coll2tde $(OBJS) coll2tde.c \
		$(TDE_LIBS) $(TDE_LDFLAGS) $(JSMN) \
		$(shell pkg-config --cflags --libs libmongoc-1.0) 

install:
	install coll2tde $(PREFIX)/coll2tde

clean:
	$(RM) $(OBJS) coll2tde
