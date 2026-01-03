#include <stdlib.h>

/* Minimal newlib compatibility shims for host-side testing. */
#if __has_include(<sys/reent.h>)
#include <sys/reent.h>
#else
struct _reent
{
    int _errno;
};
#endif

struct _reent _impure_data = {0};
struct _reent* _impure_ptr = &_impure_data;

void* _malloc_r(struct _reent* r, size_t size)
{
    (void)r;
    return malloc(size);
}

void _free_r(struct _reent* r, void* ptr)
{
    (void)r;
    free(ptr);
}

void* _realloc_r(struct _reent* r, void* ptr, size_t size)
{
    (void)r;
    return realloc(ptr, size);
}
