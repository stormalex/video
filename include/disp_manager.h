#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

#include <pic_operation.h>
#include <video_manager.h>


typedef struct video_mem {
	int id;                 
	int dev_frame_buffer;
	pixel_datas_t tpixel_datas;
	struct video_mem *next;
}video_mem_t, *video_mem_p;

typedef struct disp_opts {
	char *name;
	int x_res;
	int y_res;
	int bpp; 
	int line_width;
	unsigned char *disp_mem;
	int (*device_init)(void);
	int (*show_pixel)(int start_x, int start_y, unsigned int color);
	int (*clear_screen)(unsigned int back_color);
	int (*show_page)(pixel_datas_p pixel_data);
	struct disp_opts *next;
}disp_opts_t, *disp_opts_p;

int register_disp_opts(disp_opts_p disp_opts);
void show_disp_opts(void);
disp_opts_p get_disp_opts(char *name);
void select_init_disp_dev(char *name);
disp_opts_p get_default_disp_dev(void);
int get_disp_resolution(int *x_res, int *y_res, int *bpp);
int get_videobuf_for_display(video_buf_p frme_buf);
void flush_video_pixel_data_to_disp(pixel_datas_p pixel_data);
video_mem_p get_dev_video_mem(void);
int display_module_init(void);


#endif /* _DISP_MANAGER_H */

