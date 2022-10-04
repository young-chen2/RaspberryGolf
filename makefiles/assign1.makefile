# This will run by default with command `make`.
all: build/larson.bin

build/larson.bin: src/apps/larson.s | build
	arm-none-eabi-as src/apps/larson.s -o build/larson.o
	arm-none-eabi-objcopy build/larson.o -O binary build/larson.bin

build:
	mkdir -p build

# Upload to the Pi.
run: build/larson.bin
	rpi-run.py build/larson.bin

# Remove binary files with `make clean`.
clean:
	rm -rf build

# Check for clean build.
check:
	@! make clean all 2>&1 > /dev/null | grep 'error\|warning'
