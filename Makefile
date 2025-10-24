CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic -std=gnu99 -pthread
EXAMPLES=fibs fauxgrep fauxgrep-mt fhistogram fhistogram-mt
TESTS=test_job_queue

.PHONY: all test clean ../src.zip

all: $(TESTS) $(EXAMPLES)

job_queue.o: job_queue.c job_queue.h
	$(CC) -c job_queue.c $(CFLAGS)

%: %.c job_queue.o
	$(CC) -o $@ $^ $(CFLAGS)


test: $(TESTS)
	@set e; for test in $(TESTS); do echo ./$$test; ./$$test; done

valgrind: test_job_queue
	valgrind --leak-check=full ./test_job_queue

clean:
	rm -rf $(TESTS) $(EXAMPLES) *.o core

zip: ../src.zip

../src.zip:
	make clean
	cd .. && zip src.zip -r src
