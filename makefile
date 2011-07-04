CFLAGS=-Wall -g
LDFLAGS=-lglfw -lm
all: glfw test-queue
queue.o: queue.c
test-queue: queue.c
	gcc queue.c -I. -DTEST -o test-queue
glfw: glfw.c queue.o
