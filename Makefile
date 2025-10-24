CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic -std=gnu99 -pthread -O2
EXAMPLES=fibs fauxgrep fauxgrep-mt fhistogram fhistogram-mt
TESTS=test_job_queue

.PHONY: all test clean ../src.zip

all: $(TESTS) $(EXAMPLES)

job_queue.o: job_queue.c job_queue.h
	$(CC) -c job_queue.c $(CFLAGS)

%: %.c job_queue.o
	$(CC) -o $@ $^ $(CFLAGS)


test: $(TESTS)
	@echo "Running unit tests..."
	@set -e; for test in $(TESTS); do \
		echo "Executing $$test..."; \
		./$$test; \
	done
	@echo "Running performance comparison..."
	@chmod +x test_fauxgrep.sh
	@./test_fauxgrep.sh

valgrind: test_job_queue
	valgrind --leak-check=full ./test_job_queue
	valgrind --leak-check=full ./fauxgrep malloc src_test
	valgrind --leak-check=full ./fauxgrep-mt -n 4 malloc src_test

clean:
	rm -rf $(TESTS) $(EXAMPLES) *.o core test_dir big_test


zip: ../src.zip

../src.zip:
	make clean
	cd .. && zip src.zip -r src
