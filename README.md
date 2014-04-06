#BBBADC_Remote_Scope


Application to stream data from the BeagleBone Black ADC
to a different computer through a TCP socket
This data should be shown on the recivere site in a Occiloscope-like
style (Not Done) and give the user the option to save it for later analysis


##Special/uncommon libraries used and included:
VegetableAvengers BBBiolib (https://github.com/VegetableAvenger/BBBIOlib)

Tcp code inspired by ueg1990 (http://codereview.stackexchange.com/questions/13461/two-way-communication-in-tcp-server-client-implementation)

sagfault handling by Navrocky Vladislav (segcatch)
 
 
 
 compile server with:
 g++ server.c -o server -pthread -std=c++11 -lalleg -g -ggdb3 -Wall -fexceptions -fnon-call-exceptions  -rdynamic -lsegvcatch
