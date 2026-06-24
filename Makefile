CXX = g++
CXXFLAGS = -std=c++11 -O3 `pkg-config --cflags gtk+-3.0 webkit2gtk-4.1`
LIBS = `pkg-config --libs gtk+-3.0 webkit2gtk-4.1`

all: my_app

my_app: src/main.cpp src/webview.h
	$(CXX) $(CXXFLAGS) src/main.cpp -o my_app $(LIBS)

clean:
	rm -f my_app