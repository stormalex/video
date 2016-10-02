
#ifndef _PIC_OPERATION_H
#define _PIC_OPERATION_H

typedef struct pixel_datas {
	int width;
	int height;
	int bpp;
	int line_bytes;
	int total_bytes;
	unsigned char *aucpixel_datas;
}pixel_datas_t, *pixel_datas_p;

#endif /* _PIC_OPERATION_H */

