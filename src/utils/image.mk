$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin\
	$(BUILD)/system.bin\
	$(BUILD)/system.map\
	$(SRC)/utils/master.sfdisk\
	
	yes | bximage -hd=16M -mode=create -sectsize=512 -q $@
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
# 测试system.bin是否小于100K，否则就要修改下面的count
	test -n "$$(find $(BUILD)/system.bin -size -100k)" 
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc
	sfdisk $@ < $(SRC)/utils/master.sfdisk
# 挂载设备
	sudo losetup /dev/loop22 --partscan $@

# 创建 minux 文件系统
	sudo mkfs.minix -1 -n 14 /dev/loop22p1

# 挂载文件系统
	sudo mount /dev/loop22p1 /mnt

# 切换所有者
	sudo chown ${USER} /mnt 

	mkdir -p /mnt/empty
	mkdir -p /mnt/home
	mkdir -p /mnt/d1/d2/d3/d4

# 创建文件
	echo "master / direcotry file..." > /mnt/hello.txt
	echo "master /home direcotry file..." > /mnt/home/hello.txt

# 卸载文件系统
	sudo umount /mnt

# 卸载设备
	sudo losetup -d /dev/loop22

$(BUILD)/slave.img: $(SRC)/utils/slave.sfdisk
	yes | bximage -hd=32M -mode=create -sectsize=512 -q $@
	sfdisk $@ < $(SRC)/utils/slave.sfdisk
# 挂载设备
	sudo losetup /dev/loop22 --partscan $@

# 创建 minux 文件系统
	sudo mkfs.minix -1 -n 14 /dev/loop22p1

# 挂载文件系统
	sudo mount /dev/loop22p1 /mnt

# 切换所有者
	sudo chown ${USER} /mnt 

# 创建文件
	echo "slave root direcotry file..." > /mnt/hello.txt

# 卸载文件系统
	sudo umount /mnt

# 卸载设备
	sudo losetup -d /dev/loop22

.PHONY: mount0
mount0: $(BUILD)/master.img
	sudo losetup /dev/loop22 --partscan $<
	sudo mount /dev/loop22p1 /mnt
	sudo chown ${USER} /mnt 

.PHONY: umount0
umount0: /dev/loop22
	-sudo umount /mnt
	-sudo losetup -d $<

.PHONY: mount1
mount1: $(BUILD)/slave.img
	sudo losetup /dev/loop23 --partscan $<
	sudo mount /dev/loop23p1 /mnt
	sudo chown ${USER} /mnt 

.PHONY: umount1
umount1: /dev/loop23
	-sudo umount /mnt
	-sudo losetup -d $<

IMAGES:= $(BUILD)/master.img \
	$(BUILD)/slave.img

image: $(IMAGES)
