all:
	gcc main.c -o rebel_chicken.exe -I C:/raylib/raylib/src -L C:/raylib/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm

run: all
	./rebel_chicken.exe