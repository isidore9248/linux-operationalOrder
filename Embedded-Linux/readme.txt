开发板ip设置：
修改文件： /etc/systemd/network/50-static.network
[Match]
 Name=eth0
[Network]
 Address=192.168.5.9/24
Gateway=192.168.5.1


桥接ip：
windows：5.10
ubuntu：5.11
linux开发板：5.9

挂载命令：
mount -t nfs -o nolock,vers=3 192.168.5.11:/home/book/nfs_rootfs /mnt 

mount -t nfs -o nolock,vers=3 192.168.5.11:/home/book/Desktop/myfolder/wdslinux_exefile /mnt 

mount -t nfs -o nolock,vers=3 192.168.5.11:/home/book/Desktop/01_all_series_quickstart/04_嵌入式Linux应用开发基础知识/source/08_show_ascii /mnt 


开发板编译命令：
arm-buildroot-linux-gnueabihf-gcc -o <可执行文件名> <c文件名.c>

Makefile内核路径：
/home/book/100ask_stm32mp157_pro-sdk/Linux-5.4

打开GUI
systemctl start myir

关闭GUI
systemctl stop myir

让屏幕不自动熄灭
echo -e "\033[9;0]" > /dev/tty0