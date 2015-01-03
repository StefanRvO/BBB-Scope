#BBBADC_Remote_Scope


Application to stream data from the BeagleBone Black ADC
to a different computer through a TCP socket
This data is shown on the recivere site in a Occiloscope-like
style (WIP) and give the user the option to save it for later analysis


##Special/uncommon libraries used:
libpruio http://users.freebasic-portal.de/tjf/Projekte/libpruio/doc/html/index.html
install with http://users.freebasic-portal.de/tjf/Projekte/libpruio/doc/html/_cha_preparation.html
Run 'sudo echo BB-BONE-PRU-01 > /sys/devices/bone_capemgr.*/slots' before running any application


 
##Build##

Install the required packages (clang, SDL2, SDL2-gfx,fftw,SDL2-ttf)
run make
