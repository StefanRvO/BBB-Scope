CC=clang++ #Compiler
DEFINES= 
CFLAGS= -c -std=c++11 -Ofast -g  #Compiler Flags
LDFLAGS=-lpthread -lSDL2 -lSDL2_ttf -lSDL2_gfx -lfftw3 -lfftw3_threads -lm#Linker options
SOURCES= server.cpp SampleGrabber.cpp UIDrawer.cpp EventHandler.cpp Primitives.cpp TextDrawer.cpp PeriodFinder.cpp   #cpp files
OBJECTS=$(SOURCES:.cpp=.o)  #Object files
EXECUTEABLE=server #Output name

all: $(SOURCES) $(EXECUTEABLE)
	
$(EXECUTEABLE): $(OBJECTS) 
	$(CC)    $(OBJECTS) -o $(EXECUTEABLE) $(LDFLAGS)

.cpp.o:
	$(CC)  $(CFLAGS) $(DEFINES)   $< -o $@


clean:  ; rm *.o $(EXECUTEABLE) 
