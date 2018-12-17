test_conf: 
	rm -rf test_conf
	gcc -DCONF_TEST -std=gnu99 -g util.h util.c cstring.h cstring.c array.h array.c log.h log.c conf.h conf.c -o test_conf -lyaml

test_skip:
	rm -rf test_*
	gcc -g -std=gnu99 -DTEST skip_list.h skip_list.c -o test_skip
httpd: httpd.c
	gcc -W -Wall -lsocket -lpthread -o httpd httpd.c

clean:
	rm -rf test_*
