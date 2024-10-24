CXX = g++
CXXFLAGS = -Wall -std=c++11
TARGET = disassem 

all: $(TARGET)
	./$(TARGET) test.obj test.sym  # Run the program after building it

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET)