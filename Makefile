all:
	gcc -ggdb pitchdetector.c -o pitchdetector `pkg-config --libs --cflags gtk+-3.0` -lsndfile --std=c99
