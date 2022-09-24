linux:
	g++ main.cpp glad/src/glad.c -ojumpman -O2 -lglfw -lGL -lX11 -lpthread -ldl -lGLU -Wno-write-strings -Iglad/include

windows:
	x86_64-w64-mingw32-g++ main.cpp glad/src/glad.c -ojumpman-windows -lglfw3 -lopengl32 -lgdi32 -Iglad/include