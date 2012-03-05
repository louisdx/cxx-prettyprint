# source file:     prettyprint98.cpp
# header file:     prettyprint98.h
# executable file: prettyprint98
#
objects = prettyprint98.o
sources = prettyprint98.cpp
#
prettyprint98: $(objects)
	g++ -o prettyprint98 $(objects)
#
prettyprint98.o: prettyprint98.h
$(objects): $(sources)
	g++ -c $(sources)
#
clean:
	rm -f *.o prettyprint98
