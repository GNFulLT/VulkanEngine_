#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <cstdint>
#include <cmath>
#include <mimalloc.h>

// Argument to str
// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html#Stringizing

#ifndef _STR
#define _STR(str) #str
#define _STR_XDEF(str) _STR(str)
#define _USTR(str) U#str
#endif // _STR
// Force compile to make inline the function, variable etc.

#define _CRT_SECURE_NO_WARNINGS

#ifndef _F_INLINE_
#ifndef _DEBUG
#if defined(__GNUC__)
#define _F_INLINE_ __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define _F_INLINE_ __forceinline
#else
#define _F_INLINE_ inline
#endif // __GNUC__
#else
#define _F_INLINE_ inline
#endif // _DEBUG
#endif // _F_INLINE_

// Use this instead of inline
#ifndef _INLINE_
#ifdef _DEBUG
#define _INLINE_ inline
#else
#define _INLINE_ inline
#endif // _DEBUG
#endif // _INLINe_

// Inform the compiler that the return value of the function is important and shouldn't be ignored
#ifndef _IMP_RETURN_
#define _IMP_RETURN_ [[nodiscard]]
#endif

#ifndef _UNUSED_
#define _UNUSED_ [[maybe_unused]] 
#endif

// Undef Win32 awkward defines
#ifdef _WIN32
#undef min // override standard definition. It causes bugs some libraries like taskflow
#undef max // override standard definition. It causes bugs some libraries like taskflow
#undef ERROR 
#undef DELETE 
#undef MessageBox
#undef Error
#undef ERROR
#undef OK
#undef CONNECT_DEFERRED
#endif // _WIN32

// Swap two value. Don't forget it calls default constructor 
#ifndef SWAP
#define SWAP(m_x, m_y) __swap_tmpl((m_x), (m_y))

template <class T>
_INLINE_ void __swap_tmpl(T& x, T& y) {
	T temp = x;
	x = y;
	y = temp;
}

// Pointer definition of swap
template <class T>
_INLINE_ void __swap_tmpl(T* x, T* y) {
	T* temp = x;
	x = y;
	y = temp;
}
#endif // SWAP

#ifndef _GARR_SIZE
#if defined(_MSC_VER) && !defined(USE_GARR_SIZE)
#define _GARR_SIZE(pArr) ARRAYSIZE(pArr)
#elif defined(USE_GARR_SIZE) && defined(USE_GARR_SIZE_TEMPLATED)
template<typename T, size_t N>
size_t arrlen(T(&)[N]){return N;}
#define _GARR_SIZE(pArr) (arrlen(pArr)) 
#else
#define _GARR_SIZE(pArr)  ((sizeof(pArr) / sizeof(*(pArr))) / static_cast<size_t>(!(sizeof(pArr) % sizeof(*(pArr)))))
#endif // _MSC_VER && USE_GARR_SIZE
#endif // _GARR_SIZE
#endif // TYPEDEFS_H

#ifdef CUSTOM_NEW
#ifndef MIM_NEW
#define MIM_NEW

// C++ OVERRIDES

void* operator new(size_t size) {
	// Your implementation here
	return mi_malloc(size);
}

void* operator new[](size_t size) {
	// Your implementation here
	return mi_malloc(size);
}

void operator delete(void* ptr) noexcept {
	// Your implementation here
	mi_free(ptr);
}

void operator delete[](void* ptr) noexcept {
	// Your implementation here
	mi_free(ptr);
}


// Redirecting to the mimalloc. This is for 3rd libraries
#define malloc(size) mi_malloc(size)
#define calloc(size,n) mi_calloc(size,n)
#define realloc(p,newSize) mi_recalloc(p,newSize)
#define free(p) mi_free(p)
#define aligned_alloc(alignment,size) mi_aligned_alloc(alignment,size)
#define strdup(s) mi_strdup(s)
#define strndup(s,n) mi_strndup(sn)
#define realpath(fname,resolved_name) mi_realpath(fname,resolved_name)

// For POSIX
#if __has_include(<unistd.h>)
#define posix_memalign(p,alignment,size) mi_posix_memalign(p,alignment,size)
#endif // POSIX
#if __APPLE__
#define vfree(p) mi_free(p)
#define malloc_size(p) mi_malloc_size(p)
#define malloc_good_size(size) mi_malloc_good_size(size)
#elif __linux__ 
#define memalign(alignment,size) mi_memalign(alignment,size)
#define valloc(size) mi_valloc(size)
#define pvalloc(size) mi_pvalloc(size)
#define malloc_usable_size(p) mi_malloc_usable_size(p)
#define reallocf(p,newsize) mi_reallocf(p,newsize)
#elif _WIN32
#define expand(p,newsize) mi_expand(p,newsize)
#define _msize(p) mi_usable_size(p)

//X TODO : OVERRIDE DBG FUNCTIONALITY 

#define _malloc_dbg(size,block,fname,line) ERROR
#define _realloc_dbg(p,newsize,block_type,fname,line) ERROR
#define _calloc_dbg(count,size,block_type,fname,line) ERROR
#define _expand_dbg(p,size,block_type,fname,line) ERROR
#define _msize_dbg(p,block_type) ERROR
#define _free_dbg(p,block_type) ERROR

#endif // __APPLE__
 

#endif //MIM_NEW
#endif // CUSTOM_NEW

#define SINGLETON(class_name)	public: \
class_name(const class_name&) = delete; \
class_name& operator=(const class_name) = delete; \
_F_INLINE_ static void set_singleton(class_name* ptr)	{ singleton = ptr;}	\
_F_INLINE_ static class_name* get_singleton() \
{													\
	return singleton;								\
}													\
private:										\
	_INLINE_ static class_name* singleton;		



#ifdef USE_DEBUG_ALLOCATION
#ifndef DEBUG_NEW
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif //_WIN32
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
#ifdef USE_DEBUG_ALLOCATION_IMPLICIT
#define new DEBUG_NEW
#endif // USE_DEBUG_ALLOCATION_IMPLICIT
#endif // DEBUG_NEW
#endif // USE_DEBUG_ALLOCATION