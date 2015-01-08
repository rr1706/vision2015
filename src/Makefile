all: demo
CXX=g++ -DDEBUG -g -Wall
release: CXX = g++ -O3
release: demo
demo: demo.o yellow.o ../lib/free.o
	$(CXX) $^ -o demo -lopencv_core -lopencv_highgui -lopencv_imgproc -lfreenect_sync
%.o: %.cpp
	$(CXX) -c -std=c++11 -I. -I../lib $^ -o $@
clean:
	-rm -f *.o ../lib/*.o demo
