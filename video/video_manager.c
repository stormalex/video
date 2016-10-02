/*****************************************************
 *video_manager.c
 *
 *
 */

#include <config.h>
#include <disp_manager.h>
#include <string.h>

static video_opts_p g_video_opts_head = NULL;

/**********************************************************************
 * name�� register_video_opts
 * description�� register video operations to video module
 * input parameters�� video_opts
 * output parameters�� void
 * return value�� 0
 *
 ***********************************************************************/
int register_video_opts(video_opts_p video_opts)
{
	video_opts_p temp;

	if (!g_video_opts_head)
	{
		g_video_opts_head   = video_opts;
		video_opts->next = NULL;
	}
	else
	{
		temp = g_video_opts_head;
		while (temp->next)
		{
			temp = temp->next;
		}
		temp->next     = video_opts;
		video_opts->next = NULL;
	}

	return 0;
}


/**********************************************************************
 * name�� show_video_opts
 * description�� show all video operations
 * input parameters�� void
 * output parameters�� void
 * return value�� 0
 *
 ***********************************************************************/
void show_video_opts(void)
{
	int i = 0;
	video_opts_p temp = g_video_opts_head;

	while (temp)
	{
		printf("%02d %s\n", i++, temp->name);
		temp = temp->next;
	}
}

/**********************************************************************
 * name�� get_video_opts
 * description�� get video operations by name
 * input parameters�� name
 * output parameters�� void
 * return value�� 0
 *
 ***********************************************************************/
video_opts_p get_video_opts(char *name)
{
	video_opts_p temp = g_video_opts_head;
	
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
 * name�� video_device_init
 * description�� init video device
 * input parameters�� dev_name, video_dev
 * output parameters�� void
 * return value�� 0
 *
 ***********************************************************************/
int video_device_init(char *dev_name, video_device_p video_dev)
{
	int error = 0;
	video_opts_p temp = g_video_opts_head;
	
	while (temp)
	{
		error = temp->init_device(dev_name, video_dev);
		if(!error)
		{
			return 0;
		}
		temp = temp->next;
	}
	return -1;
}


/**********************************************************************
 * name�� video_module_init
 * description�� init video module
 * input parameters�� void
 * output parameters�� void
 * return value�� 0
 *
 ***********************************************************************/
int video_module_init(void)
{
	int error;

	error = v4l2_init();

	return error;
}

