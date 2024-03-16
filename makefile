build:
	mkdir -p ./bin/
	gcc -Wall main.c -o ./bin/pdc -l curl
clean:
	rm -r ./bin/
