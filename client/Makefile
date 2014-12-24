
LIB_PATH = ./BBBIOlib/BBBio_lib/
ADC_PATH=./
#TOOLKIT_PATH = ./Toolkit/
#LAB_PATH = ./Lab/


LIBRARIES = BBBio

all : libBBBio.a

libBBBio.a : ${LIB_PATH}BBBiolib.c ${LIB_PATH}BBBiolib.h BBBiolib_PWMSS.o BBBiolib_McSPI.o BBBiolib_ADCTSC.o
	gcc -c ${LIB_PATH}BBBiolib.c -o ${LIB_PATH}BBBiolib.o
	ar -rs ${LIB_PATH}libBBBio.a ${LIB_PATH}BBBiolib.o ${LIB_PATH}BBBiolib_PWMSS.o ${LIB_PATH}BBBiolib_McSPI.o ${LIB_PATH}BBBiolib_ADCTSC.o
	cp ${LIB_PATH}libBBBio.a ./

BBBiolib_PWMSS.o : ${LIB_PATH}BBBiolib_PWMSS.c ${LIB_PATH}BBBiolib_PWMSS.h
	gcc -c ${LIB_PATH}BBBiolib_PWMSS.c -o ${LIB_PATH}BBBiolib_PWMSS.o

BBBiolib_McSPI.o : ${LIB_PATH}BBBiolib_McSPI.c ${LIB_PATH}BBBiolib_PWMSS.h
	gcc -c ${LIB_PATH}BBBiolib_McSPI.c -o ${LIB_PATH}BBBiolib_McSPI.o

BBBiolib_ADCTSC.o : ${LIB_PATH}BBBiolib_ADCTSC.c ${LIB_PATH}BBBiolib_ADCTSC.h
	gcc -c ${LIB_PATH}BBBiolib_ADCTSC.c -o ${LIB_PATH}BBBiolib_ADCTSC.o

#---------------------------------------------------
# Demo
#---------------------------------------------------

ADC : ${ADC_PATH}ADC.c libBBBio.a
	gcc -o ADC ${ADC_PATH}/ADC.c -L ${LIB_PATH} -lBBBio -lm

.PHONY: clean
clean :
	rm -rf ${LIB_PATH}*.o ${LIB_PATH}libBBBio.a libBBBio.a LED ADT7301 GPIO_CLK_status SevenScan Ultrasonic28015 TMP SMOTOR LED_GPIO Debouncing 4x4keypad EP_status PWM RA ADXL345 ADC ADC_CALC

