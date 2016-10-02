
#ifndef _RENDER_H
#define _RENDER_H

#include <pic_operation.h>
#include <disp_manager.h>


int pic_zoom(pixel_datas_p ptOriginPic, pixel_datas_p ptZoomPic);

int pic_merge(int iX, int iY, pixel_datas_p ptSmallPic, pixel_datas_p ptBigPic);

int pic_merge_region(int iStartXofNewPic, int iStartYofNewPic, int iStartXofOldPic, int iStartYofOldPic, int width, int height, pixel_datas_p ptNewPic, pixel_datas_p ptOldPic);




#endif /* _RENDER_H */

