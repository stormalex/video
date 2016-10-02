#ifndef _CONVERT_H_
#define _CONVERT_H_

#include <linux/videodev2.h>

#include <config.h>
#include <video_manager.h>


/**************************************************************
 *typedef struct
 **************************************************************/
typedef struct video_convert{
	char * name;
	int (*is_support)(int pixel_format_in, int pixel_format_out);
	int (*convert)(video_buf_p video_buf_in, video_buf_p video_buf_out);
	int (*convert_exit)(video_buf_p video_buf_out);
	struct video_convert* next;
}video_convert_t, *video_convert_p;


/**************************************************************
 *module API
 **************************************************************/
int video_convert_init(void);
int register_videoconvert_opts(video_convert_p video_opts);
video_convert_p get_video_convert_by_formats(int pixel_format_in, int pixel_format_out);
video_convert_p get_videoconvert_opts(char *name);



#endif /*_CONVERT_H_*/