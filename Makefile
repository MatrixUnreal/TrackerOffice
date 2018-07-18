all:
	g++ -std=c++14 -o tracker *.cpp Tracker/*.cpp `pkg-config opencv  --cflags --libs` -lstdc++fs -lpthread  ~/lib/darknet/darknet.so -DOPENCV=1 