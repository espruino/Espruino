#include <stdio.h>
#include <stdlib.h>
#include "wm_config.h"
#include "wm_regs.h"
#include <reent.h>
#include <string.h>
#include <stdarg.h>


int wm_printf(const char *fmt,...) 
{
	return 0;
}

_ssize_t _write_r (struct _reent *r, int file, const void *ptr, size_t len)
{
	return len;
}

_ssize_t _read_r(struct _reent *r, int file, void *ptr, size_t len)
{
	return 0;
}

int _close_r(struct _reent *r, int file)
{
	return 0;
}

_off_t _lseek_r(struct _reent *r, int file, _off_t ptr, int dir)
{
	return (_off_t)0;	/*  Always indicate we are at file beginning.  */
}

int _fstat_r(struct _reent *r, int file, struct stat *st)
{
	return 0;
}

int _isatty(int file)
{
	return 1;
}

void abort(void)
{
  while(1);
}

extern char end[];
extern char __HeapLimit[];
static char *heap_ptr = end;
static char *heap_end = __HeapLimit;
/**
 * @brief          This function is used to extend current heap size, Limit is __HeapLimit
 *
 * @param[in]    None  
 *
 * @return         None
 *
 * @note           Normally, it is just used in gcc mode.
 */
void * _sbrk_r(struct _reent *_s_r, ptrdiff_t nbytes)
{
	char *base;

	base = heap_ptr;

	if(base + nbytes > heap_end)
    {
		return (void *)-1;
    }

	heap_ptr += nbytes;	
	return base;
}

void * tls_reserve_mem_lock(int nbytes)
{
	if(heap_end - (nbytes + 4) <= heap_ptr)
	{
		return NULL;
	}
	heap_end  -= (nbytes + 4);
	return (void *)((((int)heap_end + 3) >> 2) << 2);	
}

void tls_reserve_mem_unlock(void)
{	
	heap_end = __HeapLimit;
}

