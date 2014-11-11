coll2tde:
	gcc -o coll2tde coll2tde.c \
		$(shell pkg-config --cflags --libs libmongoc-1.0) \
	    -lcsv


clean:
	rm -v coll2tde


