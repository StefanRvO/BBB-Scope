CC=clang++ #Compiler
CFLAGS= -c -std=c++11  -Wall -g  #Compiler Flags
LDFLAGS=-lpthread -lSDL2#Linker options
SOURCES= server.cpp SampleGrabber.cpp UIDrawer.cpp EventHandler.cpp Primitives.cpp   #cpp files
OBJECTS=$(SOURCES:.cpp=.o)  #Object files
EXECUTEABLE=server #Output name

all: $(SOURCES) $(EXECUTEABLE)
	
$(EXECUTEABLE): $(OBJECTS) 
	$(CC)    $(OBJECTS) -o $(EXECUTEABLE) $(LDFLAGS)

.cpp.o:
	$(CC)  $(CFLAGS)   $< -o $@


clean:  ; rm *.o $(EXECUTEABLE) 
