CFLAGS+=-Wall -Werror -std=gnu99 -O2 
LDFLAGS = -Wl,-rpath,$(LIBROOT)/dataextract
TDE_LIBS=-L/usr/lib/dataextract -lDataExtract
TDE_LDFLAGS=-Wl,-rpath=/usr/lib/dataextract
JSMN=-Ljsmn/ -ljsmn 

OBJS=mongo.o tde.o json.o

.PHONY: all clean

all: json tde mongo coll2tde

tde:
	$(CC) $(CFLAGS) -c tde.c $(TDE_LIBS) $(TDE_LDFLAGS) 

mongo:
	$(CC) $(CFLAGS) -c mongo.c \
		$(shell pkg-config --cflags --libs libmongoc-1.0) 
json:
	$(CC) ($CFLAGS) -c json.c $(JSMN)

coll2tde:
	$(CC) $(CFLAGS) -o coll2tde $(OBJS) coll2tde.c \
		$(TDE_LIBS) $(TDE_LDFLAGS) $(JSMN) \
		$(shell pkg-config --cflags --libs libmongoc-1.0) \

clean:
	$(RM) $(OBJS) coll2tde


.PHONY: all clean
