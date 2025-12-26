
g++ -Wall -g -c sfw.cpp -o sfw.o
g++ -Wall -g -c test_scene.cpp -o test_scene.o
g++ -Wall -g -c main.cpp -o main.o

# You might need to add -lpthread and/or -latomic depending on your compiler version

g++ -Wall -g sfw.o test_scene.o main.o \
                        -lgdi32 -lShlwapi -lws2_32 \
                        -o game 
