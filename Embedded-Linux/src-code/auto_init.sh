#挂载命令：
mount -t nfs -o nolock,vers=3 192.168.5.11:/home/book/nfs_rootfs /mnt 

mount -t nfs -o nolock,vers=3 192.168.5.11:/home/book/Desktop/myfolder/wdslinux_exefile /mnt 

#打开GUI
#systemctl start myir

#关闭GUI
systemctl stop myir

#让屏幕不自动熄灭
echo -e "\033[9;0]" > /dev/tty0