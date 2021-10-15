main:
	g++ main.cpp renderer.cpp -o main -g -lGL -lX11 -lpthread -lXrandr -lXi -ldl glad.c -lglfw3
