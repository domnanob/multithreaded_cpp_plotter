
export SDKROOT=$(xcrun --show-sdk-path)

export args="-std=c++14 -w -framework cocoa -framework iokit -framework CoreFoundation -framework CoreAudio -framework AudioToolbox "

clang++ $args -g -c sfw.cpp -o sfw.o

clang++ -w -framework cocoa -framework iokit -framework CoreFoundation -framework CoreAudio -framework AudioToolbox -g -c sfw_3rd.m -o sfw_3rd.o

clang++ $args -g -c test_scene.cpp -o test_scene.o
clang++ $args -g -c main.cpp -o main.o

# You might need to add -lpthread and/or -latomic depending on your compiler version

clang++ $args -g sfw.o sfw_3rd.o test_scene.o main.o -o game 
