API=../../api
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
AR=arm-none-eabi-ar
CFLAGS=-DNDEBUG -ggdb3 -I$(API) -Os -mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard -fno-common -fdata-sections -ffunction-sections -fno-exceptions -fno-strict-aliasing -fno-common -fno-threadsafe-statics
LDFLAGS=-Wl,-L$(API) -Wl,--gc-sections -Wl,--entry=entrypoint --specs=nano.specs -nostartfiles -Wl,-Ur -lc -lstdc++ -lm -lapi -lnosys

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

app.elf: src/main.o src/lib/libgiac.a src/lib/libmpfi.a src/lib/libmpfr.a src/lib/libgmp.a src/lib/libmicropy.a
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

src/lib/libgmp.a src/lib/libmpfi.a src/lib/libmpfr.a src/lib/libmicropy.a src/include/gmp.h src/include/mpfi.h src/include/mpfr.h:
	./buildDeps


	./configure --with-mpfr=$(shell pwd)/src/ --with-gmp=$(shell pwd)/src/ --host=arm-none-eabi CFLAGS="-Os -fdata-sections -ffunction-sections -DNDEBUG -mthumb -march=armv7e-m -mfloat-abi=hard -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -fno-strict-aliasing --specs=nosys.specs"
# Build the library
	make
# Copy the library to the correct location
	cp src/.libs/libmpfi.a ../lib/
# Copy the header files to the correct location
	cp src/mpfi.h ../include/mpfi.h
	# rm -r build-aux
	# rm -r m4

src/lib/libmpfr.a src/include/mpfr.h: src/lib/libgmp.a src/include/gmp.h libDirectory
	cd src/mpfr-4.1.0
#	Rebuild the Makefile because cleanall removed it.
	./configure --with-gmp=$(shell pwd)/src/ --host=arm-none-eabi --disable-shared CFLAGS="-Os -fdata-sections -ffunction-sections -DNDEBUG -mthumb -march=armv7e-m -mfloat-abi=hard -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -fno-strict-aliasing --specs=nosys.specs"
	autoreconf
# Now, we can build the library
	make
# Copy the library to the correct location
	cp src/.libs/libmpfr.a ../lib/
# Copy the header files to the correct location
	cp src/mpfr.h ../include/mpfr.h
src/lib/libmicropy.a: libDirectory
	cd src/micropython-1.12/numworks

# Due to the update of compiler, we must ignore the error
	sed -i 's/\$$(CC) \$$(CFLAGS)/\$$(CC) \$$(CFLAGS) -Wno-error/g' ../py/mkrules.mk || true
# We need to use a script to build MicroPython, because the make command crashes in MicroPython because it tries to link without the required libraries
	./mklib

src/lib/libgiac.a: src/lib/libgmp.a src/lib/libmpfi.a src/lib/libmpfr.a src/include/gmp.h src/include/mpfi.h src/include/mpfr.h libDirectory
	cd src/giac-1.6.0/src
	make -f Makefile.numworks.n0110
	cp libgiac.a ../../lib/

clean:
	rm -f *.elf src/*.o src/include/*.h src/lib/*.a
