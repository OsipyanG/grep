CC=gcc
CFLAGS= -c -std=c11 -Wall -Wextra -Werror

.PHONY: all
all: grep

grep: grep.o parser.o
	$(CC) grep.o parser.o -o grep

grep.o: grep.c params.h
	$(CC) $(CFLAGS) grep.c

parser.o: parser.c params.h
	$(CC) $(CFLAGS) parser.c

.PHONY: commit
commit:
	git pull
	git add .
	git commit -m Update
	git push

.PHONY: clean
clean:
	rm -rf *.o grep

.PHONY: rebuild
rebuild: clean all

.PHONY: test
test: rebuild
	sh ./tests.sh

.PHONY: test_leaks
test_leaks: rebuild
	sh ./test_leaks.sh
	
.PHONY: format
format:
	clang-format -i *.c *.h
	
