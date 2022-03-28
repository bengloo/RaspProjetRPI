MAIN=temple_run
EXEC=$(MAIN)_
op=-Wall -Wextra #opertion et retoure de la compilation

TARGET_PI=../target_pi
CFLAGS_PI=-I$(TARGET_PI)/include
LDFLAGS_PI=-L$(TARGET_PI)/lib
GCC_PI=../tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

IP=172.24.1.1

all:PC PI

debug: op += -DDEBUG
debug: all

undraw: op += -DUNDRAW
undraw: op += -DDEBUG
undraw: PI_sur_PI

#compilation de la verssion PC.
PC:
	@echo "Generation de l'executable pc"
	gcc $(MAIN).c -o $(EXEC)pc $(op) -lX11 -lm
	@echo ""

#compilation de la version PI compilé en croisé.
PI: op += -DPI
PI:
	@echo "generation de l'executable pour le raspberriPi en cross compilation depuis le pc"
	$(GCC_PI) $(CFLAGS_PI) $(LDFLAGS_PI) $(MAIN).c -o $(EXEC)pi $(op) -lm -lwiringPi -std=c99
	@echo ""

#compilation de la version PI non croisé
PI_sur_PI: op += -DPI
PI_sur_PI:
	@echo "generation de l'executable pour le raspberriPi sur le raspberriPi"
	gcc $(CFLAGS_PI) $(LDFLAGS_PI) $(MAIN).c -o $(EXEC)pi $(op) -lm -lwiringPi
	@echo ""

clean:
	rm -f $(EXEC)*
	rm -f *.o

import:importCode importTarg
	
importCode:PI
	scp -r ../codesource pi@$(IP):Documents
	
importTarg:
	scp -r ../target_pi pi@$(IP):Documents
