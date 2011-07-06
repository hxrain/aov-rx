mpdm_r2: mpdm_r2.c
	gcc -g $< -o $@

clean:
	rm -f mpdm_r2
