all: visiondemo
visiondemo: demo.o yellow.o ../lib/free.o
	$(CXX) $^ -o visiondemo -lopencv_core -lopencv_highgui -lopencv_imgproc -lfreenect_sync
%.o: %.cpp
	$(CXX) -g -c -Wall -std=c++11 -I. -I../lib $^ -o $@
clean:
	-rm -f *.o ../lib/*.o visiondemo
