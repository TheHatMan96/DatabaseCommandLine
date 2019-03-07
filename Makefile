TARGET=Exec
C_SRCFILES=main.cpp DiskAPI.cpp DBAPI.cpp Helper.cpp
OBJ_FILES=${C_SRCFILES:.cpp=.o}
.PHONY: clean

$(TARGET): $(OBJ_FILES)
	g++ -o $@ $(OBJ_FILES)

%.o: %.cpp
	g++ -std=c++11 -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f *.o
	rm -f $(TARGET)
