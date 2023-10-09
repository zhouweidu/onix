# 不要使用bochs，环境没配好
.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q

QEMU:= qemu-system-i386 
QEMU+= -m 32M 
QEMU+= -soundhw pcspk #pcspeaker
QEMU+= -audiodev pa,id=snd # Pause Audio backend
QEMU+= -device sb16,audiodev=snd # Sound Blaster 16
QEMU+= -rtc base=localtime 
QEMU+= -chardev stdio,mux=on,id=com1 # 字符设备 1
# QEMU+= -chardev udp,id=com1,port=6666,ipv4=on # 字符设备 1
# QEMU+= -chardev vc,mux=on,id=com2 # 字符设备 2
# QEMU+= -chardev udp,id=com2,port=7777,ipv4=on # 字符设备 2
QEMU+= -serial chardev:com1 # 串口 1
# QEMU+= -serial chardev:com2 # 串口 2

QEMU_DISK := -drive file=$(BUILD)/master.img,if=ide,index=0,media=disk,format=raw # 主硬盘
QEMU_DISK += -drive file=$(BUILD)/slave.img,if=ide,index=1,media=disk,format=raw # 从硬盘
QEMU_DISK += -drive file=$(BUILD)/floppya.img,if=floppy,index=0,media=disk,format=raw # 软盘a

QEMU_DISK_BOOT:= -boot c
QEMU_DEBUG:= -s -S

.PHONY: qemu
qemu: $(IMAGES)
	$(QEMU) $(QEMU_DISK) $(QEMU_DISK_BOOT)

.PHONY: qemug
qemug: $(IMAGES)
	$(QEMU) $(QEMU_DISK) $(QEMU_DISK_BOOT) $(QEMU_DEBUG)

$(BUILD)/master.vmdk: $(BUILD)/master.img
	qemu-img convert -O vmdk $< $@

.PHONY: vmdk
vmdk: $(BUILD)/master.vmdk