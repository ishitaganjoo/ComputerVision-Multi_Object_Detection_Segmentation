all: SiftCNN.cpp
	g++ SiftCNN.cpp -o SiftCNN -lX11 -lpthread -I. -Isiftpp -O3 siftpp/sift.cpp -std=c++11 `pkg-config opencv --cflags --libs` 

clean:
	rm SiftCNN