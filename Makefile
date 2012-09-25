CFLAGS=-Wall -g

all: stress

aov-rx.o: aov-rx.c
	$(CC) $(CFLAGS) -c $< -o $@

stress-test: stress
	./stress

stress: stress.c aov-rx.o
	$(CC) $(CFLAGS) $< aov-rx.o -o $@

clean:
	rm -f *.o stress *.exe
