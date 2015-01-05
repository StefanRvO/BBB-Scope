#include <sys/types.h>
#include <cstdio>
#include <netdb.h>
#include "SampleSender.h"
#include "../server/RingBuffer.h"
#include "../server/structures.h"
#include "ADCOptions.h"
#include "ADC.h"
int main(int argc, char **argv)
{
    //Setup network
    hostent *he;
    if (argc != 2) {
        fprintf(stderr, "Usage: client hostname\n");
        exit(1);
    }

    if ((he = gethostbyname(argv[1]))==NULL) {
        fprintf(stderr, "Cannot get host name\n");
        exit(1);
    }
    ADCOptions options;
    RingBuffer<sample,1000000> RB;
    ADC Adc(&options, &RB);
    SampleSender SS(&options, &RB, 3490, 3491, he);
    SS.sampleSocketThread();
}
