# 不要使用bochs，环境没配好
.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q

QEMU:= qemu-system-i386 
QEMU+= -m 32M 
QEMU+= -soundhw pcspk 
QEMU+= -rtc base=localtime 
QEMU+= -drive file=$(BUILD)/master.img,if=ide,index=0,media=disk,format=raw 
QEMU+= -drive file=$(BUILD)/slave.img,if=ide,index=1,media=disk,format=raw 

QEMU_DISK:= -boot c
QEMU_DEBUG:= -s -S
.PHONY: qemu
qemu: $(IMAGES)
	$(QEMU) $(QEMU_DISK)

.PHONY: qemug
qemug: $(IMAGES)
	$(QEMU) $(QEMU_DEBUG)

$(BUILD)/master.vmdk: $(BUILD)/master.img
	qemu-img convert -O vmdk $< $@

.PHONY: vmdk
vmdk: $(BUILD)/master.vmdk