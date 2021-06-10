rs485:main.c http.c cJSON.c comport.c
	arm-linux-gnueabihf-gcc -o rs485 main.c http.c cJSON.c comport.c
#	gcc -o main main.c http.c cJSON.c comport.c

.PHONY : clean
clean :
	-rm main
