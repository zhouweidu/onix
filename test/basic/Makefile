CFLAGS:= -m32
CFLAGS+= -Qn #去掉gcc的版本信息
# CFLAGS+= -fno-builtin
# CFLAGS+= -nostdinc
CFLAGS+= -fno-pic #不需要位置无关代码
# CFLAGS+= -fno-pie 
# CFLAGS+= -nostdlib 
# CFLAGS+= -fno-stack-protector
CFLAGS+= -fno-asynchronous-unwind-tables
CFLAGS+= -mpreferred-stack-boundary=2
# CFLAGS+= -fomit-frame-pointer
CFLAGS:=$(strip ${CFLAGS}) 

.PHONY: hello.s
hello.s: hello.c
	gcc $(CFLAGS) -S $< -o $@

.PHONY: params.s
params.s: params.c
	gcc $(CFLAGS) -S $< -o $@

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf *.out