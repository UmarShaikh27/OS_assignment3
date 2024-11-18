build:
	gcc -o paging paging.c
run:
	./paging $(ARGS)
clean:
	rm -f paging