TARGETS = example1 example2 example3

CXXFLAGS = -g

all:	$(TARGETS)


clean:
	rm -f *.o $(TARGETS)


example1:	example1.o decodeJSON.o
	$(CXX) example1.o decodeJSON.o -o example1

example2:	example2.o decodeJSON.o JSONschema.o
	$(CXX) example2.o decodeJSON.o JSONschema.o -o example2

example3:	example3.o decodeJSON.o JSONschema.o
	$(CXX) example3.o decodeJSON.o JSONschema.o -o example3
