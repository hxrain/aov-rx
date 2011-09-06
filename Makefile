aov-rx.o: aov-rx.c
	gcc -g -c $< -o $@

stress: stress.c aov-rx.o
	gcc -g $< aov-rx.o -o $@

clean:
	rm -f *.o stress *.exe
