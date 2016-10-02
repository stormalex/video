#include <config.h>
#include <pic_operation.h>
#include <stdlib.h>
#include <string.h>



/**********************************************************************
 * name： yuv422torgb32
 * description： convert data format form V4L2_PIX_FMT_YUYV(YUV2) to V4L2_PIX_FMT_RGB32
 * input parameters： input data addr, output data addr, video width and height
 * output parameters： NULL
 * return value： return 0.
 *
 ***********************************************************************/
int pic_zoom(pixel_datas_p origin_pic, pixel_datas_p zoom_pic)
{
    unsigned long dst_width = zoom_pic->width;
    unsigned long* src_x_table;
	unsigned long x;
	unsigned long y;            
	unsigned long src_y;
	unsigned char *dest;
	unsigned char *src;
	unsigned long pixel_bytes = origin_pic->bpp/8;

	if (origin_pic->bpp != zoom_pic->bpp)
	{
		return -1;
	}

    src_x_table = malloc(sizeof(unsigned long) * dst_width);
    if (NULL == src_x_table)
    {
        printf("malloc error!\n");
        return -1;
    }

    for (x = 0; x < dst_width; x++)//生成表 src_x_table
    {
        src_x_table[x]=(x*origin_pic->width/zoom_pic->width);
    }

    for (y = 0; y < zoom_pic->height; y++)
    {			
        src_y = (y * origin_pic->height / zoom_pic->height);

		dest = zoom_pic->aucpixel_datas + y*zoom_pic->line_bytes;
		src  = origin_pic->aucpixel_datas + src_y*origin_pic->line_bytes;
		
        for (x = 0; x <dst_width; x++)
        {
            /* 原图座标: src_x_table[x]，srcy
             * 缩放座标: x, y
			 */
			 memcpy(dest+x*pixel_bytes, src+src_x_table[x]*pixel_bytes, pixel_bytes);
        }
    }

    free(src_x_table);
	return 0;
}

