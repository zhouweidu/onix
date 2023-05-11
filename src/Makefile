BUILD:=../build
SRC:=.

ENTRYPOINT:=0x10000

CFLAGS:= -m32
CFLAGS+= -fno-builtin
CFLAGS+= -nostdinc
CFLAGS+= -fno-pic #不需要位置无关代码
CFLAGS+= -fno-pie 
CFLAGS+= -nostdlib 
CFLAGS+= -fno-stack-protector
CFLAGS:=$(strip ${CFLAGS}) 


DEBUG:= -g

INCLUDE:= -I$(SRC)/include

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 -gdwarf $< -o $@

$(BUILD)/%.o: $(SRC)/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@


$(BUILD)/kernel.bin: \
	$(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/console.o \
	$(BUILD)/kernel/printk.o \
	$(BUILD)/kernel/assert.o \
	$(BUILD)/kernel/debug.o \
	$(BUILD)/kernel/global.o \
	$(BUILD)/kernel/task.o \
	$(BUILD)/kernel/schedule.o \
	$(BUILD)/kernel/interrupt.o \
	$(BUILD)/kernel/handler.o \
	$(BUILD)/kernel/clock.o \
	$(BUILD)/kernel/time.o \
	$(BUILD)/kernel/rtc.o \
	$(BUILD)/kernel/memory.o \
	$(BUILD)/lib/bitmap.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/lib/vsprintf.o \
	$(BUILD)/lib/stdlib.o \

	$(shell mkdir -p $(dir $@))
	ld -m elf_i386 -static $^ -o $@ -Ttext $(ENTRYPOINT)

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin\
	$(BUILD)/system.bin\
	$(BUILD)/system.map\
	
	yes | bximage -hd=16M -mode=create -sectsize=512 -q $@
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
# 测试system.bin是否小于100K，否则就要修改下面的count
	test -n "$$(find $(BUILD)/system.bin -size -100k)" 
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

.PHONY:test
test: $(BUILD)/master.img


.PHONY: clean
clean:
	rm -rf $(BUILD)

# 不要使用bochs，环境没配好
.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q

QEMU:=qemu-system-i386 \
	-m 32M \
	-boot c \
	-drive file=$(BUILD)/master.img,if=ide,index=0,media=disk,format=raw \
	-soundhw pcspk \
	-rtc base=localtime \

.PHONY: qemu
qemu: $(BUILD)/master.img
	$(QEMU)

.PHONY: qemug
qemug: $(BUILD)/master.img
	$(QEMU) -s -S

$(BUILD)/master.vmdk: $(BUILD)/master.img
	qemu-img convert -sO vmdk $< $@

.PHONY: vmdk
vmdk: $(BUILD)/master.vmdk