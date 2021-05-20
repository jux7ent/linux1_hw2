rmmod miniFS_module.ko || true && make && dmesg --clear && insmod miniFS_module.ko && dmesg
