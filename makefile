build:
	mkdir -p ./bin/
	gcc -Wall main.c -o ./bin/pdc -l curl
json:
	gcc -Wall json.c -g -o ./bin/json
clean:
	rm -r ./bin/
