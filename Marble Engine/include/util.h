#pragma once

#include <api.h>
#include <system.h>


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
	void (MB_CALLBACK *mfn_dest)(void **);
};


/*
 * Destroys a vector, optionally freeing all of its objects,
 * and releasing all of the resources occupied by it.
 * 
 * Returns nothing.
 */
void inline marble_util_vec_destroy(
	_Uninit_(pps_vector) struct marble_util_vec **pps_vector /* vector to destroy */ 
) {
	if (pps_vector == NULL || *pps_vector == NULL)
		return;

	/* Call the destructors. */
	if ((*pps_vector)->mfn_dest)
		for (size_t i = 0; i < (*pps_vector)->m_size; i++)
			(*(*pps_vector)->mfn_dest)(&(*pps_vector)->mpp_data[i]);

	/* Free memory used by the vector itself. */
	free((*pps_vector)->mpp_data);
	free(*pps_vector);
	*pps_vector = NULL;
}

/*
 * Create and initialize a vector.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ marble_ecode_t inline marble_util_vec_create(
	_In_opt_           size_t startcap,                      /* initial capacity */
	_In_opt_           void (MB_CALLBACK *fn_dest)(void **), /* object destructor */
	/*
	 * Pointer to a "struct marble_util_vec *" pointer that will
	 * receive the pointer to the newly-created vector.
	 */
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
	(*pps_vector)->mfn_dest = fn_dest;
	(*pps_vector)->m_cap    = startcap;
	(*pps_vector)->m_defcap = startcap;
	(*pps_vector)->m_size   = 0;

lbl_END:
	if (ecode != MARBLE_EC_OK)
		marble_util_vec_destroy(pps_vector);

	return ecode;
}

/*
 * Inserts **p_obj** at index **index**. If **p_obj** is
 * NULL, it will not be inserted. If **index** is out of
 * bounds, the function will fail.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_util_vec_insert(
	_In_ struct marble_util_vec *ps_vector, /* vector to modify */
	_In_ void *p_obj,                       /* object pointer to insert */
	     size_t index                       /* position where to insert **p_obj** */
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

		/* Update data. */
		ps_vector->m_cap    = newcap;
		ps_vector->mpp_data = (void **)p_new;
	}

	/* In case we just want to add **p_obj**
	 * to the end, do not even attempt
	 * to move any memory.
	 */
	if (index == ps_vector->m_size) {
		ps_vector->mpp_data[ps_vector->m_size] = p_obj;

		goto lbl_END;
	}

	/* If **p_obj** is to be inserted in the middle
	 * or at the beginning, we have to move the following
	 * block by one position to make space for the pointer.
	 */
	memmove(
		&ps_vector->mpp_data[index + 1],
		&ps_vector->mpp_data[index],
		(ps_vector->m_size - index) * sizeof *ps_vector->mpp_data
	);
	/* Insert **p_obj**. */
	ps_vector->mpp_data[index] = p_obj;

lbl_END:
	++ps_vector->m_size;
	return MARBLE_EC_OK;
}

/*
 * Inserts **p_obj** at the end. If **p_obj** is
 * NULL, it will not be inserted.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_util_vec_pushback(
	_In_ struct marble_util_vec *ps_vector, /* vector to modify */
	_In_ void *p_obj                        /* object to insert */
) {
	return marble_util_vec_insert(ps_vector, p_obj, ps_vector->m_size);
}

/*
 * Inserts **p_obj** at the beginning (that is, index 0).
 * If **p_obj** is NULL, it will not be inserted.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_util_vec_pushfront(
	_In_ struct marble_util_vec *ps_vector, /* vector to modify */
	_In_ void *p_obj                        /* object to insert */
) {
	return marble_util_vec_insert(ps_vector, p_obj, 0);
}

/*
 * Erases the object at index **index**. If 
 * the **mfn_dest** member of **ps_vector** is not
 * NULL, the object will be destroyed and its memory
 * released by calling **ps_vector->mfn_dest()**.
 * 
 * Returns NULL in case of an error or if the object was freed,
 * or the pointer to the object at index **index** is the
 * object was not freed.
 */
void inline *marble_util_vec_erase(
	_In_ struct marble_util_vec *ps_vector, /* vector to modify */
	     size_t index,                      /* object to insert */
	     /*
	      * This member can be used to override the freeing-behavior
	      * of this function. If this parameter is non-zero, it will
	      * not run the destructor, even if it is set and valid.
	      */
	     bool rundest
) {
	if (ps_vector == NULL || index >= ps_vector->m_size)
		return NULL;

	/* Call object destructor if necessary. */
	if (rundest != false && ps_vector->mfn_dest != NULL)
		(*ps_vector->mfn_dest)(&ps_vector->mpp_data[index]);

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

/*
 * Erases the last element in **ps_vector**.
 * 
 * Returns NULL if the vector is empty, there was an
 * error or it has been destroyed. If the object was
 * not destroyed, the function returns the pointer to
 * it.
 */
void inline *marble_util_vec_popback(
	_In_ struct marble_util_vec *ps_vector, /* vector to modify */
	     bool rundest                       /* run object destructor? */
) {
	return marble_util_vec_erase(ps_vector, ps_vector->m_size - 1, rundest);
}

/*
 * Erases the first element in **ps_vector**.
 * 
 * Returns NULL if the vector is empty, there was an
 * error or it has been destroyed. If the object was
 * not destroyed, the function returns the pointer to
 * it.
 */
void inline *marble_util_vec_popfront(
	_In_ struct marble_util_vec *ps_vector, /* vector to modify */
	     bool rundest                       /* run object destructor? */
) {
	return marble_util_vec_erase(ps_vector, 0, rundest);
}

/*
 * Returns the index of the first occurrence
 * of **p_obj** inside **ps_vector**. The search-range
 * can be determined by setting **start** and **end**
 * to valid indices. If both **start** and **end** are
 * 0, the entire vector will be searched.
 *
 * If there are no occurrences, the function
 * returns (size_t)(-1).
 */
size_t inline marble_util_vec_find(
	_In_ struct marble_util_vec *ps_vector, /* vector to modify */
	_In_ void *p_obj,                       /* object to search for */
	     size_t start,                      /* first index of searching range */
	     size_t end                         /* last index of searching range */
) {
	if (ps_vector == NULL || p_obj == NULL || start > end || start >= ps_vector->m_size || end >= ps_vector->m_size)
		return (size_t)(-1);

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
	return (size_t)(-1);
}

/*
 * Returns the object pointer at index **index**.
 * Even though the "struct marble_util_vec" is not opaque,
 * thus allowing everyone to read **ps_vector->mpp_data**
 * directly,this function allows for more safety as it
 * will carry-out some parameter validation while also
 * increasing readability of code.
 * 
 * Returns object pointer at index **index**, or NULL
 * on error.
 */
void inline *marble_util_vec_get(
	_In_ struct marble_util_vec *ps_vector, /* vector to read */
	     size_t index                       /* index */
) {
	if (ps_vector == NULL || index >= ps_vector->m_size)
		return NULL;

	return ps_vector->mpp_data[index];
}
#pragma endregion


/*
 * Structure representing an HPC (uses platform-specific
 * means of measuring time with high resolution).
 */
#pragma region UTIL-CLOCK
extern uint64_t gl_pfreq; /* variable holding frequency of HPC */


struct marble_util_clock {
	uint64_t m_tstart; /* start time */
	uint64_t m_tend;   /* end time */
};


/*
 * Starts/Restarts a clock.
 * 
 * Returns nothing.
 */
void inline marble_util_clock_start(
	_Out_ struct marble_util_clock *ps_clock /* clock to start/reset */
) {
#if (defined MB_PLATFORM_WINDOWS)
	QueryPerformanceCounter((LARGE_INTEGER *)&ps_clock->m_tstart);
#endif
}

/*
 * Stops a clock.
 * It is now safe to calculate elapsed time.
 * 
 * Returns nothing.
 */
void inline marble_util_clock_stop(
	_Out_ struct marble_util_clock *ps_clock /* clock to stop */
) {
#if (defined MB_PLATFORM_WINDOWS)
	QueryPerformanceCounter((LARGE_INTEGER *)&ps_clock->m_tend);
#endif
}

/*
 * Calculates time between **m_tstart** and **m_tend**,
 * in seconds.
 * 
 * Returns elapsed time, in seconds.
 */
double inline marble_util_clock_assec(
	_In_ struct marble_util_clock *ps_clock /* clock */
) {
	return (ps_clock->m_tend - ps_clock->m_tstart) / (double)gl_pfreq;
}

/*
 * Calculates time between **m_tstart** and **m_tend**,
 * in milliseconds.
 * 
 * Returns elapsed time, in milliseconds.
 */
double inline marble_util_clock_asmsec(
	_In_ struct marble_util_clock *ps_clock /* clock */
) {
	return (ps_clock->m_tend - ps_clock->m_tstart) / (gl_pfreq / 1e+3);
}

/*
 * Calculates time between **m_tstart** and **m_tend**,
 * in microseconds.
 * 
 * Returns elapsed time, in microseconds.
 */
double inline marble_util_clock_asmcsec(
	_In_ struct marble_util_clock *ps_clock /* clock */
) {
	return (ps_clock->m_tend - ps_clock->m_tstart) / (gl_pfreq / 1e+6);
}

/*
 * Calculates time between **m_tstart** and **m_tend**,
 * in nanoseconds.
 * 
 * Returns elapsed time, in nanoseconds.
 */
double inline marble_util_clock_asnsec(
	_In_ struct marble_util_clock *ps_clock /* clock */
) {
	return (ps_clock->m_tend - ps_clock->m_tstart) / (gl_pfreq / 1e+9);
}
#pragma endregion


/*
 * Interface that handles file I/O.
 */
#pragma region UTIL-FILE
/*
 * These flags can be combined to allow
 * for multiple access rights.
 */
enum marble_util_file_perms {
	MARBLE_UTIL_FILEPERM_READ  = 1 << 0, /* open file for reading */
	MARBLE_UTIL_FILEPERM_WRITE = 1 << 1  /* open file for writing */
};

struct marble_util_file {
	FILE *mp_handle; /* file handle */

	struct stat                 ms_info; /* file info */
	enum marble_util_file_perms m_perms; /* permissions */
};


/*
 * Closes and destroys a file.
 * **pps_file** will be NULL after
 * the function returns.
 * 
 * Returns nothing.
 */
void inline marble_util_file_destroy(
	_Uninit_(pps_file) struct marble_util_file **pps_file /* file to destroy */
) {
	if (pps_file == NULL || *pps_file == NULL)
		return;

	fclose((*pps_file)->mp_handle);

	free(*pps_file);
	*pps_file = NULL;
}

/*
 * Opens a file.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ marble_ecode_t inline marble_util_file_open(
	_In_z_           char const *pz_path,               /* file path */
	                 enum marble_util_file_perms perms, /* permissions */
	                 /*
	                  * pointer to receive the pointer
	                  * to the file object
	                  */
	_Init_(pps_file) struct marble_util_file **pps_file
) { MB_ERRNO
	if (pz_path == NULL || pps_file == NULL)
		return MARBLE_EC_PARAM;

	/* Allocate memory file object. */
	ecode = marble_system_alloc(
		MB_CALLER_INFO,
		sizeof **pps_file,
		false,
		false,
		pps_file
	);
	if (ecode != MARBLE_EC_OK)
		goto lbl_END;

	/* Open the file. */
	(*pps_file)->mp_handle = NULL;
	if (fopen_s(&(*pps_file)->mp_handle, pz_path, "rb" /* TODO: change later */) != 0)
		ecode = MARBLE_EC_OPENFILE;

lbl_END:
	/*
	 * If an error occurs, the file object will be
	 * destroyed, and **pps_file** will be set to NULL.
	 */
	if (ecode != MARBLE_EC_OK)
		marble_util_file_destroy(pps_file);

	return ecode;
}

/*
 * Sets the position of the internal file indicator.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_util_file_goto(
	_In_ struct marble_util_file *ps_file,
	     size_t newpos
) {
	if (ps_file == NULL)
		return MARBLE_EC_PARAM;

	return _fseeki64(ps_file->mp_handle, (long long)newpos, SEEK_SET) ? MARBLE_EC_FSEEK : MARBLE_EC_OK;	
}

/*
 * Reads **size** bytes from **ps_file**.
 * The bytes will be stored in **p_dest**. If the
 * buffer is not large enough to hold all of the
 * read bytes, the behavior is undefined.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_util_file_read(
	_In_         struct marble_util_file *ps_file, /* file object */
	             size_t size,                      /* number of bytes to read */
	_Size_(size) void *p_dest                      /* buffer to write read bytes to */
) {
	if (ps_file == NULL || size == 0 || p_dest == NULL)
		return MARBLE_EC_PARAM;

	/* currently not implemented. */
	return MARBLE_EC_UNIMPLFEATURE;
}

/*
 * Reads a byte from **ps_file** and stores
 * it inside **p_dest**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_util_file_read8(
	_In_  struct marble_util_file *ps_file, /* file object */
	_Out_ uint8_t *p_dest                   /* destination buffer */
) {
	return marble_util_file_read(ps_file, 1, p_dest);
}

/*
 * Reads two bytes from **ps_file** and stores
 * them inside **p_dest**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_util_file_read16(
	_In_  struct marble_util_file *ps_file, /* file object */
	_Out_ uint16_t *p_dest				    /* destination buffer */
) {
	return marble_util_file_read(ps_file, 2, p_dest);
}

/*
 * Reads four bytes from **ps_file** and stores
 * them inside **p_dest**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_util_file_read32(
	_In_  struct marble_util_file *ps_file, /* file object */
	_Out_ uint32_t *p_dest				    /* destination buffer */
) {
	return marble_util_file_read(ps_file, 4, p_dest);
}
#pragma endregion


/*
 * Structure representing a basic hashtable. 
 */
#pragma region UTIL-HASHTABLE
#define MB_UTIL_HTABLE_DEFNBUCKETS (128)

struct marble_util_htable {
	size_t m_cbucket;                     /* number of buckets */
	struct marble_util_vec **pps_storage; /* bucket array */

	void (*mfn_dest)(void **);            /* object destructor */
};


#pragma region UTIL-HASHTABLE-INTERNAL
extern uint32_t gl_hashseed;


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
uint32_t inline __marble_util_htable_hash(
	_In_z_ char const *pz_key
) {
	uint32_t c1 = 0xcc9e2d51;
	uint32_t c2 = 0x1b873593;
	uint32_t r1 = 15;
	uint32_t r2 = 13;
	uint32_t m  = 5;
	uint32_t n  = 0xe6546b64;
	uint32_t h  = 0;
	uint32_t k  = 0;
	uint8_t *d  = (uint8_t *)pz_key;

	const uint32_t *chunks = NULL;
	const uint8_t  *tail   = NULL;
	int i = 0;
	int const len = (int)strlen(pz_key);
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

/*
 * Attempt to locate an element inside **ps_htable**.
 * If the element is found is determined by running
 * **fn_find()** over every element in the bucket the
 * hash value of **pz_key** evaluates to.
 * If the function succeeds, **p_bucketindex** and **p_vecindex**
 * will be set to the indices in both the bucket array and
 * the bucket itself. A value of (size_t)(-1) indicates failure.
 * 
 * Returns 0 on success, non-zero on failure.
 */
marble_ecode_t inline __marble_util_htable_locate(
	_In_   struct marble_util_htable *ps_htable, /* hashtable to search */
	_In_z_ char const *pz_key,                   /* key to search for */
	/* callback function */
	_In_   bool (*fn_find)(
		_In_z_ char const *,
		_In_ void *
	),
	_Out_  size_t *p_bucketindex,                /* index of bucket */
	_Out_  size_t *p_vecindex                    /* index of element in bucket */
) {
	/* Get index of key in bucket array. */
	*p_bucketindex = __marble_util_htable_hash(pz_key) % ps_htable->m_cbucket;

	struct marble_util_vec *ps_bucket = ps_htable->pps_storage[*p_bucketindex];
	if (ps_bucket == NULL)
		goto lbl_END;

	for (size_t i = 0; i < ps_bucket->m_size; i++)
		if (fn_find(pz_key, marble_util_vec_get(ps_bucket, i)) != false) {
			*p_vecindex = i;

			return MARBLE_EC_OK;
		}

lbl_END:
	*p_vecindex = (size_t)(-1);

	return MARBLE_EC_NOTFOUND;
}
#pragma endregion


/*
 * Destroys a hashtable objects, all of its buckets, and,
 * if its **pps_hashtable->mfn_dest** member is not NULL,
 * all of the objects the hashtable holds using its
 * **pps_hashtable->mfn_dest()** function.
 * 
 * When the function returns, **pps_hashtable** will
 * be NULL.
 * 
 * Returns nothing.
 */
void inline marble_util_htable_destroy(
	_Uninit_(pps_htable) struct marble_util_htable **pps_htable /* hashtable to destroy */
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

/*
 * Creates a hashtable and initializes its bucket array.
 * Note that the buckets themselves will not be
 * initialized until an attempt to add an element to
 * the bucket was made.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Critical_ marble_ecode_t inline marble_util_htable_create(
	_In_opt_           size_t nbuckets,          /* number of buckets */
	_In_opt_           void (*fn_dest)(void **), /* object destructor */
	                   /*
	                    * pointer to receive the pointer
	                    * to the hashtable object
	                    */
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

	(*pps_htable)->mfn_dest = fn_dest;

lbl_END:
	if (ecode != MARBLE_EC_OK)
		marble_util_htable_destroy(pps_htable);

	return ecode;
}

/*
 * Inserts **p_obj** into **ps_hashtable** using
 * the hash value of **pz_key**. **pz_key** should
 * be the same as the key of **p_obj** internally.
 * 
 * Returns 0 on success, non-zero on failure.
 */
_Success_ok_ marble_ecode_t inline marble_util_htable_insert(
	_In_   struct marble_util_htable *ps_htable, /* hashtable */
	_In_z_ char const *pz_key,                   /* **p_obj**'s key */
	_In_   void *p_obj                           /* object to insert */
) { MB_ERRNO
	if (ps_htable == NULL || pz_key == NULL || *pz_key == '\0' || p_obj == NULL)
		return MARBLE_EC_PARAM;

	/* Get index of key in bucket array. */
	uint32_t const index = __marble_util_htable_hash(pz_key) % ps_htable->m_cbucket;

	/*
	 * If the bucket at position **index** does not already exist,
	 * create it.
	 */
	if (ps_htable->pps_storage[index] == NULL) {
		ecode = marble_util_vec_create(
			0,
			ps_htable->mfn_dest,
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

/*
 * Erases an object whose key is equal to **pz_key**
 * from **ps_hashtable**. Equality is determined by the
 * return value of **fn_find**, a user-defined callback
 * function.
 * 
 * Returns NULL on error or if the element got destroyed, or
 * the element's pointer on success.
 */
void inline *marble_util_htable_erase(
	_In_   struct marble_util_htable *ps_htable, /* hashtable */
	_In_z_ char const *pz_key,                   /* key */
	/*
	 * The callback function shall return non-zero if
	 * the element was found, and 'false' if not.
	 * If this parameter is NULL, the function fails
	 * and returns NULL.
	 */
	_In_   bool (*fn_find)(char const *, void *),
	       bool rundest                          /* destroy (found) object? */
) { MB_ERRNO
	if (ps_htable == NULL || pz_key == NULL || *pz_key == '\0' || fn_find == NULL)
		return NULL;

	/* Get location of element. */
	size_t bucketindex, vecindex;
	ecode = __marble_util_htable_locate(
		ps_htable,
		pz_key,
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

/*
 * Finds an object that can be associated with
 * **pz_key**.
 * 
 * Returns the pointer to the object, or NULL
 * on error or if the object cannot be found.
 */
void inline *marble_util_htable_find(
	_In_   struct marble_util_htable *ps_htable, /* hashtable */
	_In_z_ char const *pz_key,                   /* key to find */
	/*
	 * The callback function shall return non-zero if
	 * the element was found, and 'false' if not.
	 * If this parameter is NULL, the function fails
	 * and returns NULL.
	 */
	_In_   bool (*fn_find)(char const *, void *)
) { MB_ERRNO
	if (ps_htable == NULL || pz_key == NULL || *pz_key == '\0' || fn_find == NULL)
		return NULL;

	/* Get indices. */
	size_t bucketindex, vecindex;
	ecode = __marble_util_htable_locate(
		ps_htable,
		pz_key,
		fn_find,
		&bucketindex,
		&vecindex
	);
	
	return ecode == MARBLE_EC_OK
		? marble_util_vec_get(ps_htable->pps_storage[bucketindex], vecindex)
		: NULL
	;
}
#pragma endregion


