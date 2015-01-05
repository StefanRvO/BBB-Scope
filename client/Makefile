CC=clang++ #Compiler
DEFINES= 
CFLAGS= -c -std=c++11 -Ofast -g  #Compiler Flags
LDFLAGS=-lpthread -lpruio#Linker options
SOURCES= ADC.cpp SampleSender.cpp client.cpp   #cpp files
OBJECTS=$(SOURCES:.cpp=.o)  #Object files
EXECUTEABLE=client #Output name

all: $(SOURCES) $(EXECUTEABLE)
	
$(EXECUTEABLE): $(OBJECTS) 
	$(CC)    $(OBJECTS) -o $(EXECUTEABLE) $(LDFLAGS)

.cpp.o:
	$(CC)  $(CFLAGS) $(DEFINES)   $< -o $@


clean:  ; rm *.o $(EXECUTEABLE) 
