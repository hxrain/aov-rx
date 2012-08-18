CFLAGS=-Wall -g

aov-rx.o: aov-rx.c
	$(CC) $(CFLAGS) -c $< -o $@

stress: stress.c aov-rx.o
	$(CC) $(CFLAGS) $< aov-rx.o -o $@ && ./stress

clean:
	rm -f *.o stress *.exe
