CC := clang
# CFLAGS := -g -fsanitize=address
CFLAGS := -g

all: server client

clean:
	rm -rf server client

server: server.c message.h message.c socket.h
	$(CC) $(CFLAGS) -o server server.c message.c -lpthread

client: client.c message.h message.c socket.h
	$(CC) $(CFLAGS) -o client client.c message.c -lpthread

zip:
	@zip -q -r project.zip . -x .git/\* .vscode/\* .clang-format .gitignore client message socket
	@echo "Done. Please upload to Gradescope."


