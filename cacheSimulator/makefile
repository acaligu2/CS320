#
# Specifiy the target
make:	cache-sim

# Specify the object files that the target depends on
# Also specify the object files needed to create the executable
cache-sim:	cacheHits.o
	g++ cacheHits.o -o cache-sim


# Specify how the object files should be created from source files
cacheHits.o:	cacheHits.cpp
	g++ -c cacheHits.cpp


# Specify the object files and executables that are generated
# and need to be removed to re-compile the whole thing
clean:
	rm -f *.o cache-sim
