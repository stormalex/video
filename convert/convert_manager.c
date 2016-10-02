/*****************************************************
 *convert_manager.c
 *
 *
 */
#include <string.h>

#include <config.h>
#include <convert_manager.h>

/**************************************************************
 *module global variable
 **************************************************************/
static video_convert_p g_video_opts_head = NULL;

/**************************************************************
 *extern function declaration
 **************************************************************/
extern int yuv2rgb_init(void);


/**********************************************************************
 * name: register_videoconvert_opts
 * description: register a video_convert struct,this struct provides
 * 				a set of functions to convert video format
 * input parameters: A video_convert struct
 * output parameters:void
 * return value:returen 0 if seccess
 * 
 ***********************************************************************/
int register_videoconvert_opts(video_convert_p video_opts)
{
	video_convert_p tmp;

	if (!g_video_opts_head)
	{
		g_video_opts_head   = video_opts;
		video_opts->next = NULL;
	}
	else
	{
		tmp = g_video_opts_head;
		while (tmp->next)
		{
			tmp = tmp->next;
		}
		tmp->next = video_opts;
		video_opts->next = NULL;
	}

	return 0;
}


/**********************************************************************
 * name�� show_videoconvert_opts
 * description�� show all the video convertor name
 * input parameters�� void
 * output parameters�� void
 * return value�� NULL
 *
 ***********************************************************************/
void show_videoconvert_opts(void)
{
	int i = 0;
	video_convert_p tmp = g_video_opts_head;

	while (tmp)
	{
		printf("%02d %s\n", i++, tmp->name);
		tmp = tmp->next;
	}
}

/**********************************************************************
 * name�� get_videoconvert_opts
 * description�� get a video convertor by name
 * input parameters�� convertor name
 * output parameters�� void
 * return value�� if find convertor, return this video_convert, else return NULL
 *
 ***********************************************************************/
video_convert_p get_videoconvert_opts(char *name)
{
	video_convert_p tmp = g_video_opts_head;
	
	while (tmp)
	{
		if (strcmp(tmp->name, name) == 0)
		{
			return tmp;
		}
		tmp = tmp->next;
	}
	return NULL;
}

/**********************************************************************
 * name�� get_video_convert_by_formats
 * description�� get a video convertor by input and output video format
 * input parameters�� input format and output format
 * output parameters�� void
 * return value�� if find convertor, return this video_convert, else return NULL
 *
 ***********************************************************************/
video_convert_p get_video_convert_by_formats(int pixel_format_in, int pixel_format_out)
{
	video_convert_p tmp = g_video_opts_head;
	
	while (tmp)
	{
		if (tmp->is_support(pixel_format_in, pixel_format_out))
		{
			return tmp;
		}
		tmp = tmp->next;
	}
	return NULL;
}


/**********************************************************************
 * name�� video_convert_init
 * description�� init video convert module, init all convertor
 * input parameters�� void
 * output parameters�� void
 * return value�� 
 *
 ***********************************************************************/
int video_convert_init(void)
{
	int error;

	error  = yuv2rgb_init();

	return error;
}

