#!/bin/bash
GRUB_PATH=/home/george/Git/grub-legacy/usr

qemu-img create -f raw george.disk 32760K

echo ';;;*' | sfdisk george.disk -D -C 65 -H 16 -S 63
sudo losetup -o 32256 /dev/loop0 george.disk
sudo mke2fs /dev/loop0
sudo losetup -d /dev/loop0

mkdir disk
sudo mount -t ext2 george.disk disk -o loop,offset=32256
sudo mkdir disk/boot
sudo cp menu.lst disk/boot/
sudo cp george.multiboot disk/boot/george.multiboot
sudo cp ${GRUB_PATH}/lib/grub/i386-pc/{stage1,e2fs_stage1_5,stage2} disk/boot/

sudo ${GRUB_PATH}/sbin/grub << "EOF"
device (hd0) george.disk
geometry (hd0) 65 16 63
root (hd0,0)
setup --prefix=/boot (hd0)
quit
EOF
sudo umount disk
rmdir disk
