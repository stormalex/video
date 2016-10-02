
#include <config.h>
#include <disp_manager.h>
#include <string.h>

static disp_opts_p g_disp_opts_head;
static disp_opts_p g_default_disp_opts;
static video_mem_p g_video_mem_head;

int fb_init(void);

/**********************************************************************
 * name£º register_disp_opts
 * description£º register a display to display module
 * input parameters£º display operations struct
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
int register_disp_opts(disp_opts_p disp_opts)
{
	disp_opts_p temp;

	if (!g_disp_opts_head)
	{
		g_disp_opts_head   = disp_opts;
		disp_opts->next = NULL;
	}
	else
	{
		temp = g_disp_opts_head;
		while (temp->next)
		{
			temp = temp->next;
		}
		temp->next	  = disp_opts;
		disp_opts->next = NULL;
	}

	return 0;
}


/**********************************************************************
 * name£º show_disp_opts
 * description£º show all display
 * input parameters£º void
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
void show_disp_opts(void)
{
	int i = 0;
	disp_opts_p temp = g_disp_opts_head;

	while (temp)
	{
		printf("%02d %s\n", i++, temp->name);
		temp = temp->next;
	}
}


/**********************************************************************
 * name£º show_disp_opts
 * description£º get a display by name
 * input parameters£º name
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
disp_opts_p get_disp_opts(char *name)
{
	disp_opts_p temp = g_disp_opts_head;
	
	while (temp)
	{
		if (strcmp(temp->name, name) == 0)
		{
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

/**********************************************************************
 * name£º select_init_disp_dev
 * description£º select a display by name, and init it 
 * input parameters£º name
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
void select_init_disp_dev(char *name)
{
	g_default_disp_opts = get_disp_opts(name);
	if (g_default_disp_opts)
	{
		g_default_disp_opts->device_init();
		g_default_disp_opts->clear_screen(0);
	}
}


/**********************************************************************
 * name£º get_default_disp_dev
 * description£º get default display 
 * input parameters£º void
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
disp_opts_p get_default_disp_dev(void)
{
	return g_default_disp_opts;
}

/**********************************************************************
 * name£º get_disp_resolution
 * description£º get selected display resolution
 * input parameters£º void
 * output parameters£º x_res, y_res, bpp
 * return value£º return 0 if success, else return 0
 *
 ***********************************************************************/
int get_disp_resolution(int *x_res, int *y_res, int *bpp)
{
	if (g_default_disp_opts)
	{
		*x_res = g_default_disp_opts->x_res;
		*y_res = g_default_disp_opts->y_res;
		*bpp  = g_default_disp_opts->bpp;
		return 0;
	}
	else
	{
		return -1;
	}
}

/**********************************************************************
 * name: get_videobuf_for_display
 * description: get a frame buff depend default display
 * input parameters: void
 * output parameters: frme_buf
 * return value:  0
 *
 ***********************************************************************/
int get_videobuf_for_display(video_buf_p frme_buf)
{
	frme_buf->pixel_format = (g_default_disp_opts->bpp == 16) ? V4L2_PIX_FMT_RGB565 :\
								(g_default_disp_opts->bpp == 32) ? V4L2_PIX_FMT_RGB32 :\
								 0;
	frme_buf->pixel_datas.width = g_default_disp_opts->x_res;
	frme_buf->pixel_datas.height = g_default_disp_opts->y_res;
	frme_buf->pixel_datas.bpp	= g_default_disp_opts->bpp;
	frme_buf->pixel_datas.line_bytes= g_default_disp_opts->line_width;
	frme_buf->pixel_datas.total_bytes= frme_buf->pixel_datas.line_bytes* frme_buf->pixel_datas.height;
	frme_buf->pixel_datas.aucpixel_datas = g_default_disp_opts->disp_mem;
	return 0;
}

/**********************************************************************
 * name: flush_video_pixel_data_to_disp
 * description: flush data to display device
 * input parameters: pixel_data
 * output parameters: void
 * return value: 0
 *
 ***********************************************************************/
void flush_video_pixel_data_to_disp(pixel_datas_p pixel_data)
{
	g_default_disp_opts->show_page(pixel_data);
}

/**********************************************************************
* name: get_dev_video_mem
* description: 
* input parameters: void
* output parameters: void
* return value: video mem pointer
*
***********************************************************************/
video_mem_p get_dev_video_mem(void)
{
	video_mem_p temp = g_video_mem_head;
	
	while (temp)
	{
		if (temp->dev_frame_buffer)
		{
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}


/**********************************************************************
* name: display_module_init
* description: 
* input parameters: void
* output parameters: void
* return value: void
*
***********************************************************************/
int display_module_init(void)
{
	int error;
	
	error = fb_init();

	return error;
}

