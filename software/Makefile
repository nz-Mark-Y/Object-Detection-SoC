#
PLATFORM = ARM

#
ifeq ($(PLATFORM),ARM)
	CROSS_COMPILE = arm-linux-gnueabihf-
	CFLAGS = -g -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include -I \
		${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include/soc_cv_av
	LDFLAGS =  -g -Wall
	CC = $(CROSS_COMPILE)gcc -std=c99 -DARM -D_XOPEN_SOURCE=500
	ARCH= arm
else
	CC = gcc -std=c99
	CFLAGS=-Wall -O3 -pedantic -DECRYPT_API="ecrypt-sync.h"
endif

TARGET=trivium-test
LOAD_DIRECTORY=F:

default: $(TARGET)

ecrypt-test: ecrypt-test.o ecrypt-sync.o my_trivium.o

trivium-test: trivium-test.o ecrypt-sync.o my_trivium.o my_trivium.h

ecrypt-test.o: ecrypt-sync.h
ecrypt-sync.o: ecrypt-sync.h
trivium.o: ecrypt-sync.h
my_trivium.o: ecrypt-sync.h my_trivium.h trivium.c

.PHONEY: load
load: $(TARGET)
	cp -f $< $(LOAD_DIRECTORY)/$(TARGET)

CLEAN_TARGETS = ecrypt-test trivium-test
.PHONY: clean
clean:
	rm -f *.o *.exe *~ $(CLEAN_TARGETS)
