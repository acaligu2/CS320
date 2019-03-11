#
# Specifiy the target
make:	predictors

# Specify the object files that the target depends on
# Also specify the object files needed to create the executable
predictors:	branchPredict.o
	g++ branchPredict.o -o predictors


# Specify how the object files should be created from source files
branchPredict.o:	branchPredict.cpp
	g++ -c branchPredict.cpp


# Specify the object files and executables that are generated
# and need to be removed to re-compile the whole thing
clean:
	rm -f *.o predictors
