MUSIC:= ./utils/deer.mp3

$(BUILD)/mono.wav: $(MUSIC)
	ffmpeg -i $< -ac 1 -ar 44100 -acodec pcm_u8 -y $@

$(BUILD)/stereo.wav: $(MUSIC)
	ffmpeg -i $< -ac 2 -ar 44100 -acodec pcm_s16le -y $@

$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin\
	$(BUILD)/system.bin\
	$(BUILD)/system.map\
	$(SRC)/utils/master.sfdisk\
	$(BUILTIN_APPS)\
	$(BUILD)/mono.wav\
	$(BUILD)/stereo.wav\
	
	yes | bximage -hd=16M -mode=create -sectsize=512 -q $@
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
# 测试system.bin是否小于100K，否则就要修改下面的count，还有loader里面的内容
	test -n "$$(find $(BUILD)/system.bin -size -500k)" 
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=1000 seek=10 conv=notrunc
	sfdisk $@ < $(SRC)/utils/master.sfdisk
# 挂载设备
	sudo losetup /dev/loop22 --partscan $@

# 创建 minux 文件系统
	sudo mkfs.minix -1 -n 14 /dev/loop22p1

# 挂载文件系统
	sudo mount /dev/loop22p1 /mnt

# 切换所有者
	sudo chown ${USER} /mnt 

	mkdir -p /mnt/bin
	mkdir -p /mnt/dev
	mkdir -p /mnt/mnt

# 创建文件
	echo "master / direcotry file..." > /mnt/hello.txt

# 拷贝音频
	mkdir -p /mnt/data
	cp $(BUILD)/mono.wav /mnt/data
	cp $(BUILD)/stereo.wav /mnt/data

# 拷贝程序
	for app in $(BUILTIN_APPS); \
	do \
		cp $$app /mnt/bin; \
	done

# 卸载文件系统
	sudo umount /mnt

# 卸载设备
	sudo losetup -d /dev/loop22

$(BUILD)/floppya.img:

# 创建一个 1.44M 的软盘镜像
	yes | bximage -q -fd=1.44M -mode=create -sectsize=512 -imgmode=flat $@

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
	$(BUILD)/slave.img \
	$(BUILD)/floppya.img

image: $(IMAGES)
