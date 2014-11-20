CC=clang++ #Compiler
CFLAGS= -c -std=c++11  -Wall -g  #Compiler Flags
LDFLAGS=-lpthread -lallegro -lallegro_primitives -lallegro_ttf -lallegro_font#Linker options
SOURCES= server.cpp SampleGrabber.cpp EventHandler.cpp   #cpp files
OBJECTS=$(SOURCES:.cpp=.o)  #Object files
EXECUTEABLE=server #Output name

all: $(SOURCES) $(EXECUTEABLE)
	
$(EXECUTEABLE): $(OBJECTS) 
	$(CC)    $(OBJECTS) -o $(EXECUTEABLE) $(LDFLAGS)

.cpp.o:
	$(CC)  $(CFLAGS)   $< -o $@


clean:  ; rm *.o $(EXECUTEABLE) 
