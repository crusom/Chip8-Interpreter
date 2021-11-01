main:
	g++ main.cpp renderer.cpp -o chip8 -g -lGL -lX11 -lpthread -lXrandr -lXi -ldl glad.c -lglfw3
