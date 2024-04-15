g++ -c main.cpp stb_image.cpp -I ./include/ -DGLEW_STATIC
g++ *.o -o main.exe -L ./lib/ -DGLEW_STATIC -lglfw3 -lglew32s -lopengl32  -lgdi32
del *.o
main.exe