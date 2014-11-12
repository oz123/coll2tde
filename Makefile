LIBS = -L$(LIBROOT)/dataextract -lDataExtract
LDFLAGS = -Wl,-rpath,$(LIBROOT)/dataextract
TDE_LIBS=-L/usr/lib/dataextract -lDataExtract
TDE_LDFLAGS=-Wl,-rpath=/usr/lib/dataextract

.PHONY: all coll2tde clean

coll2tde:
	gcc -o coll2tde coll2tde.c \
		$(TDE_LIBS) $(TDE_LDFLAGS) \
		$(shell pkg-config --cflags --libs libmongoc-1.0) \

clean:
	rm -v coll2tde


