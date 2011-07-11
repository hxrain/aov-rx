mpdm_r2.o: mpdm_r2.c
	gcc -c $< -o $@

stress: mpdm_r2.c
	gcc -g -DSTRESS $< -o $@

clean:
	rm -f *.o stress *.exe
