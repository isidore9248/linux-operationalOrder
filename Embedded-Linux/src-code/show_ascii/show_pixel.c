#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

static int fd_fb;
static struct fb_var_screeninfo var;	/* Current var */
static int screen_size;
static unsigned char *fb_base;
static unsigned int line_width;
static unsigned int pixel_width;

/**********************************************************************
 * 函数名称： lcd_put_pixel
 * 功能描述： 在LCD指定位置上输出指定颜色（描点）
 * 输入参数： x坐标，y坐标，颜色
 * 输出参数： 无
 * 返 回 值： 会
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/05/12	     V1.0	  zh(angenao)	      创建
 ***********************************************************************/ 
void lcd_put_pixel(int x, int y, unsigned int color)
{
	unsigned char *pen_8 = fb_base+y*line_width+x*pixel_width;
	unsigned short *pen_16;	
	unsigned int *pen_32;	

	unsigned int red, green, blue;	

	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch (var.bits_per_pixel)
	{
		case 8:
		{
			*pen_8 = color;
			break;
		}
		case 16:
		{
			/* 565 */
			red   = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			break;
		}
		case 32:
		{
			*pen_32 = color;
			break;
		}
		default:
		{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
			break;
		}
	}
}


//*dev/fb0通常Linux系统中常见的帧缓冲设备文件
//*通常用于访问系统的主显示屏
int main(int argc, char **argv)
{
	int i;
	
	//以读写方式打开/dev/fb0
	fd_fb = open("/dev/fb0", O_RDWR);
	if (fd_fb < 0)	//检查是否成功打开
	{
		printf("can't open /dev/fb0\n");
		return -1;
	}
	//获取屏幕信息
	//*将信息存储在var中
	if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var))
	{
		printf("can't get var\n");
		return -1;
	}
	
	//计算每行宽度、每个像素的宽度和屏幕总大小。
	line_width  = var.xres * var.bits_per_pixel / 8;
	pixel_width = var.bits_per_pixel / 8;
	screen_size = var.xres * var.yres * var.bits_per_pixel / 8;

	//?将帧缓冲设备的内容直接映射到进程的内存中
	//?使得可以通过指针 fb_base 直接访问和修改帧缓冲设备的数据。
	fb_base = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
	if (fb_base == (unsigned char *)-1)
	{
		printf("can't mmap\n");
		return -1;
	}

	/* 清屏: 全部设为白色 */
	memset(fb_base, 0xff, screen_size);

	/* 随便设置出100个为红色 */
	for (i = 0; i < 100; i++)
		//*var.xres/2屏幕中心的 x 坐标
		//*var.yres/2屏幕中心的 y 坐标
		lcd_put_pixel(var.xres/2+i, var.yres/2, 0xFF0000);	//RRGGBB
	
	//*用于解除之前通过 mmap 映射的内存区域。
	munmap(fb_base , screen_size);
	close(fd_fb);
	
	return 0;	
}

