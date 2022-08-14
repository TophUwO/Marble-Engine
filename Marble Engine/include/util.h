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
	struct marble_util_vec **pps_vector /* vector to destroy */ 
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
marble_ecode_t inline marble_util_vec_create(
	size_t startcap,                      /* initial capacity */
	void (MB_CALLBACK *fn_dest)(void **), /* object destructor */
	/*
	 * Pointer to a "struct marble_util_vec *" pointer that will
	 * receive the pointer to the newly-created vector.
	 */
	struct marble_util_vec **pps_vector
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
marble_ecode_t inline marble_util_vec_insert(
	struct marble_util_vec *ps_vector, /* vector to modify */
	void *p_obj,                       /* object pointer to insert */
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
marble_ecode_t inline marble_util_vec_pushback(
	struct marble_util_vec *ps_vector, /* vector to modify */
	void *p_obj                        /* object to insert */
) {
	return marble_util_vec_insert(ps_vector, p_obj, ps_vector->m_size);
}

/*
 * Inserts **p_obj** at the beginning (that is, index 0).
 * If **p_obj** is NULL, it will not be inserted.
 * 
 * Returns 0 on success, non-zero on failure.
 */
marble_ecode_t inline marble_util_vec_pushfront(
	struct marble_util_vec *ps_vector, /* vector to modify */
	void *p_obj                        /* object to insert */
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
	struct marble_util_vec *ps_vector, /* vector to modify */
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
	struct marble_util_vec *ps_vector, /* vector to modify */
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
	struct marble_util_vec *ps_vector, /* vector to modify */
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
	struct marble_util_vec *ps_vector, /* vector to modify */
	void *p_obj,                       /* object to search for */
	size_t start,                      /* first index of searching range */
	size_t end                         /* last index of searching range */
) {
	if (ps_vector == NULL || p_obj == NULL || start > end || start >= ps_vector->m_size || end >= ps_vector->m_size)
		return MARBLE_EC_PARAM;

	/*
	 * If **start** and **end** are equal, and they are 0,
	 * set **end** to the last index of the last element
	 * in the vector.
	 */
	end = start == end && start == 0 ? ps_vector->m_size - 1 : end;

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
	struct marble_util_vec *ps_vector, /* vector to read */
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
	struct marble_util_clock *ps_clock /* clock to start/reset */
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
	struct marble_util_clock *ps_clock /* clock to stop */
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
	struct marble_util_clock *ps_clock /* clock */
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
	struct marble_util_clock *ps_clock /* clock */
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
	struct marble_util_clock *ps_clock /* clock */
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
	struct marble_util_clock *ps_clock /* clock */
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
	struct marble_util_file **pps_file /* file to destroy */
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
marble_ecode_t inline marble_util_file_open(
	char const *pz_path,               /* file path */
	enum marble_util_file_perms perms, /* permissions */
	/*
	 * pointer to receive the pointer
	 * to the file object
	 */
	struct marble_util_file **pps_file
) { MB_ERRNO
	if (pz_path == NULL || pps_file == NULL)
		return MARBLE_EC_PARAM;

	/* Allocate memory file object. */
	ecode = marble_system_alloc(
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
marble_ecode_t inline marble_util_file_goto(
	struct marble_util_file *ps_file,
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
marble_ecode_t inline marble_util_file_read(
	struct marble_util_file *ps_file, /* file object */
	size_t size,                      /* number of bytes to read */
	void *p_dest                      /* buffer to write read bytes to */
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
marble_ecode_t inline marble_util_file_read8(
	struct marble_util_file *ps_file, /* file object */
	uint8_t *p_dest                   /* destination buffer */
) {
	return marble_util_file_read(ps_file, 1, p_dest);
}

/*
 * Reads two bytes from **ps_file** and stores
 * them inside **p_dest**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
marble_ecode_t inline marble_util_file_read16(
	struct marble_util_file *ps_file, /* file object */
	uint16_t *p_dest				  /* destination buffer */
) {
	return marble_util_file_read(ps_file, 2, p_dest);
}

/*
 * Reads four bytes from **ps_file** and stores
 * them inside **p_dest**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
marble_ecode_t inline marble_util_file_read32(
	struct marble_util_file *ps_file, /* file object */
	uint32_t *p_dest				  /* destination buffer */
) {
	return marble_util_file_read(ps_file, 4, p_dest);
}
#pragma endregion


/*
 * Structure representing a basic hashtable. 
 */
#pragma region UTIL-HASHTABLE
struct marble_util_htable {
	size_t m_cbucket;                     /* number of buckets */
	struct marble_util_vec **pps_storage; /* bucket array */

	void (*mfn_dest)(void **);            /* object destructor */
};


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
	struct marble_util_htable **pps_hashtable /* hashtable to destroy */
) {
	if (pps_hashtable == NULL || *pps_hashtable == NULL)
		return;

	/*
	 * Destroy all buckets; the destroying of the
	 * objects will be taken care of by their
	 * buckets.
	 */
	for (size_t i = 0; i < (*pps_hashtable)->m_cbucket; i++)
		marble_util_vec_destroy(&(*pps_hashtable)->pps_storage[i]);

	free((*pps_hashtable)->pps_storage);
	free(*pps_hashtable);
	*pps_hashtable = NULL;
}

/*
 * Creates a hashtable and initializes its bucket array.
 * Note that the buckets themselves will not be
 * initialized until an attempt to add an element to
 * the bucket was made.
 * 
 * Returns 0 on success, non-zero on failure.
 */
marble_ecode_t inline marble_util_htable_create(
	size_t nbuckets,          /* number of buckets */
	void (*fn_dest)(void **), /* object destructor */
	/*
	 * pointer to receive the pointer
	 * to the hashtable object
	 */
	struct marble_util_htable **pps_hashtable
) {
	if (pps_hashtable == NULL)
		return MARBLE_EC_PARAM;

	*pps_hashtable = NULL;
	return MARBLE_EC_OK;
}

/*
 * Inserts **p_obj** into **ps_hashtable** using
 * the hash value of **pz_key**. **pz_key** should
 * be the same as the key of **p_obj** internally.
 * 
 * Returns 0 on success, non-zero on failure.
 */
marble_ecode_t inline marble_util_htable_insert(
	struct marble_util_htable *ps_hashtable, /* hashtable */
	char const *pz_key,                      /* **p_obj**'s key */
	void *p_obj                              /* object to insert */
) {
	return MARBLE_EC_UNIMPLFEATURE;
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
	struct marble_util_htable *ps_hashtable, /* hashtable */
	char const *pz_key,                      /* key */
	/*
	 * The callback function shall return non-zero if
	 * the element was found, and 'false' if not.
	 * If this parameter is NULL, the function fails
	 * and returns NULL.
	 */
	bool (*fn_find)(char const *, void *),
	bool rundest                             /* destroy (found) object? */
) {
	return NULL;
}

/*
 * Finds an object that can be associated with
 * **pz_key**.
 * 
 * Returns the pointer to the object, or NULL
 * on error or if the object cannot be found.
 */
void inline *marble_util_htable_find(
	struct marble_util_htable *ps_hashtable, /* hashtable */
	char const *pz_key,                      /* key to find */
	/*
	 * The callback function shall return non-zero if
	 * the element was found, and 'false' if not.
	 * If this parameter is NULL, the function fails
	 * and returns NULL.
	 */
	bool (*fn_find)(char const *, void *)
) {
	return NULL;
}
#pragma endregion


