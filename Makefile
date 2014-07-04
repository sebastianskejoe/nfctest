all:
	cd lib/NFC/ && go run generate.go
	ino build
	ino upload

ng:
	ino build
	ino upload

nu:
	cd lib/NFC/ && go run generate.go
	ino build

### TEST
LIBRARY_PATH=/usr/share/arduino/libraries
HW_PATH=/usr/share/arduino/hardware/arduino

NFCLIB=lib/NFC
LIBS=$(LIBRARY_PATH)/LiquidCrystal_old\
	 $(HW_PATH)/cores/arduino\
	 /usr/avr/include\
	 $(HW_PATH)/variants/standard\
	 $(NFCLIB)

.PHONY: all test ng nu

test:
	$(MAKE) -C $(NFCLIB)
	g++ -g $(NFCLIB)/NFC.cpp test/test.c -o test_nfc $(foreach d, $(LIBS), -I $d) -Wwrite-strings -DTEST
