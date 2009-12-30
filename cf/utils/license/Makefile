CC = g++
CFLAGS = -c -Wall -DCF_LICENSE_GENERATOR
LDFLAGS = 
INCLUDES = -I../../common

LICENSE_SOURCES = main.cpp license.cpp
LICENSE_OBJECTS = $(LICENSE_SOURCES:.cpp=.o)

license: $(LICENSE_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(LICENSE_OBJECTS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

