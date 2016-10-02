#include <stdlib.h>

#include <convert_manager.h>
#include "color.h"


/**********************************************************************
 * name£º yuv2rgb_is_support
 * description£º whether to support input data format convert to out put data format
 * input parameters£º input data format and output data format
 * output parameters£º NULL
 * return value£º if support return 1, else return 0.
 *
 ***********************************************************************/
static int yuv2rgb_is_support(int pixel_format_in, int pixel_format_out)
{	
	if((pixel_format_in == V4L2_PIX_FMT_YUYV) && ((pixel_format_out == V4L2_PIX_FMT_RGB565) || (pixel_format_out == V4L2_PIX_FMT_RGB32)))
		return 1;
	return 0;
}

/**********************************************************************
 * name£º yuv422torgb565
 * description£º convert data format form V4L2_PIX_FMT_YUYV(YUV2) to V4L2_PIX_FMT_RGB565
 * input parameters£º input data addr, output data addr, video width and height
 * output parameters£º NULL
 * return value£º return 0.
 *
 ***********************************************************************/
static unsigned int yuv422torgb565(unsigned char *input_ptr, unsigned char *output_ptr, unsigned int image_width, unsigned int image_height)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = input_ptr;
	unsigned char *output_pt = output_ptr;
	size = image_width * image_height / 2;
	unsigned int r, g, b;
	unsigned int color;

	for(i = size; i > 0; i--)
	{
		Y = buff[0];
		U = buff[1];
		Y1= buff[2];
		V = buff[3];
		buff += 4;
		r = R_FROMYV(Y,V);
		g = G_FROMYUV(Y,U,V);
		b = B_FROMYU(Y,U);

		r = r >> 3;
		g = g >> 2;
		b = b >> 3;

		color = (r << 11) | (g << 5) | (b);
		*output_pt++ = color & 0xff;
		*output_pt++ = (color >> 8) & 0xff;
		
		r = R_FROMYV(Y1,V);
		g = G_FROMYUV(Y1,U,V);
		b = B_FROMYU(Y1,U);	
		r = r >> 3;
		g = g >> 2;
		b = b >> 3;

		color = (r << 11) | (g << 5) | (b);
		*output_pt++ = color & 0xff;
		*output_pt++ = (color >> 8) & 0xff;
	}

	return 0;
}

/**********************************************************************
 * name£º yuv422torgb32
 * description£º convert data format form V4L2_PIX_FMT_YUYV(YUV2) to V4L2_PIX_FMT_RGB32
 * input parameters£º input data addr, output data addr, video width and height
 * output parameters£º NULL
 * return value£º return 0.
 *
 ***********************************************************************/
static unsigned int yuv422torgb32(unsigned char *input_ptr, unsigned char *output_ptr, unsigned int image_width, unsigned int image_height)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *buff = input_ptr;
	unsigned int *output_pt = (unsigned int *)output_ptr;
	size = image_width * image_height / 2;
	unsigned int r, g, b;
	unsigned int color;

	for(i = size; i > 0; i--)
	{
		Y = buff[0];
		U = buff[1];
		Y1= buff[2];
		V = buff[3];
		buff += 4;
		r = R_FROMYV(Y,V);
		g = G_FROMYUV(Y,U,V);
		b = B_FROMYU(Y,U);

		color = (r << 16) | (g << 8) | (b);
		*output_pt++ = color;
		
		r = R_FROMYV(Y1,V);
		g = G_FROMYUV(Y1,U,V);
		b = B_FROMYU(Y1,U);	
		r = r >> 3;
		g = g >> 2;
		b = b >> 3;

		color = (r << 16) | (g << 8) | (b);
		*output_pt++ = color;
	}

	return 0;
}

/**********************************************************************
 * name£º yuv2rgb_convert
 * description£º convert video data format
 * input parameters£º input buf, output buf
 * output parameters£º NULL
 * return value£º return 0.
 *
 ***********************************************************************/
static int yuv2rgb_convert(video_buf_p convert_buf_in, video_buf_p convert_buf_out)
{
	pixel_datas_p pixel_datas_in_p  = &convert_buf_in->pixel_datas;
	pixel_datas_p pixel_datas_out_p = &convert_buf_out->pixel_datas;

	pixel_datas_out_p->width = pixel_datas_in_p->width;
	pixel_datas_out_p->height= pixel_datas_in_p->height;
	
	if(convert_buf_out->pixel_format == V4L2_PIX_FMT_RGB565)
	{
		pixel_datas_out_p->bpp = 16;
		pixel_datas_out_p->line_bytes = pixel_datas_out_p->width * pixel_datas_out_p->bpp / 8;
		pixel_datas_out_p->total_bytes = pixel_datas_out_p->line_bytes * pixel_datas_out_p->height;

		if(!pixel_datas_out_p->aucpixel_datas)
		{
			pixel_datas_out_p->aucpixel_datas = malloc(pixel_datas_out_p->total_bytes);
			if(pixel_datas_out_p->aucpixel_datas == NULL)
			{
				printf("Malloc failed\n");
				return -1;
			}
		}
		yuv422torgb565(pixel_datas_in_p->aucpixel_datas, pixel_datas_out_p->aucpixel_datas, pixel_datas_in_p->width, pixel_datas_in_p->height);
	}
	else if(convert_buf_out->pixel_format == V4L2_PIX_FMT_RGB32)
	{
		pixel_datas_out_p->bpp = 32;
		pixel_datas_out_p->line_bytes = pixel_datas_out_p->width * pixel_datas_out_p->bpp / 8;
		pixel_datas_out_p->total_bytes = pixel_datas_out_p->line_bytes * pixel_datas_out_p->height;
		
		if(!pixel_datas_out_p->aucpixel_datas)
		{
			pixel_datas_out_p->aucpixel_datas = malloc(pixel_datas_out_p->total_bytes);
			if(pixel_datas_out_p->aucpixel_datas == NULL)
			{
				printf("Malloc failed\n");
				return -1;
			}
		}
		
		yuv422torgb32(pixel_datas_in_p->aucpixel_datas, pixel_datas_out_p->aucpixel_datas, pixel_datas_in_p->width, pixel_datas_in_p->height);
	}
	
	return 0;
}

/**********************************************************************
 * name£º yuv2rgb_convert_exit
 * description£º exit data convert, free the convert buf
 * input parameters£º input buf, output buf
 * output parameters£º NULL
 * return value£º return 0.
 *
 ***********************************************************************/
static int yuv2rgb_convert_exit(video_buf_p convert_buf_out)
{
	if(convert_buf_out->pixel_datas.aucpixel_datas)
	{
		free(convert_buf_out->pixel_datas.aucpixel_datas);
		convert_buf_out->pixel_datas.aucpixel_datas = NULL;
	}
	return 0;
}

static video_convert_t g_yuv2rgb_convert={
	.name			= "yuv2rgb",
	.is_support		= yuv2rgb_is_support,
	.convert		= yuv2rgb_convert,
	.convert_exit	= yuv2rgb_convert_exit,
};

int yuv2rgb_init(void)
{
	initLut();
	return register_videoconvert_opts(&g_yuv2rgb_convert);
}

