#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mbx.h>


MBX_Handle	MBX_create(int number)
{
	MBX_Handle	handle;
	handle = (MBX_Handle)malloc(sizeof(MBX_Obj));

	pthread_mutex_init(&(handle->lock),NULL);

	pthread_cond_init(&(handle->cond),NULL);

	handle->buf		= malloc(number * sizeof(SUB_MBX_Handle));
	handle->wpos	= 0;
	handle->rpos	= 0;
	handle->cnt		= 0;
	handle->number	= number;

	return handle;
}

void MBX_delete(MBX_Handle handle)
{
	pthread_mutex_destroy(&(handle->lock));
	pthread_cond_destroy(&(handle->cond));

	if(handle->buf != NULL)
	{
		free(handle->buf);
	}
	
	free(handle);
}

void MBX_post(MBX_Handle handle, void *value, int sz)
{
	pthread_mutex_lock(&(handle->lock));

	if(handle->cnt != handle->number)
	{
		handle->buf[handle->wpos] 		= (SUB_MBX_Handle)malloc(sizeof(SUB_MBX_Obj));
		handle->buf[handle->wpos]->buf 	= (unsigned char*)malloc(sz);
		memcpy(handle->buf[handle->wpos]->buf, value, sz);
		handle->buf[handle->wpos]->size = sz;
		handle->wpos = (handle->wpos + 1) % handle->number;

		if(handle->cnt == 0)
		{
			pthread_cond_signal(&(handle->cond));
		}
		handle->cnt ++;
	}

	pthread_mutex_unlock(&(handle->lock));
}

void MBX_pend(MBX_Handle handle,void *value)
{
	pthread_mutex_lock(&(handle->lock));
	if(handle->cnt == 0)
	{
		pthread_cond_wait(&(handle->cond),&(handle->lock));
	}

	handle->cnt --;
	memcpy(value, handle->buf[handle->rpos]->buf, handle->buf[handle->rpos]->size);
	free(handle->buf[handle->rpos]->buf);
	free(handle->buf[handle->rpos]);
	handle->rpos = (handle->rpos + 1) % handle->number;
	pthread_mutex_unlock(&(handle->lock));
}

