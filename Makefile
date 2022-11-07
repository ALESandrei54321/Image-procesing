build:
	gcc *.c -o bmp -lm -g

run:
	./bmp

clean:
	rm -f bmp
