  # the compiler: gcc for C program, g++ for C++
  CC = g++

  # compiler flags:
  #  -g                           adds debugging information to the executable file
  #  -Wall                        enables most, but not all, compiler warnings
  #  --std=c++11                  enables C++11 functionality
  #  -fslp-vectorize-aggressive   enables SLP Vectorizer which merges multiple scalars into vectors
  #  -march=native                enables GCC optimization for local CPU instruction sets
  CFLAGS  = -g -Wall -Werror --std=c++11 -O3 -fslp-vectorize-aggressive -march=native

  # the build target executables:
  TARGET1 = hexagonalSearch

  all: $(TARGET1)

  $(TARGET1): $(TARGET1).cpp $(DEPS1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(TARGET1).cpp

  clean:
	$(RM) $(TARGET1)
