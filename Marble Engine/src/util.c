#include <util.h>


/*
 * Structure representing an std::vector-like
 * container.
 */
#pragma region UTIL-VECTOR
#define MB_UTIL_VEC_DEFSTARTCAP (32)
#if (SIZE_MAX == UINT64_MAX)
    #define MB_UTIL_VEC_GROWTHFAC (1.5)
#else
    #define MB_UTIL_VEC_GROWTHFAC (1.5f)
#endif


struct marble_util_vec {
    size_t m_size;   /* Current number of elements */
    size_t m_cap;    /* Maximum number of elements */
    size_t m_defcap; /* Initial maximum capacity */

    void **mpp_data; /* actual data storage */
    /*
     * Custom "destructor" which will be called
     * on every "object" inside **mpp_data**.
     */
    marble_dtor_t mfn_dtor;
};


void marble_util_vec_destroy(
    _Uninit_(pps_vector) struct marble_util_vec **pps_vector
) {
    if (pps_vector == NULL || *pps_vector == NULL)
        return;

    /* Call the destructors. */
    if ((*pps_vector)->mfn_dtor)
        for (size_t i = 0; i < (*pps_vector)->m_size; i++)
            (*(*pps_vector)->mfn_dtor)(&(*pps_vector)->mpp_data[i]);

    /* Free memory used by the vector itself. */
    free((*pps_vector)->mpp_data);
    free(*pps_vector);
    *pps_vector = NULL;
}

_Critical_ marble_ecode_t marble_util_vec_create(
    _In_opt_           size_t startcap,
    _In_opt_           marble_dtor_t fn_dtor,
    _Init_(pps_vector) struct marble_util_vec **pps_vector
) { MB_ERRNO
    if (pps_vector == NULL)
        return MARBLE_EC_PARAM;
    
    /*
     * Correct optional parameters in case
     * they are set to their standard
     * values. **fn_dest** can be NULL here,
     * meaning that no destructor will be called
     * on the objects.
     */
    startcap = startcap == 0 ? MB_UTIL_VEC_DEFSTARTCAP : startcap;
    
    /* Allocate memory for vector. */
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        sizeof **pps_vector,
        false,
        false,
        pps_vector
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;
    
    /*
     * Allocate memory for the dynamic
     * array, the memory that will hold the
     * actual data.
     */
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        startcap * sizeof *pps_vector,
        true,
        false,
        (void **)&(*pps_vector)->mpp_data
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;
    
    /* Initialize the info fields. */
    (*pps_vector)->mfn_dtor = fn_dtor;
    (*pps_vector)->m_cap    = startcap;
    (*pps_vector)->m_defcap = startcap;
    (*pps_vector)->m_size   = 0;
    
lbl_END:
    if (ecode != MARBLE_EC_OK)
        marble_util_vec_destroy(pps_vector);
    
    return ecode;
}

_Success_ok_ marble_ecode_t marble_util_vec_insert(
    _Inout_ struct marble_util_vec *ps_vector,
    _In_    void *p_obj,
            size_t index
) {
    if (ps_vector == NULL || p_obj == NULL || index > ps_vector->m_size)
        return MARBLE_EC_PARAM;

    /*
     * Reallocate the vector, increasing its
     * capacity if there is no space left.
     */
    if (ps_vector->m_size + 1 >= ps_vector->m_cap) {
        size_t newcap = (size_t)(MB_UTIL_VEC_GROWTHFAC * ps_vector->m_cap);

        /*
         * Attempt to allocate a new buffer of the given size.
         * If the attempt fails, the function will not modify
         * the vector.
         */
        void *p_new = realloc(
            ps_vector->mpp_data,
            sizeof *ps_vector->mpp_data * newcap
        );
        if (p_new == NULL)
            return MARBLE_EC_MEMREALLOC;

        ps_vector->m_cap    = newcap;
        ps_vector->mpp_data = (void **)p_new;
    }

    /*
     * In case we just want to add **p_obj**
     * to the end, do not even attempt
     * to move any memory.
     */
    if (index == ps_vector->m_size) {
        ps_vector->mpp_data[ps_vector->m_size] = p_obj;

        goto lbl_END;
    }

    /*
     * If **p_obj** is to be inserted in the middle
     * or at the beginning, we have to move the following
     * block by one position to make space for the pointer.
     */
    memmove(
        &ps_vector->mpp_data[index + 1],
        &ps_vector->mpp_data[index],
        (ps_vector->m_size - index) * sizeof *ps_vector->mpp_data
    );
    ps_vector->mpp_data[index] = p_obj;

lbl_END:
    ++ps_vector->m_size;

    return MARBLE_EC_OK;
}

_Success_ok_ marble_ecode_t marble_util_vec_pushback(
    _Inout_ struct marble_util_vec *ps_vector,
    _In_    void *p_obj
) {
    return
        marble_util_vec_insert(
            ps_vector,
            p_obj,
            ps_vector->m_size
        );
}

_Success_ok_ marble_ecode_t marble_util_vec_pushfront(
    _Inout_ struct marble_util_vec *ps_vector,
    _In_    void *p_obj
) {
    return 
        marble_util_vec_insert(
            ps_vector,
            p_obj,
            0
        );
}

void *marble_util_vec_erase(
    _Inout_ struct marble_util_vec *ps_vector,
            size_t index,
            bool rundest
) {
    if (ps_vector == NULL || index >= ps_vector->m_size)
        return NULL;

    /* Call object destructor if necessary. */
    if (rundest && ps_vector->mfn_dtor != NULL)
        (*ps_vector->mfn_dtor)(&ps_vector->mpp_data[index]);

    /* Save pointer value; if it has been destructed, it is NULL. */
    void *p_ret = ps_vector->mpp_data[index];

    /*
     * Move memory backwards to overwrite pointer.
     * 
     * If **index** points to the last element in the vector,
     * we do not call "memmove()" as it is useless anyway.
     */
    if (index != ps_vector->m_size - 1)
        memmove(
            &ps_vector->mpp_data[index],
            &ps_vector->mpp_data[index + 1],
            (ps_vector->m_size - index) * sizeof *ps_vector->mpp_data
        );

    /*
     * As "memmove()" does not actually "move" but copy the memory,
     * and we moved the rest of the vector to the left by one position,
     * the last element will now be duplicated.
     * So we have to NULL it.
     */
    ps_vector->mpp_data[--ps_vector->m_size] = NULL;

    /* Return pointer, may be NULL. */
    return p_ret;
}

void *marble_util_vec_popback(
    _Inout_ struct marble_util_vec *ps_vector,
            bool rundest
) {
    return
        marble_util_vec_erase(
            ps_vector,
            ps_vector->m_size - 1,
            rundest
        );
}

void *marble_util_vec_popfront(
    _Inout_ struct marble_util_vec *ps_vector,
            bool rundest
) {
    return
        marble_util_vec_erase(
            ps_vector,
            0,
            rundest
        );
}

size_t marble_util_vec_find(
    _In_ struct marble_util_vec *ps_vector,
    _In_ void *p_obj,
         size_t start,
         size_t end
) {
    if (ps_vector == NULL || p_obj == NULL || start > end || start >= ps_vector->m_size || end >= ps_vector->m_size)
        return SIZE_MAX;

    /*
     * If **start** and **end** are equal, and they are 0,
     * set **end** to the last index of the last element
     * in the vector.
     */
    end = start == end && start == 0 ? ps_vector->m_size : end;

    /*
     * Search for **p_obj** within the given searching range,
     * returning its index as soon as it is found.
     */
    for (size_t i = start; i < end; i++)
        if (ps_vector->mpp_data[i] == p_obj)
            return i;

    /*
     * Return SIZE_MAX essentially if **p_obj** cannot be
     * found.
     */
    return SIZE_MAX;
}

void *marble_util_vec_get(
    _In_ struct marble_util_vec *ps_vector,
         size_t index
) {
    if (ps_vector == NULL || index >= ps_vector->m_size)
        return NULL;

    return ps_vector->mpp_data[index];
}

size_t marble_util_vec_count(
    _In_ struct marble_util_vec *ps_vector
) {
    if (ps_vector == NULL)
        return SIZE_MAX;

    return ps_vector->m_size;
}
#pragma endregion (UTIL-VECTOR)


/*
 * Structure representing an HPC (uses platform-specific
 * means of measuring time with high resolution).
 */
#pragma region UTIL-CLOCK
/*
 * Variable holding frequency of HPC; this variable is
 * initialized when the engine starts-up, and is only
 * read throughout the entire lifetime of the
 * application.
 */
static int64_t gl_pfreq = -1;

/*
 * Initializes the internal HPC data.
 * An application should only call this function once when it
 * starts-up. Subsequent calls will do nothing.
 *
 * Returns true on success, false on failure. This function
 * should theoretically never fail.
 */
static bool marble_util_clock_internal_init(void) {
#if (defined MB_PLATFORM_WINDOWS)
    /*
     * Check whether the required type is 64 bits.
     * Should never fail.
     */
    static_assert(
        sizeof(((LARGE_INTEGER *)0)->QuadPart) == sizeof(int64_t),
        "Size of high-resolution timestamp must be 64 bits."
    );
    /*
     * Check alignment requirement of int64_t and
     * LARGE_INTEGER. 
     */
    static_assert(
        _Alignof(LARGE_INTEGER) == _Alignof(int64_t),
        "Alignment mismatch between platform-specific HPC datatype and int64_t."
    );

    return QueryPerformanceFrequency((LARGE_INTEGER *)&gl_pfreq);
#else
    #error HPC support is not implemented for this platform.
#endif
}

int64_t marble_util_clock_getfreq(void) {
    return gl_pfreq; 
}

void marble_util_clock_start(
    _Out_ struct marble_util_clock *ps_clock
) {
    if (ps_clock == NULL)
        return;

#if (defined MB_PLATFORM_WINDOWS)
    QueryPerformanceCounter((LARGE_INTEGER *)&ps_clock->m_tstart);
#endif

    ps_clock->m_tend      = 0;
    ps_clock->m_isrunning = true;
}

void marble_util_clock_stop(
    _Inout_ struct marble_util_clock *ps_clock
) {
    if (ps_clock == NULL || !ps_clock->m_isrunning)
        return;

#if (defined MB_PLATFORM_WINDOWS)
    QueryPerformanceCounter((LARGE_INTEGER *)&ps_clock->m_tend);
#endif

    ps_clock->m_isrunning = false;
}

double marble_util_clock_assec(
    _Inout_ struct marble_util_clock const *ps_clock
) {
    if (ps_clock == NULL)
        return -1.0;

#if (defined MB_PLATFORM_WINDOWS)
    if (ps_clock->m_isrunning)
        QueryPerformanceCounter((LARGE_INTEGER *)&ps_clock->m_tend);
#endif

    return (ps_clock->m_tend - ps_clock->m_tstart) / (double)gl_pfreq;
}

double marble_util_clock_asmsec(
    _Inout_ struct marble_util_clock const *ps_clock
) {
    if (ps_clock == NULL)
        return -1.0;

#if (defined MB_PLATFORM_WINDOWS)
    if (ps_clock->m_isrunning)
        QueryPerformanceCounter((LARGE_INTEGER *)&ps_clock->m_tend);
#endif

    return (ps_clock->m_tend - ps_clock->m_tstart) / (gl_pfreq / 1e+3);
}

double marble_util_clock_asmcsec(
    _Inout_ struct marble_util_clock const *ps_clock
) {
    if (ps_clock == NULL)
        return -1.0;

#if (defined MB_PLATFORM_WINDOWS)
    if (ps_clock->m_isrunning)
        QueryPerformanceCounter((LARGE_INTEGER *)&ps_clock->m_tend);
#endif

    return (ps_clock->m_tend - ps_clock->m_tstart) / (gl_pfreq / 1e+6);
}

double marble_util_clock_asnsec(
    _Inout_ struct marble_util_clock const *ps_clock
) {
    if (ps_clock == NULL)
        return -1.0;

#if (defined MB_PLATFORM_WINDOWS)
    if (ps_clock->m_isrunning)
        QueryPerformanceCounter((LARGE_INTEGER *)&ps_clock->m_tend);
#endif

    return (ps_clock->m_tend - ps_clock->m_tstart) / (gl_pfreq / 1e+9);
}

uint64_t marble_util_clock_raw(void) {
#if (defined MB_PLATFORM_WINDOWS)
    LARGE_INTEGER u_time;
    if (!QueryPerformanceCounter(&u_time))
        return 0;

    return (uint64_t)u_time.QuadPart;
#else
    return 0;
#endif
}
#pragma endregion (UTIL-CLOCK)


/*
 * Interface that handles file I/O.
 */
#pragma region UTIL-FILE
struct marble_util_file {
    FILE           *mp_handle; /* file handle */
    struct _stat64  ms_info;   /* file info */
};


_Critical_ marble_ecode_t marble_util_file_open(
    _In_z_           char const *pz_path,
    _In_opt_         int flags,
    _Init_(pps_file) struct marble_util_file **pps_file
) { MB_ERRNO
    if (pz_path == NULL || pps_file == NULL)
        return MARBLE_EC_PARAM;

    /*
     * By default, we open a new file
     * (i.e. overwrite an existing one).
     */
    char const *pz_mode = "w+";

    /* Allocate memory for file object. */
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        sizeof **pps_file,
        false,
        false,
        pps_file
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;
    
    /* Open the file. It must exist. */
    (*pps_file)->mp_handle = NULL;

    /*
     * Check if file exists. If it exists, we will --
     * instead of overwriting the file with an empty
     * one -- open the existing one.
     */
    if (!(flags & MB_UTIL_FILE_FLAG_OVERWRITE) && access(pz_path, 0 /* existence only */) == 0)
        pz_mode = "r+";

    /*
     * If the caller specified MB_UTIL_FILE_FLAG_OVERWRITE
     * or the file does not exist, create a new one.
     */
    if (fopen_s(&(*pps_file)->mp_handle, pz_path, pz_mode) != 0) {
        ecode = MARBLE_EC_OPENFILE;

        goto lbl_END;
    }
    
    /* Read file info. */
    _fstat64(
        _fileno((*pps_file)->mp_handle),
        &(*pps_file)->ms_info
    );
    
lbl_END:
    /*
     * If an error occurs, the file object will be
     * destroyed, and **pps_file** will be set to NULL.
     */
    if (ecode != MARBLE_EC_OK)
        marble_util_file_destroy(pps_file);
    
    return ecode;
}

void marble_util_file_destroy(
    _Uninit_(pps_file) struct marble_util_file **pps_file
) {
    if (pps_file == NULL || *pps_file == NULL)
        return;

    fclose((*pps_file)->mp_handle);

    free(*pps_file);
    *pps_file = NULL;
}

_Success_ok_ marble_ecode_t marble_util_file_goto(
    _In_ struct marble_util_file *ps_file,
         int64_t newpos
) {
    if (ps_file == NULL)
        return MARBLE_EC_PARAM;

    return 
        _fseeki64(
            ps_file->mp_handle,
            newpos,
            SEEK_SET
        ) ? MARBLE_EC_FSEEK : MARBLE_EC_OK;	
}

int64_t marble_util_file_tell(
    _In_ struct marble_util_file *ps_file
) {
    if (ps_file == NULL)
        return -1;

    return _ftelli64(ps_file->mp_handle);
}

_Success_ok_ marble_ecode_t marble_util_file_read(
    _In_         struct marble_util_file *ps_file,
                 size_t size,
    _Size_(size) void *p_dest
) {
    if (ps_file == NULL || size == 0 || p_dest == NULL)
        return MARBLE_EC_PARAM;

    if (fread_s(p_dest, size, 1, size, ps_file->mp_handle) != size)
        return MARBLE_EC_READFILE;

    return MARBLE_EC_OK;
}

_Success_ok_ marble_ecode_t marble_util_file_read8(
    _In_  struct marble_util_file *ps_file,
    _Out_ uint8_t *p_dest
) {
    return marble_util_file_read(ps_file, 1, p_dest);
}

_Success_ok_ marble_ecode_t marble_util_file_read16(
    _In_  struct marble_util_file *ps_file,
    _Out_ uint16_t *p_dest
) {
    return marble_util_file_read(ps_file, 2, p_dest);
}

_Success_ok_ marble_ecode_t marble_util_file_read32(
    _In_  struct marble_util_file *ps_file,
    _Out_ uint32_t *p_dest
) {
    return marble_util_file_read(ps_file, 4, p_dest);
}

_Success_ok_ marble_ecode_t marble_util_file_getinfo(
    _In_  struct marble_util_file *ps_file,
    _Out_ struct _stat64 *ps_info
) {
    if (ps_file == NULL || ps_file->mp_handle == NULL || ps_info == NULL)
        return MARBLE_EC_PARAM;

    marble_system_cpymem(ps_info, &ps_file->ms_info, sizeof *ps_info);

    return MARBLE_EC_OK;
}
#pragma endregion (UTIL-FILE)


/*
 * basic hash-table implementation 
 */
#pragma region UTIL-HASHTABLE
#define MB_UTIL_HTABLE_DEFNBUCKETS (128)

struct marble_util_htable {
    size_t m_cbucket;                     /* number of buckets */
    struct marble_util_vec **pps_storage; /* bucket array */

    marble_dtor_t mfn_dtor;               /* object destructor */
};


#pragma region UTIL-HASHTABLE-INTERNAL
static uint32_t gl_hashseed = UINT32_MAX;


/*
 * Marble uses MurmurHash3 as its hash function.
 * 
 * Source: https://github.com/jwerle/murmurhash.c/blob/master/murmurhash.c
 * 
 * ------------------------------------------
 * 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2014 Joseph Werle
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
static uint32_t marble_util_htable_internal_hash(
    _In_ void const *p_key,
    _In_ size_t const keysize
) {
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    uint32_t r1 = 15;
    uint32_t r2 = 13;
    uint32_t m  = 5;
    uint32_t n  = 0xe6546b64;
    uint32_t h  = 0;
    uint32_t k  = 0;
    uint8_t *d  = (uint8_t *)p_key;

    const uint32_t *chunks = NULL;
    const uint8_t  *tail   = NULL;
    int i = 0;
    int const len = (int)keysize;
    int l = len / 4;

    h = gl_hashseed;

    chunks = (const uint32_t *)(d + (size_t)l * 4);
    tail   = (const uint8_t *)(d + (size_t)l * 4);

    for (i = -l; i != 0; ++i) {
        k = chunks[i];

        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        h ^= k;
        h = (h << r2) | (h >> (32 - r2));
        h = h * m + n;
    }
    k = 0;

    switch (len & 3) {
        case 3: k ^= (tail[2] << 16);
        case 2: k ^= (tail[1] << 8);
        case 1:
            k ^= tail[0];
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;
            h ^= k;
    }
    h ^= len;

    h ^= (h >> 16);
    h *= 0x85ebca6b;
    h ^= (h >> 13);
    h *= 0xc2b2ae35;
    h ^= (h >> 16);

    return h;
}

static marble_ecode_t marble_util_htable_internal_locate(
    _In_ struct marble_util_htable *ps_htable,
    _In_ void *p_key,
    _In_ size_t const keysize,
    _In_ marble_find_t fn_find,
    _Out_ size_t *p_bucketindex,
    _Out_ size_t *p_vecindex
) {
    /* Get index of key in bucket array. */
    *p_bucketindex = marble_util_htable_internal_hash(p_key, keysize) % ps_htable->m_cbucket;

    struct marble_util_vec *ps_bucket = ps_htable->pps_storage[*p_bucketindex];
    if (ps_bucket == NULL)
        goto lbl_END;

    for (size_t i = 0; i < ps_bucket->m_size; i++)
        if (fn_find(p_key, marble_util_vec_get(ps_bucket, i))) {
            *p_vecindex = i;

            return MARBLE_EC_OK;
        }

lbl_END:
    *p_vecindex = (size_t)(-1);

    return MARBLE_EC_NOTFOUND;
}
#pragma endregion (UTIL-HASHTABLE-INTERNAL)


_Critical_ marble_ecode_t marble_util_htable_create(
    _In_opt_           size_t nbuckets,
    _In_opt_           marble_dtor_t fn_dtor,
    _Init_(pps_htable) struct marble_util_htable **pps_htable
) { MB_ERRNO
    if (pps_htable == NULL)
        return MARBLE_EC_PARAM;
    
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        sizeof **pps_htable,
        false,
        false,
        pps_htable
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;
    
    (*pps_htable)->m_cbucket = nbuckets != 0 ? nbuckets : MB_UTIL_HTABLE_DEFNBUCKETS;

    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        sizeof *(*pps_htable)->pps_storage * (*pps_htable)->m_cbucket,
        true,
        false,
        (void **)&(*pps_htable)->pps_storage
    );
    if (ecode != MARBLE_EC_OK)
        goto lbl_END;
    
    (*pps_htable)->mfn_dtor = fn_dtor;
    
lbl_END:
    if (ecode != MARBLE_EC_OK)
        marble_util_htable_destroy(pps_htable);
    
    return ecode;
}

void marble_util_htable_destroy(
    _Uninit_(pps_htable) struct marble_util_htable **pps_htable
) {
    if (pps_htable == NULL || *pps_htable == NULL)
        return;

    /*
     * Destroy all buckets; the destroying of the
     * objects will be taken care of by their
     * buckets.
     */
    for (size_t i = 0; i < (*pps_htable)->m_cbucket; i++)
        marble_util_vec_destroy(&(*pps_htable)->pps_storage[i]);

    free((*pps_htable)->pps_storage);
    free(*pps_htable);
    *pps_htable = NULL;
}

_Success_ok_ marble_ecode_t marble_util_htable_insert(
    _In_ struct marble_util_htable *ps_htable,
    _In_ void *p_key,
    _In_ size_t const keysize,
    _In_ void *p_obj
) { MB_ERRNO
    if (ps_htable == NULL || p_key == NULL  || p_obj == NULL)
        return MARBLE_EC_PARAM;
    
    /* Get index of key in bucket array. */
    uint32_t const index = marble_util_htable_internal_hash(p_key, keysize) % ps_htable->m_cbucket;
    
    /*
     * If the bucket at position **index** does not already exist,
     * create it.
     */
    if (ps_htable->pps_storage[index] == NULL) {
        ecode = marble_util_vec_create(
            0,
            ps_htable->mfn_dtor,
            &ps_htable->pps_storage[index]
        );
    
        if (ecode != MARBLE_EC_OK)
            goto lbl_END;
    }
    
    /*
     * Even if the push fails, we do not destroy the bucket we
     * may just have created.
     */
    ecode = marble_util_vec_pushback(
        ps_htable->pps_storage[index],
        p_obj
    );
    
lbl_END:
    return ecode;
}

void *marble_util_htable_erase(
    _In_ struct marble_util_htable *ps_htable,
    _In_ void *p_key,
    _In_ size_t const keysize,
    _In_ marble_find_t fn_find,
         bool rundest
) { MB_ERRNO
    if (ps_htable == NULL || p_key == NULL || fn_find == NULL)
        return NULL;
    
    /* Get location of element. */
    size_t bucketindex, vecindex;
    ecode = marble_util_htable_internal_locate(
        ps_htable,
        p_key,
        keysize,
        fn_find,
        &bucketindex,
        &vecindex
    );
    if (ecode != MARBLE_EC_OK)
        return NULL;
    
    return marble_util_vec_erase(
        ps_htable->pps_storage[bucketindex],
        vecindex,
        rundest
    );
}

void *marble_util_htable_find(
    _In_ struct marble_util_htable *ps_htable,
    _In_ void *p_key,
    _In_ size_t const keysize,
    _In_ marble_find_t fn_find
) { MB_ERRNO
    if (ps_htable == NULL || p_key == NULL || fn_find == NULL)
        return NULL;
    
    /* Get indices. */
    size_t bucketindex, vecindex;
    ecode = marble_util_htable_internal_locate(
        ps_htable,
        p_key,
        keysize,
        fn_find,
        &bucketindex,
        &vecindex
    );
    
    return ecode == MARBLE_EC_OK
        ? marble_util_vec_get(ps_htable->pps_storage[bucketindex], vecindex)
        : NULL
    ;
}
#pragma endregion (UTIL-HASHTABLE)


/*
 * implementation of a contiguous
 * 2-dimensional array of pointers
 */
#pragma region UTIL-ARRAY2D
/*
 * Computes linear offset of the given 2D-coordinates,
 * relative the first element in the array.
 */
#define MB_UTIL_A2D_LINOFF(x, y, width) ((size_t)(y * width + x))


struct marble_util_array2d {
    size_t m_width;  /* width */
    size_t m_height; /* height */

    /*
     * custom destructor, called for every
     * pointer in **mpp_data**; may be NULL
     */
    marble_dtor_t mfn_dtor;

#if (defined MB_COMPILER_MSVC)
    /*
     * MSVC complains about variable-sized arrays being non-
     * standard despite /std:c11.
     */
    #pragma warning (push)
    #pragma warning (disable: 4200)
#endif
    void *mpp_data[]; /* pointer array */
#if (defined MB_COMPILER_MSVC)
    #pragma warning (pop)
#endif
};


_Critical_ marble_ecode_t marble_util_array2d_create(
    _In_              size_t width,
    _In_              size_t height,
    _In_opt_          marble_dtor_t fn_dtor,
    _Init_(pps_array) struct marble_util_array2d **pps_array
) { MB_ERRNO
    if (width == 0 || height == 0 || pps_array == NULL)
        return MARBLE_EC_PARAM;

    /* Allocate memory for structure and array. */
    ecode = marble_system_alloc(
        MB_CALLER_INFO,
        sizeof **pps_array + width * height * sizeof(void *),
        true,
        false,
        pps_array
    );
    if (ecode != MARBLE_EC_OK)
        return ecode;

    /* Init state. */
    (*pps_array)->m_width  = width;
    (*pps_array)->m_height = height;
    (*pps_array)->mfn_dtor = fn_dtor;

    return ecode;
}

void marble_util_array2d_destroy(
    _Uninit_(pps_array) struct marble_util_array2d **pps_array
) {
    if (pps_array == NULL || *pps_array == NULL)
        return;

    /*
     * Call known destructor; or none if
     * **mfn_dtor** is NULL.
     */
    if ((*pps_array)->mfn_dtor != NULL)
        for (size_t i = 0; i < (*pps_array)->m_width * (*pps_array)->m_height; i++)
            (*(*pps_array)->mfn_dtor)(&(*pps_array)->mpp_data[i]);

    free(*pps_array);
    *pps_array = NULL;
}

_Critical_ marble_ecode_t marble_util_array2d_resize(
    _Reinit_opt_(pps_array) struct marble_util_array2d **pps_array,
    _In_                    size_t nwidth,
    _In_                    size_t nheight
) { MB_ERRNO
    if (pps_array == NULL || *pps_array == NULL || nwidth == 0 || nheight == 0)
        return MARBLE_EC_PARAM;

    struct marble_util_array2d *ps_narray = NULL;

    /*
     * Check if new sizes differ from the old ones. If they don't,
     * there is no point in resizing.
     */
    if ((*pps_array)->m_width == nwidth && (*pps_array)->m_height == nheight)
        return MARBLE_EC_OK;

    /*
     * First, try to allocate a new array of the requested size.
     * (We cannot attempt to realloc() the array directly, since
     * we would first have to move all valid contents, which in
     * itself is possible. However, if the reallocation fails,
     * we would end up with an invalid array, and, as we have
     * already touched the array's memory, invalid data).
     * It's more reliable to first allocate a new array and then
     * move all contents that lie within the bounds of the old
     * array to the new one. If the creation of that new array
     * fails, the function should also fail.
     * 
     * In the future, we may want to optimize for some embedded
     * platforms where memory is sparce. In such a case, it may
     * be preferable to resize the array first at the risk of
     * invalidating it in the process.
     */
    ecode = marble_util_array2d_create(
        nwidth,
        nheight,
        (*pps_array)->mfn_dtor,
        &ps_narray
    );
    if (ecode != MARBLE_EC_OK)
        return ecode;

    /*
     * Copy-over the part of the old array that is congruent with
     * the new array. As of now, we always initiate copying relative
     * to the "upper-left corner" of the array; arbitrary offsets
     * are not supported.
     */
    for (size_t row = 0; row < min(nheight, (*pps_array)->m_height); row++)
        marble_system_cpymem(
            &ps_narray->mpp_data[0],
            &(*pps_array)->mpp_data[(*pps_array)->m_width * row],
            min(nwidth, (*pps_array)->m_width) * sizeof(void *)
        );

    /*
     * Destroy old array and update given array pointer. We have to
     * remove the destructor, though, since that would destroy the
     * copied elements as well, which is not precisely what we want.
     */
    (*pps_array)->mfn_dtor = NULL;
    marble_util_array2d_destroy(pps_array);

    *pps_array = ps_narray;
    return MARBLE_EC_OK;
}

_Success_ok_ marble_ecode_t marble_util_array2d_insert(
    _In_         struct marble_util_array2d *ps_array,
                 size_t posx,
                 size_t posy,
                 bool rundest,
    _In_         void *p_element,
    _Outptr_opt_ void **pp_oldobj
) {
    size_t const off = MB_UTIL_A2D_LINOFF(posx, posy, ps_array->m_width);

    if (ps_array <= NULL
        || p_element == NULL
        || p_element == MB_INVPTR
    ) return MARBLE_EC_PARAM;
    if (off >= ps_array->m_width * ps_array->m_height)
        return MARBLE_EC_OUTOFRANGE;

    /*
     * Get pointer to the memory location of the object
     * in question.
     */
    void **pp_obj = &ps_array->mpp_data[off];

    /*
     * Run destructor, but only if the caller explicitly
     * instructed the function to do so and they did not
     * pass a valid pointer for **pp_oldobj**. If **rundest**
     * is true, and **pp_oldobj** is valid, the function
     * fails.
     */
    if (rundest && pp_oldobj > NULL)
        return MARBLE_EC_PCONTRADICTION;
    else if (rundest) {
        (*ps_array->mfn_dtor)(pp_obj);

        return MARBLE_EC_OK;
    } else if (pp_oldobj > NULL)
        *pp_oldobj = pp_obj;

    *pp_obj = p_element;
    return MARBLE_EC_OK;
}

_Success_ptr_ void *marble_util_array2d_erase(
    _In_ struct marble_util_array2d *ps_array,
         size_t posx,
         size_t posy,
         bool rundest
) {
    size_t const off = MB_UTIL_A2D_LINOFF(posx, posy, ps_array->m_width);

    if (ps_array == NULL || off >= ps_array->m_width * ps_array->m_height)
        return MB_INVPTR;

    void **pp_obj = &ps_array->mpp_data[off];

    /* Call destructor if there is one and return. */
    if (rundest && ps_array->mfn_dtor != NULL && *pp_obj != NULL) {
        (*ps_array->mfn_dtor)(pp_obj);
        
        return NULL;
    }

    /* Return pointer to existing object. */
    return *pp_obj;
}

_Success_ptr_ void *marble_util_array2d_get(
    _In_ struct marble_util_array2d *ps_array,
         size_t posx,
         size_t posy
) {
    size_t const off = MB_UTIL_A2D_LINOFF(posx, posy, ps_array->m_width);

    if (ps_array == NULL || off >= ps_array->m_width * ps_array->m_height)
        return MB_INVPTR;

    return ps_array->mpp_data[off];
}
#pragma endregion (UTIL-ARRAY2D)


bool marble_util_init(void) {
    /*
     * Frequency of HPC; calling this multiple times is okay
     * since the frequency is always the same for the current
     * session.
     */
    if (!marble_util_clock_internal_init())
        return false;

    /*
     * Initialize hash seed. This shall only be done once
     * because a different seed will change all subsequent
     * hash values, causing big problems with the existing
     * keys.
     * The default value of the hash seed is UINT32_MAX. After
     * this function has been called, it will hold a different
     * (smaller) value, signifying it has been initialized.
     */
    if (gl_hashseed == UINT32_MAX) {
#if (defined MB_PLATFORM_WINDOWS)
        LARGE_INTEGER u_pc = { .QuadPart = (LONGLONG)UINT32_MAX };
        QueryPerformanceCounter(&u_pc);

        gl_hashseed = (uint32_t)(u_pc.QuadPart % UINT32_MAX);
#else
        /*
         * Use time() (i.e. Fallback #1) for other platforms than
         * Windows for now.
         */
        gl_hashseed = 0;
#endif
        /* Fallback #1: If seed is 0, use a different timer. */
        if (gl_hashseed == 0)
            gl_hashseed = ((uint32_t)_time32(NULL)) % UINT32_MAX;

        marble_log_debug(__func__, "seed=%u", gl_hashseed);

        /*
         * If we still cannot get a valid hash, just call it
         * quits.
         */
        if (gl_hashseed == 0)
            return false;
    }

    return true;
}


