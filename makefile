CFLAGS = -O2 -Wall -Wextra -Wdeclaration-after-statement
CPPFLAGS = -O2 -Wall -Wextra

# need zxcvbn.h prior to package installation
CFLAGS += -I.
CPPFLAGS += -I.

# library metadata
TARGET_LIB = libzxcvbn.so.0.0.0
SONAME = libzxcvbn.so.0

WORDS = words-10k-pass.txt words-english.txt words-female.txt words-male.txt words-surname.txt

all: test-file test-inline test-c++inline test-c++file test-shlib test-statlib

test-shlib: test.c $(TARGET_LIB)
	if [ ! -e libzxcvbn.so ]; then ln -s $(TARGET_LIB) libzxcvbn.so; fi
	gcc $(CFLAGS) -o $@ $< -L. -lzxcvbn -lm

$(TARGET_LIB): zxcvbn-inline-pic.o
	gcc $(CFLAGS) $(LDFLAGS) -fPIC -shared -Wl,-soname,$(SONAME) -o $@ $^ -lm
	if [ ! -e $(SONAME) ]; then ln -s $(TARGET_LIB) $(SONAME); fi

test-statlib: test.c libzxcvbn.a
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^ -lm

libzxcvbn.a: zxcvbn-inline.o
	ar cvq $@ $^

test-file: test.c zxcvbn-file.o
	gcc $(CFLAGS) -DUSE_DICT_FILE -o test-file test.c zxcvbn-file.o -lm

zxcvbn-file.o: zxcvbn.c dict-crc.h zxcvbn.h
	gcc $(CFLAGS) -DUSE_DICT_FILE -c -o zxcvbn-file.o zxcvbn.c

test-inline: test.c zxcvbn-inline.o
	gcc $(CFLAGS) -o test-inline test.c zxcvbn-inline.o -lm

zxcvbn-inline-pic.o: zxcvbn.c dict-src.h zxcvbn.h
	gcc $(CFLAGS) -fPIC -c -o $@ $<

zxcvbn-inline.o: zxcvbn.c dict-src.h zxcvbn.h
	gcc $(CFLAGS) -c -o zxcvbn-inline.o zxcvbn.c

dict-src.h: dictgen $(WORDS)
	./dictgen -o dict-src.h $(WORDS)

dict-crc.h: dictgen $(WORDS)
	./dictgen -b -o zxcvbn.dict -h dict-crc.h $(WORDS)

dictgen: dict-generate.cpp makefile
	g++ -std=c++11 $(CPPFLAGS) -o dictgen dict-generate.cpp

test-c++inline: test.c zxcvbn-c++inline.o
	if [ ! -e test.cpp ]; then ln -s test.c test.cpp; fi
	g++ $(CPPFLAGS) -o test-c++inline test.cpp zxcvbn-c++inline.o -lm

zxcvbn-c++inline.o: zxcvbn.c dict-src.h zxcvbn.h
	if [ ! -e zxcvbn.cpp ]; then ln -s zxcvbn.c zxcvbn.cpp; fi
	g++ $(CPPFLAGS) -c -o zxcvbn-c++inline.o zxcvbn.cpp

test-c++file: test.c zxcvbn-c++file.o
	if [ ! -e test.cpp ]; then ln -s test.c test.cpp; fi
	g++ $(CPPFLAGS) -DUSE_DICT_FILE -o test-c++file test.cpp zxcvbn-c++file.o -lm

zxcvbn-c++file.o: zxcvbn.c dict-crc.h zxcvbn.h 
	if [ ! -e zxcvbn.cpp ]; then ln -s zxcvbn.c zxcvbn.cpp; fi
	g++ $(CPPFLAGS) -DUSE_DICT_FILE -c -o zxcvbn-c++file.o zxcvbn.cpp

test: test-file test-inline test-c++inline test-c++file test-shlib test-statlib testcases.txt
	@echo Testing C build, dictionary from file
	./test-file -t testcases.txt
	@echo Testing C build, dictionary in executable
	./test-inline -t testcases.txt
	@echo Testing C shlib, dictionary in shlib
	LD_LIBRARY_PATH=. ./test-shlib -t testcases.txt
	@echo Testing C static lib, dictionary in lib
	./test-statlib -t testcases.txt
	@echo Testing C++ build, dictionary from file
	./test-c++file -t testcases.txt
	@echo Testing C++ build, dictionary in executable
	./test-c++inline -t testcases.txt
	@echo Finished

clean:
	rm -f test-file zxcvbn-file.o test-c++file zxcvbn-c++file.o 
	rm -f test-inline zxcvbn-inline.o zxcvbn-inline-pic.o test-c++inline zxcvbn-c++inline.o
	rm -f dictgen
	rm -f ${TARGET_LIB} ${SONAME} libzxcvbn.so test-shlib libzxcvbn.a test-statlib
