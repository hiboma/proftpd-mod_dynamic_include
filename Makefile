
PRXS_DIR=/usr/bin
PRXS=$(PRXS_DIR)/prxs
SOURCE=mod_dynamic_include.c

mod_libmemcached_deny.so:
	$(PRXS) -c $(SOURCE) 

install: mod_libmemcached_deny.so
	$(PRXS) $(SOURCE) -i

clean:
	rm -v *.la
	rm -v *.lo
	rm -v *.o
	rm -frv .libs
