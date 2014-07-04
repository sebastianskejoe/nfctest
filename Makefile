NFCLIB=lib/NFC

all: generate build upload

generate:
	$(MAKE) -C $(NFCLIB)

build:
	ino build

upload:
	ino upload

no-gen: build upload

no-upload: generate build

### TEST
LIBRARY_PATH=/usr/share/arduino/libraries
HW_PATH=/usr/share/arduino/hardware/arduino

LIBS=$(LIBRARY_PATH)/LiquidCrystal_old\
	 $(HW_PATH)/cores/arduino\
	 /usr/avr/include\
	 $(HW_PATH)/variants/standard\
	 $(NFCLIB)

.PHONY: all test ng nu

test:
	$(MAKE) -C $(NFCLIB)
	g++ -g $(NFCLIB)/NFC.cpp test/test.c -o test_nfc $(foreach d, $(LIBS), -I $d) -Wwrite-strings -DTEST
