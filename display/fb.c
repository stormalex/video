#include <config.h>
#include <disp_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>

static int fb_show_pixel(int iX, int iY, unsigned int color);
static int fb_clear_screen(unsigned int back_color);
static int fb_device_init(void);
static int fb_show_page(pixel_datas_p pixel_data);

static int g_fd;

static struct fb_var_screeninfo g_fb_var;
static struct fb_fix_screeninfo g_fb_fix;			
static unsigned char *g_fb_mem;
static unsigned int g_screen_size;

static unsigned int g_line_width;
static unsigned int g_pixel_width;

static disp_opts_t g_fb_opts = {
	.name        = "fb",
	.device_init  = fb_device_init,
	.show_pixel   = fb_show_pixel,
	.clear_screen = fb_clear_screen,
	.show_page    = fb_show_page,
};


/**********************************************************************
 * name£º fb_show_pixel
 * description£º show pixel
 * input parameters£º x, y, color
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
static int fb_show_pixel(int x, int y, unsigned int color)
{
	unsigned char *fb_mem;
	unsigned short *fb_16bpp;
	unsigned int *fb_32bpp;
	unsigned short color_16bpp; /* 565 */
	int red;
	int green;
	int blue;

	if ((x >= g_fb_var.xres) || (y >= g_fb_var.yres))
	{
		printf("out of region\n");
		return -1;
	}

	fb_mem      = g_fb_mem + g_line_width * y + g_pixel_width * x;
	fb_16bpp  = (unsigned short *)fb_mem;
	fb_32bpp = (unsigned int *)fb_mem;
	
	switch (g_fb_var.bits_per_pixel)
	{
		case 8:
		{
			*fb_mem = (unsigned char)color;
			break;
		}
		case 16:
		{
			red   = (color >> (16+3)) & 0x1f;
			green = (color >> (8+2)) & 0x3f;
			blue  = (color >> 3) & 0x1f;
			color_16bpp = (red << 11) | (green << 5) | blue;
			*fb_16bpp	= color_16bpp;
			break;
		}
		case 32:
		{
			*fb_32bpp = color;
			break;
		}
		default :
		{
			printf("can't support %d bpp\n", g_fb_var.bits_per_pixel);
			return -1;
		}
	}

	return 0;
}



/**********************************************************************
 * name£º fb_clear_screen
 * description£º clean fb
 * input parameters£º back_color
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
static int fb_clear_screen(unsigned int back_color)
{
	unsigned char *fb_mem;
	unsigned short *fb_16bpp;
	unsigned int *fb_32bpp;
	unsigned short color_16bpp; /* 565 */
	int red;
	int green;
	int blue;
	int i = 0;

	fb_mem      = g_fb_mem;
	fb_16bpp  = (unsigned short *)fb_mem;
	fb_32bpp = (unsigned int *)fb_mem;

	switch (g_fb_var.bits_per_pixel)
	{
		case 8:
		{
			memset(g_fb_mem, back_color, g_screen_size);
			break;
		}
		case 16:
		{
			red   = (back_color >> (16+3)) & 0x1f;
			green = (back_color >> (8+2)) & 0x3f;
			blue  = (back_color >> 3) & 0x1f;
			color_16bpp = (red << 11) | (green << 5) | blue;
			while (i < g_screen_size)
			{
				*fb_16bpp	= color_16bpp;
				fb_16bpp++;
				i += 2;
			}
			break;
		}
		case 32:
		{
			while (i < g_screen_size)
			{
				*fb_32bpp	= back_color;
				fb_32bpp++;
				i += 4;
			}
			break;
		}
		default :
		{
			printf("can't support %d bpp\n", g_fb_var.bits_per_pixel);
			return -1;
		}
	}

	return 0;
}


/**********************************************************************
 * name£º fb_show_page
 * description£º show pixel_data on fb
 * input parameters£º pixel_data
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
static int fb_show_page(pixel_datas_p pixel_data)
{
	if(g_fb_opts.disp_mem != pixel_data->aucpixel_datas)
	{
		memcpy(g_fb_opts.disp_mem, pixel_data->aucpixel_datas, pixel_data->total_bytes);
	}
	return 0;
}

/**********************************************************************
 * name£º fb_init
 * description£º init fb device
 * input parameters£º void
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
static int fb_device_init(void)
{
	int ret;
	
	g_fd = open(FB_DEVICE_NAME, O_RDWR);
	if (0 > g_fd)
	{
		printf("can't open %s\n", FB_DEVICE_NAME);
	}

	ret = ioctl(g_fd, FBIOGET_VSCREENINFO, &g_fb_var);
	if (ret < 0)
	{
		printf("can't get fb's var\n");
		return -1;
	}

	ret = ioctl(g_fd, FBIOGET_FSCREENINFO, &g_fb_fix);
	if (ret < 0)
	{
		printf("can't get fb's fix\n");
		return -1;
	}
	
	g_screen_size = g_fb_var.xres * g_fb_var.yres * g_fb_var.bits_per_pixel / 8;
	g_fb_mem = (unsigned char *)mmap(NULL , g_screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
	if (0 > g_fb_mem)	
	{
		printf("can't mmap\n");
		return -1;
	}

	g_fb_opts.x_res		 	= g_fb_var.xres;
	g_fb_opts.y_res			= g_fb_var.yres;
	g_fb_opts.bpp			= g_fb_var.bits_per_pixel;
	g_fb_opts.line_width	= g_fb_var.xres * g_fb_opts.bpp/ 8;
	g_fb_opts.disp_mem  	= g_fb_mem;

	g_line_width  = g_fb_var.xres * g_fb_var.bits_per_pixel / 8;
	g_pixel_width = g_fb_var.bits_per_pixel / 8;
	
	return 0;
}


/**********************************************************************
 * name£º fb_init
 * description£º init fb display
 * input parameters£º void
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
int fb_init(void)
{
	return register_disp_opts(&g_fb_opts);
}

