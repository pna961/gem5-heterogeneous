KERN=fft.c top.c
build: $(FILES)
	clang -O1 -S -target armv7-pc-none-eabi -emit-llvm -I/usr/include/x86_64-linux-gnu $(KERN)

clean:
	rm -f *.ll
