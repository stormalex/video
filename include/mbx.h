#ifndef MBX_H_
#define MBX_H_

typedef struct _tagSUB_MBX
{
	unsigned char	*buf;
	unsigned int	size;
}SUB_MBX_Obj, *SUB_MBX_Handle;


typedef struct _tagMBX
{
	pthread_mutex_t lock;

	pthread_cond_t	cond;
	SUB_MBX_Handle	*buf;
	int				wpos;
	int				rpos;
	int				cnt;
	int				number;
}MBX_Obj, *MBX_Handle;


MBX_Handle	MBX_create(int number);
void 		MBX_delete(MBX_Handle handle);
void 		MBX_post(MBX_Handle handle, void *value, int sz);
void 		MBX_pend(MBX_Handle handle, void *value);
#endif /* MBX_H_ */

