#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <config.h>
#include <disp_manager.h>
#include <pic_operation.h>
#include <render.h>
#include <convert_manager.h>
#include <video_manager.h>
#include <mbx.h>


typedef struct main_info{
	int capture_pixel_format;
	int display_pixel_format;
	video_device_p video_dev;
	video_convert_p video_convert;
	video_buf_p video_buf;
	video_buf_p frame_buf;
	video_buf_p convert_buf;
	MBX_Handle capture2convert;
	MBX_Handle convert2display;
	int convert_bpp;
	int width;
	int height;
}main_info_t, *main_info_p;

static void *capture_thread(void *arg);
static void *convert_thread(void *arg);
static void *display_thread(void *arg);


int main(int argc, char *argv[])
{
	int error;
	video_device_t video_dev;
	video_buf_t video_buf;
	video_buf_t convert_buf;
	video_buf_t zoom_buf;
	video_buf_t frame_buf;
	video_convert_p video_convert;
	int video_pixel_format;
	int display_pixel_format;
	int width, height, bpp;
	pthread_t capture_id;
	pthread_t convert_id;
	pthread_t display_id;
	main_info_t cfg_para;
	
	if(argc != 2)
	{
		printf("Usage : video2lcd </dev/video0,1,2,3...>\n");
		return -1;
	}

	error = display_module_init();
	if(error)
	{
		printf("display_module_init failed!\n");
		exit(0);
	}
	select_init_disp_dev("fb");
	get_disp_resolution(&width, &height, &bpp);
	
	get_videobuf_for_display(&frame_buf);
	display_pixel_format = frame_buf.pixel_format;
	cfg_para.display_pixel_format = display_pixel_format;

	error = video_module_init();
	if(error)
	{
		printf("display_module_init failed!\n");
		exit(0);
	}
	

	error = video_device_init(argv[1], &video_dev);
	if(error)
	{
		printf("video_video_module_init failed!\n");
		exit(0);
	}

	video_pixel_format = video_dev.video_ops->get_format(&video_dev);
	cfg_para.capture_pixel_format = video_pixel_format;

	error = video_convert_init();
	if(error)
	{
		printf("display_module_init failed!\n");
		exit(0);
	}

	video_convert = get_video_convert_by_formats(video_pixel_format, display_pixel_format);
	if(!video_convert)
	{
		printf("Get_video_convert_for_formats failed!\n");
		exit(0);
	}

	error = video_dev.video_ops->start_device(&video_dev);
	if(error)
	{
		printf("video device start failed!\n");
		exit(0);
	}
	
	memset(&zoom_buf, 0, sizeof(zoom_buf));
	memset(&video_buf, 0, sizeof(video_buf));
	memset(&convert_buf, 0, sizeof(convert_buf));

	convert_buf.pixel_datas.bpp = bpp;
	convert_buf.pixel_format = display_pixel_format;

	cfg_para.video_dev = &video_dev;
	cfg_para.video_buf = &video_buf;
	cfg_para.video_convert = video_convert;
	cfg_para.convert_buf = &convert_buf;
	cfg_para.frame_buf = &frame_buf;

	cfg_para.width = width;
	cfg_para.height = height;
	cfg_para.convert_bpp = bpp;

	cfg_para.capture2convert = MBX_create(2);
	cfg_para.convert2display = MBX_create(2);

	if(pthread_create(&capture_id, NULL, capture_thread, (void *)&cfg_para))
	{
		printf("Capture thread create failed!\n");
		exit(0);
	}
	
	if(pthread_create(&convert_id, NULL, convert_thread, (void *)&cfg_para))
	{
		printf("Convert thread create failed!\n");
		exit(0);
	}
	if(pthread_create(&display_id, NULL, display_thread, (void *)&cfg_para))
	{
		printf("Display thread create failed!\n");
		exit(0);
	}

	pthread_join(capture_id, NULL);
	pthread_join(convert_id, NULL);
	pthread_join(display_id, NULL);
	return 0;
}

static void *capture_thread(void *arg)
{
	int error;
	main_info_p cfg_para = (main_info_p)arg;
	video_device_p video_dev = cfg_para->video_dev;
	video_buf_p video_buf = cfg_para->video_buf;
	
	MBX_post(cfg_para->capture2convert, video_buf, sizeof(video_buf_t));

	while(1)
	{
		error = video_dev->video_ops->get_frame(video_dev, video_buf);
		if(error)
		{
			printf("Get frame failed\n");
			return NULL;
		}

		MBX_post(cfg_para->capture2convert, video_buf, sizeof(video_buf_t));

		error = video_dev->video_ops->put_frame(video_dev, video_buf);
		if(error)
		{
			printf("Put frame failed\n");
			return NULL;
		}
	}
	
	return NULL;
}
static void *convert_thread(void *arg)
{
	int error;
	main_info_p cfg_para = (main_info_p)arg;
	video_buf_p convert_buf = cfg_para->convert_buf;
	video_buf_t cap_buf;
	video_convert_p video_convert = cfg_para->video_convert;
	int count=0;
	
	convert_buf->pixel_datas.bpp = cfg_para->convert_bpp;
	convert_buf->pixel_format = cfg_para->display_pixel_format;
	MBX_pend(cfg_para->capture2convert,&cap_buf);
	while(1)
	{count++;
		MBX_pend(cfg_para->capture2convert,&cap_buf);
		if(cfg_para->capture_pixel_format != cfg_para->display_pixel_format)
		{
			error = video_convert->convert(&cap_buf, convert_buf);
			if(error)
			{
				printf("Video convert failed\n");
				return NULL;
			}
		}
		MBX_post(cfg_para->convert2display, convert_buf, sizeof(video_buf_t));
	}
	return NULL;
}
static void *display_thread(void *arg)
{
	main_info_p cfg_para = (main_info_p)arg;
	video_buf_t convert_buf;
	video_buf_p frame_buf = cfg_para->frame_buf;
	int top_left_x, top_left_y;

	//int count = 0;

	while(1)
	{
		MBX_pend(cfg_para->convert2display,&convert_buf);
		//merge to framebuffer
		top_left_x = (cfg_para->width - convert_buf.pixel_datas.width)/2;
		top_left_y = (cfg_para->height - convert_buf.pixel_datas.height)/2;
		
		//dbg_put_file(&convert_buf);
		pic_merge(top_left_x, top_left_y, &convert_buf.pixel_datas, &frame_buf->pixel_datas);

		//display
		flush_video_pixel_data_to_disp(&frame_buf->pixel_datas);
	}
	return NULL;
}


