shell: parser.c
		gcc -std=gnu11 parser.c -o shell

clean:
		rm -f shell *.o