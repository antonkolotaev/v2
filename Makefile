CXX=g++
CFLAGS=-W



INCLDIR =  -I. -I/usr/local/boost

TARGET=TESTER

all: $(EXEC)
	$(CXX) ./tests/*.cpp $(INCLDIR) -o $(TARGET) $(CFLAGS)
clean:
	rm -rf *.*~
	rm $(TARGET)

