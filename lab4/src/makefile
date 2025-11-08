CC=gcc
CFLAGS=-I. -pthread
TARGETS=parallel_min_max parallel_sum process_memory
ALL_TARGETS=$(TARGETS)

all: $(ALL_TARGETS)

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

parallel_sum : utils.o sum_lib.o utils.h sum_lib.h
	$(CC) -o parallel_sum utils.o sum_lib.o parallel_sum.c $(CFLAGS)

process_memory : 
	$(CC) -o process_memory process_memory.c $(CFLAGS)

sum_lib.o : sum_lib.h
	$(CC) -o sum_lib.o -c sum_lib.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean :
	rm utils.o find_min_max.o parallel_min_max parallel_sum process_memory