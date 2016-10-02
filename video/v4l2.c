/*****************************************************
 *v4l2.c
 *构造一个struct video_opts,向video_manager注册这个结构体
 *
 */
#include <config.h>
#include <video_manager.h>
#include <disp_manager.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int v4l2_init_device(char *dev_name, video_device_p video_dev);
static int v4l2_exit_device(video_device_p video_dev);
static int v4l2_get_frame_for_streaming(video_device_p video_dev, video_buf_p video_buf_p);
static int v4l2_put_frame_for_streaming(video_device_p video_dev, video_buf_p video_buf_p);
static int v4l2_get_frame_for_readwrite(video_device_p video_dev, video_buf_p video_buf_p);
static int v4l2_put_frame_for_readwrite(video_device_p video_dev, video_buf_p video_buf_p);
static int v4l2_start_device(video_device_p video_dev);
static int v4l2_stop_device(video_device_p video_dev);
static int v4l2_get_format(video_device_p video_dev);

static int g_support_format[] = {
	V4L2_PIX_FMT_YUYV,
	V4L2_PIX_FMT_MJPEG,
	V4L2_PIX_FMT_RGB565,
	V4L2_PIX_FMT_RGB24,
	V4L2_PIX_FMT_RGB32,
};

video_opts_t g_tV4l2VideoOpr = {
	.name				= "v4l2",
	.init_device		= v4l2_init_device,
	.exit_device		= v4l2_exit_device,
	.get_frame			= v4l2_get_frame_for_streaming,
	.put_frame			= v4l2_put_frame_for_streaming,
	.start_device		= v4l2_start_device,
	.stop_device		= v4l2_stop_device,
	.get_format			= v4l2_get_format,
};


static int is_support_format(int format)
{
	int i;
	for(i = 0; i < (sizeof(g_support_format)/sizeof(g_support_format[0])); i++)
	{
		if(g_support_format[i] == format)
			return 1;
	}
	
	return 0;
}

static int v4l2_init_device(char *dev_name, video_device_p video_dev)
{
	int fd;
	int error;
	struct v4l2_capability v4l2_cap;
	struct v4l2_fmtdesc fmt_desc;
	struct v4l2_format tv4l2_format;
	struct v4l2_requestbuffers v4l2_req_buffers;
	struct v4l2_buffer v4l2_buffers;
	int lcd_width,lcd_height,lcd_bpp;
	int i;
	
	fd = open(dev_name, O_RDWR);
	if(fd < 0)
	{
		printf("Can not open %s\n", dev_name);
		exit(0);
	}
	
	video_dev->fd = fd;

	//查询设备的能力
	memset(&v4l2_cap, 0, sizeof(struct v4l2_capability));
	error = ioctl(fd, VIDIOC_QUERYCAP, &v4l2_cap);
	if(error < 0)
	{
		printf("Error device %s : unable to query device.\n", dev_name);
		goto err_exit;
	}

	if(!(v4l2_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		printf("Error device %s : is not a video capture device.\n", dev_name);
		goto err_exit;
	}

	if(v4l2_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
	{
		printf("Device %s : support streaming i/o.\n", dev_name);
	}
	if(v4l2_cap.capabilities & V4L2_CAP_READWRITE)
	{
		g_tV4l2VideoOpr.get_frame = v4l2_get_frame_for_readwrite;
		g_tV4l2VideoOpr.put_frame = v4l2_put_frame_for_readwrite;
		printf("Device %s : support readwrite i/o.\n", dev_name);
	}

	//检查是否支持format
	video_dev->pixel_format = 0;
	memset(&fmt_desc, 0, sizeof(struct v4l2_fmtdesc));
	fmt_desc.index = 0;
	fmt_desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while((error = ioctl(fd, VIDIOC_ENUM_FMT, &fmt_desc)) == 0)
	{
		fmt_desc.index++;
		if(is_support_format(fmt_desc.pixelformat))
		{
			video_dev->pixel_format = fmt_desc.pixelformat;
			break;
		}
	}

	if(!video_dev->pixel_format)
	{
		printf("Error device %s : can not support the format of this divice.\n", dev_name);
		goto err_exit;
	}

	get_disp_resolution(&lcd_width, &lcd_height, &lcd_bpp);

	memset(&tv4l2_format, 0, sizeof(struct v4l2_format));
	tv4l2_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tv4l2_format.fmt.pix.width 			= lcd_width;
	tv4l2_format.fmt.pix.height 		= lcd_height;
	tv4l2_format.fmt.pix.pixelformat 	= video_dev->pixel_format;
	tv4l2_format.fmt.pix.field = V4L2_FIELD_ANY;
	error = ioctl(fd, VIDIOC_S_FMT, &tv4l2_format);		//如果驱动程序发现某些参数无法支持，它会调整这些参数，再返回给应用程序
	if(error < 0)
	{
		printf("Error device %s : unable to set format.\n", dev_name);
		goto err_exit;
	}

	video_dev->width = tv4l2_format.fmt.pix.width;
	video_dev->height = tv4l2_format.fmt.pix.height;

	memset(&v4l2_req_buffers, 0, sizeof(struct v4l2_requestbuffers));
	v4l2_req_buffers.count = NB_BUFFER;
	v4l2_req_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_req_buffers.memory = V4L2_MEMORY_MMAP;

	error = ioctl(fd, VIDIOC_REQBUFS, &v4l2_req_buffers);
	if(error < 0)
	{
		printf("Error device %s : unable request buffers.\n", dev_name);
		goto err_exit;
	}

	
	if(v4l2_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
	{
		video_dev->video_buf_count = v4l2_req_buffers.count;
		
		for(i = 0; i < video_dev->video_buf_count; i++)			//查询缓冲区，并依次mmap映射到用户空间
		{
			memset(&v4l2_buffers, 0, sizeof(struct v4l2_buffer));
			v4l2_buffers.index = i;
			v4l2_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			v4l2_buffers.memory = V4L2_MEMORY_MMAP;
			error = ioctl(fd, VIDIOC_QUERYBUF, &v4l2_buffers);
			if(error < 0)
			{
				printf("Error device %s : unable to query buf.\n", dev_name);
				goto err_exit;
			}
			video_dev->mem[i] = mmap(0,
								 v4l2_buffers.length, PROT_READ, MAP_SHARED, fd,
								 v4l2_buffers.m.offset);
			if(video_dev->mem[i] == MAP_FAILED)
			{
				printf("Error device %s : can not mmap buf[%d].\n", dev_name, i);
				goto err_exit;
			}
			video_dev->video_buf_max_length = v4l2_buffers.length;
		}
		for(i = 0; i < video_dev->video_buf_count; i++)
		{
			memset(&v4l2_buffers, 0, sizeof(struct v4l2_buffer));
			v4l2_buffers.index = i;
			v4l2_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			v4l2_buffers.memory = V4L2_MEMORY_MMAP;
			error = ioctl(fd, VIDIOC_QBUF, &v4l2_buffers);
			if(error < 0)
			{
				printf("Error device %s : unable to queue buf.\n", dev_name);
				goto err_exit;
			}
		}
	}
	else if(v4l2_cap.capabilities & V4L2_CAP_READWRITE)
	{
		video_dev->video_buf_count = 1;
		video_dev->mem[0] = malloc(video_dev->width * video_dev->height * 4);	//在这个系统中，一个像素最多占4个字节
		video_dev->video_buf_max_length = (video_dev->width * video_dev->height * 4);
	}

	video_dev->video_ops = &g_tV4l2VideoOpr;
	
	return 0;
	
err_exit:
	close(fd);
	return error;
}

static int v4l2_exit_device(video_device_p video_dev)
{
	int i;

	for(i = 0; i < video_dev->video_buf_count; i++)
	{
		if(video_dev->mem[i])
			munmap(video_dev->mem[i], video_dev->video_buf_max_length);
		video_dev->mem[i] = NULL;
	}

	close(video_dev->fd);
	return 0;
}

static int v4l2_get_frame_for_streaming(video_device_p video_dev, video_buf_p video_buf_p)
{
	struct pollfd fds[1];
	struct v4l2_buffer buf;
	int ret = 0;
	
	fds[0].fd = video_dev->fd;
	fds[0].events = POLLIN;
	
	ret = poll(&fds[0], 1, -1);
	if(ret <= 0)
	{
		printf("Poll error.\n");
		return -1;
	}

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(fds[0].fd, VIDIOC_DQBUF, &buf);
	if(ret < 0)
	{
		printf("Unable to dequeue buf.\n");
		return -1;
	}
	
	video_dev->video_buf_curidx = buf.index;
	video_buf_p->pixel_format = video_dev->pixel_format;
	video_buf_p->pixel_datas.width = video_dev->width;
	video_buf_p->pixel_datas.height = video_dev->height;
	video_buf_p->pixel_datas.bpp = (video_dev->pixel_format == V4L2_PIX_FMT_YUYV) ? 16 :\
									(video_dev->pixel_format == V4L2_PIX_FMT_MJPEG) ? 0 :\
									(video_dev->pixel_format == V4L2_PIX_FMT_RGB565) ? 16 :\
									(video_dev->pixel_format == V4L2_PIX_FMT_RGB24) ? 24 :\
									(video_dev->pixel_format == V4L2_PIX_FMT_RGB32) ? 32 : 0;
	video_buf_p->pixel_datas.line_bytes = video_buf_p->pixel_datas.width * video_buf_p->pixel_datas.bpp / 8;
	video_buf_p->pixel_datas.total_bytes = buf.bytesused;
	video_buf_p->pixel_datas.aucpixel_datas = video_dev->mem[buf.index];
	
	return 0;
}
	
static int v4l2_put_frame_for_streaming(video_device_p video_dev, video_buf_p video_buf_p)
{
	struct v4l2_buffer buf;
	int ret = 0;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.index = video_dev->video_buf_curidx;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(video_dev->fd, VIDIOC_QBUF, &buf);
	if(ret < 0)
	{
		printf("Unable to queue buf.\n");
		return -1;
	}

	return 0;
}

static int v4l2_get_frame_for_readwrite(video_device_p video_dev, video_buf_p video_buf_p)
{
	int ret = 0;

	ret = read(video_dev->fd, video_dev->mem[0], video_dev->video_buf_max_length);
	if(ret <= 0)
	{
		printf("Unable to queue buf.\n");
		return -1;
	}

	video_buf_p->pixel_format = video_dev->pixel_format;
	video_buf_p->pixel_datas.width = video_dev->width;
	video_buf_p->pixel_datas.height = video_dev->height;
	video_buf_p->pixel_datas.bpp = (video_dev->pixel_format == V4L2_PIX_FMT_YUYV) ? 16 :\
									(video_dev->pixel_format == V4L2_PIX_FMT_MJPEG) ? 0 :\
									(video_dev->pixel_format == V4L2_PIX_FMT_RGB565) ? 16 :\
									(video_dev->pixel_format == V4L2_PIX_FMT_RGB24) ? 24 :\
									(video_dev->pixel_format == V4L2_PIX_FMT_RGB32) ? 32 : 0;
	video_buf_p->pixel_datas.line_bytes = video_buf_p->pixel_datas.width * video_buf_p->pixel_datas.bpp / 8;
	video_buf_p->pixel_datas.total_bytes = ret;
	video_buf_p->pixel_datas.aucpixel_datas = video_dev->mem[0];

	return 0;
}

static int v4l2_put_frame_for_readwrite(video_device_p video_dev, video_buf_p video_buf_p)
{
	return 0;
}

static int v4l2_start_device(video_device_p video_dev)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	ret = ioctl(video_dev->fd, VIDIOC_STREAMON, &type);
	if(ret < 0)
	{
		printf("Unable to start capture.\n");
		return -1;	
	}

	return 0;
}

static int v4l2_stop_device(video_device_p video_dev)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	ret = ioctl(video_dev->fd, VIDIOC_STREAMOFF, &type);
	if(ret < 0)
	{
		printf("Unable to start capture.\n");
		return -1;	
	}

	return 0;
}

static int v4l2_get_format(video_device_p video_dev)
{
	return (video_dev->pixel_format);
}

int v4l2_init(void)
{
	return register_video_opts(&g_tV4l2VideoOpr);
}
