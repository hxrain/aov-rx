aov-rx.o: aov-rx.c
	gcc -c $< -o $@

stress: aov-rx.c
	gcc -g -DSTRESS $< -o $@

clean:
	rm -f *.o stress *.exe
