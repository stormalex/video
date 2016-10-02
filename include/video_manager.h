/*****************************************************
 *include file for video_manager.c
 *
 *
 */
#ifndef _VIDEO_MANAGER_H
#define _VIDEO_MANAGER_H

#include <linux/videodev2.h>

#include <config.h>
#include <pic_operation.h>

#define NB_BUFFER (4)


struct video_device;
struct video_opts;

typedef struct video_device{
	int fd;
	int pixel_format;
	int width;
	int height;
	int video_buf_count;
	int video_buf_max_length;
	int video_buf_curidx;
	unsigned char *mem[NB_BUFFER];
	struct video_opts* video_ops;
}video_device_t, *video_device_p;

typedef struct video_buf{
	pixel_datas_t pixel_datas;
	int pixel_format;
}video_buf_t, *video_buf_p;

typedef struct video_opts{
	char *name;
	struct video_opts* next;
	int (*init_device)(char *dev_name, struct video_device* video_dev);
	int (*exit_device)(video_device_p video_dev);
	int (*get_frame)(video_device_p video_dev, video_buf_p video_buf_p);
	int (*put_frame)(video_device_p video_dev, video_buf_p video_buf_p);
	int (*start_device)(video_device_p video_dev);
	int (*stop_device)(video_device_p video_dev);
	int (*get_format)(video_device_p video_dev);
}video_opts_t, *video_opts_p;

int video_module_init(void);

int v4l2_init(void);
int video_device_init(char *dev_name, video_device_p video_dev);
int register_video_opts(video_opts_p video_opts);


#endif	/*_VIDEO_MANAGER_H*/
