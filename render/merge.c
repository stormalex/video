
#include <pic_operation.h>
#include <string.h>

/**********************************************************************
 * name£º pic_merge
 * description£º merge picture
 * input parameters£º x, y, font_pic, back_pic
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
int pic_merge(int x, int y, pixel_datas_p font_pic, pixel_datas_p back_pic)
{
	int i;
	unsigned char *src;
	unsigned char *dst;
	
	if ((font_pic->width > back_pic->width)  ||
		(font_pic->height > back_pic->height) ||
		(font_pic->bpp != back_pic->bpp))
	{
		return -1;
	}

	src = font_pic->aucpixel_datas;
	dst = back_pic->aucpixel_datas + y * back_pic->line_bytes + x * back_pic->bpp / 8;
	for (i = 0; i < font_pic->height; i++)
	{
		memcpy(dst, src, font_pic->line_bytes);
		src += font_pic->line_bytes;
		dst += back_pic->line_bytes;
	}
	return 0;
}


/**********************************************************************
 * name£º pic_merge_region
 * description£º merge picture region
 * input parameters£º x, y, font_pic, back_pic
 * output parameters£º void
 * return value£º 0
 *
 ***********************************************************************/
int pic_merge_region(int new_pic_start_x, int new_pic_start_y, int old_pic_start_x, int old_pic_start_y, int width, int height, pixel_datas_p new_pic, pixel_datas_p old_pic)
{
	int i;
	unsigned char *src;
	unsigned char *dst;
    int line_bytes_copy = width * new_pic->bpp / 8;

    if ((new_pic_start_x < 0 || new_pic_start_x >= new_pic->width) || \
        (new_pic_start_y < 0 || new_pic_start_y >= new_pic->height) || \
        (old_pic_start_x < 0 || old_pic_start_x >= old_pic->width) || \
        (old_pic_start_y < 0 || old_pic_start_y >= old_pic->height))
    {
        return -1;
    }
	
	src = new_pic->aucpixel_datas + new_pic_start_y * new_pic->line_bytes + new_pic_start_x * new_pic->bpp / 8;
	dst = old_pic->aucpixel_datas + old_pic_start_y * old_pic->line_bytes + old_pic_start_x * old_pic->bpp / 8;
	for (i = 0; i < height; i++)
	{
		memcpy(dst, src, line_bytes_copy);
		src += new_pic->line_bytes;
		dst += old_pic->line_bytes;
	}
	return 0;
}

