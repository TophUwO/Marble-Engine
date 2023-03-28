#pragma once

#include <api.h>
#include <system.h>


MB_BEGIN_HEADER


/*
 * Destructor functions for all object types must be
 * of this form. This is especially important for
 * "marble_util_vec" etc. utils.
 */
typedef void (MB_CALLBACK *marble_dtor_t)(void **);
/*
 * Function used to identify an object inside a
 * hash table, etc.
 * The first parameter is the current object that
 * is being examined while the second argument
 * is custom data that is used to determine if
 * the object is the one searched for; this can be
 * a key for instance.
 * object. If they match, the function shall return
 * true.
 */
typedef bool (MB_CALLBACK *marble_find_t)(void *, void *);


#pragma region UTIL-VECTOR
struct marble_util_vec;

/*
 * Create and initialize a vector.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Critical_ marble_ecode_t marble_util_vec_create(
    _In_opt_           size_t startcap,       /* initial capacity */
    _In_opt_           marble_dtor_t fn_dtor, /* object destructor */
    /*
     * Pointer to a "struct marble_util_vec *" pointer that will
     * receive the pointer to the newly-created vector.
     */
    _Init_(pps_vector) struct marble_util_vec **pps_vector
);

/*
 * Destroys a vector, optionally freeing all of its objects,
 * and releasing all of the resources occupied by it.
 * 
 * Returns nothing.
 */
MB_API void marble_util_vec_destroy(
    _Uninit_(pps_vector) struct marble_util_vec **pps_vector /* vector to destroy */ 
);

/*
 * Inserts **p_obj** at index **index**. If **p_obj** is
 * NULL, it will not be inserted. If **index** is out of
 * bounds, the function will fail.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_vec_insert(
	_Inout_ struct marble_util_vec *ps_vector, /* vector to modify */
	_In_    void *p_obj,                       /* object pointer to insert */
	        size_t index                       /* position where to insert **p_obj** */
);

/*
 * Inserts **p_obj** at the end. If **p_obj** is
 * NULL, it will not be inserted.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_vec_pushback(
	_Inout_ struct marble_util_vec *ps_vector, /* vector to modify */
	_In_    void *p_obj                        /* object to insert */
);

/*
 * Inserts **p_obj** at the beginning (that is, index 0).
 * If **p_obj** is NULL, it will not be inserted.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_vec_pushfront(
	_Inout_ struct marble_util_vec *ps_vector, /* vector to modify */
	_In_    void *p_obj                        /* object to insert */
);

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
MB_API void *marble_util_vec_erase(
	_Inout_ struct marble_util_vec *ps_vector, /* vector to modify */
	        size_t index,                      /* object to insert */
	        /*
	         * This member can be used to override the freeing-behavior
	         * of this function. If this parameter is non-zero, it will
	         * not run the destructor, even if it is set and valid.
	         */
	        bool rundest
);

/*
 * Erases the last element in **ps_vector**.
 * 
 * Returns NULL if the vector is empty, there was an
 * error or it has been destroyed. If the object was
 * not destroyed, the function returns the pointer to
 * it.
 */
MB_API void *marble_util_vec_popback(
	_Inout_ struct marble_util_vec *ps_vector, /* vector to modify */
	        bool rundest                       /* run object destructor? */
);

/*
 * Erases the first element in **ps_vector**.
 * 
 * Returns NULL if the vector is empty, there was an
 * error or it has been destroyed. If the object was
 * not destroyed, the function returns the pointer to
 * it.
 */
MB_API void *marble_util_vec_popfront(
	_Inout_ struct marble_util_vec *ps_vector, /* vector to modify */
	        bool rundest                       /* run object destructor? */
);

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
MB_API size_t marble_util_vec_find(
	_In_ struct marble_util_vec *ps_vector, /* vector to modify */
	_In_ void *p_obj,                       /* object to search for */
	     size_t start,                      /* first index of searching range */
	     size_t end                         /* last index of searching range */
);

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
MB_API void *marble_util_vec_get(
	_In_ struct marble_util_vec *ps_vector, /* vector to read */
	     size_t index                       /* index */
);

/*
 * Retrieves the current size, i.e. the number of objects
 * in the given vector.
 * 
 * Returns size, in objects, or SIZE_MAX on error.
 */
MB_API size_t marble_util_vec_count(
    _In_ struct marble_util_vec *ps_vector /* vector to get size of */
);
#pragma endregion (UTIL-VECTOR)


/*
 * Structure representing an HPC (uses platform-specific
 * means of measuring time with high resolution).
 */
#pragma region UTIL-CLOCK
struct marble_util_clock {
	int64_t m_tstart; /* start time, in counts per second */
	int64_t m_tend;   /* end time, in counts per second */

    /*
     * specifies whether the clock is still running
     * or has been stopped
     */
    bool m_isrunning;
};


/*
 * Retrieves frequency of HPC.
 * 
 * Returns frequency, in counts per second.
 */
MB_API int64_t marble_util_clock_getfreq(void);

/*
 * Starts/Restarts a clock.
 * 
 * Returns nothing.
 */
MB_API void marble_util_clock_start(
	_Out_ struct marble_util_clock *ps_clock /* clock to start/reset */
);

/*
 * Stops a clock.
 * It is now safe to calculate elapsed time.
 * 
 * Returns nothing.
 */
MB_API void marble_util_clock_stop(
    _Inout_ struct marble_util_clock *ps_clock /* clock to stop */
);

/*
 * Calculates time between **m_tstart** and **m_tend**,
 * in seconds.
 * If the clock is still running, the function calculates
 * the difference between the current time and the time
 * when the clock was last started.
 * 
 * Returns elapsed time, in seconds. If there was
 * an error, the function returns -1.0.
 */
MB_API double marble_util_clock_assec(
    _Inout_ struct marble_util_clock const *ps_clock /* clock */
);

/*
 * Calculates time between **m_tstart** and **m_tend**,
 * in milliseconds.
 * If the clock is still running, the function calculates
 * the difference between the current time and the time
 * when the clock was last started.
 * 
 * Returns elapsed time, in milliseconds. If there was
 * an error, the function returns -1.0.
 */
MB_API double marble_util_clock_asmsec(
    _Inout_ struct marble_util_clock const *ps_clock /* clock */
);

/*
 * Calculates time between **m_tstart** and **m_tend**,
 * in microseconds.
 * If the clock is still running, the function calculates
 * the difference between the current time and the time
 * when the clock was last started.
 * 
 * Returns elapsed time, in microseconds. If there was
 * an error, the function returns -1.0.
 */
MB_API double marble_util_clock_asmcsec(
    _Inout_ struct marble_util_clock const *ps_clock /* clock */
);

/*
 * Calculates time between **m_tstart** and **m_tend**,
 * in nanoseconds.
 * If the clock is still running, the function calculates
 * the difference between the current time and the time
 * when the clock was last started.
 * 
 * Returns elapsed time, in nanoseconds. If there was
 * an error, the function returns -1.0.
 */
MB_API double marble_util_clock_asnsec(
    _Inout_ struct marble_util_clock const *ps_clock /* clock */
);

/*
 * Retrieves a raw platform-specific time-stamp value.
 * 
 * Returns time-stamp or 0 if the raw timer is not
 * available.
 */
MB_API uint64_t marble_util_clock_raw(void);
#pragma endregion (UTIL-CLOCK)


/*
 * Interface that handles file I/O.
 */
#pragma region UTIL-FILE
struct marble_util_file;

/*
 * A combination of these flags can be specified as
 * the **flags** parameter of "marble_util_file_open()".
 * 
 * MB_UTIL_FILE_FLAG_OVERWRITE = overwrite file if it exists
 */
#define MB_UTIL_FILE_FLAG_OVERWRITE (1 << 0)


/*
 * Opens a file. If the file does not exist, a new one
 * will be created at the specified path.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Critical_ marble_ecode_t marble_util_file_open(
	_In_z_           char const *pz_path, /* file path */
    _In_opt_         int flags,           /* optional flags */
	                 /*
	                  * pointer to receive the pointer
	                  * to the file object
	                  */
	_Init_(pps_file) struct marble_util_file **pps_file
);

/*
 * Closes and destroys a file.
 * **pps_file** will be NULL after
 * the function returns.
 * 
 * Returns nothing.
 */
MB_API void marble_util_file_destroy(
    _Uninit_(pps_file) struct marble_util_file **pps_file /* file to destroy */
);

/*
 * Sets the position of the internal file indicator.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_file_goto(
	_In_ struct marble_util_file *ps_file, /* file descriptor */
	     int64_t newpos                    /* new position */
);

/*
 * Retrieves the current internal file position
 * indicator.
 * 
 * Returns current position indicator, or -1 on
 * error.
 */
MB_API int64_t marble_util_file_tell(
    _In_ struct marble_util_file *ps_file /* file descriptor */
);

/*
 * Reads **size** bytes from **ps_file**.
 * The bytes will be stored in **p_dest**. If the
 * buffer is not large enough to hold all of the
 * read bytes, the behavior is undefined.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_file_read(
	_In_         struct marble_util_file *ps_file, /* file object */
	             size_t size,                      /* number of bytes to read */
	_Size_(size) void *p_dest                      /* buffer to write read bytes to */
);

/*
 * Reads a byte from **ps_file** and stores
 * it inside **p_dest**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_file_read8(
	_In_  struct marble_util_file *ps_file, /* file object */
	_Out_ uint8_t *p_dest                   /* destination buffer */
);

/*
 * Reads two bytes from **ps_file** and stores
 * them inside **p_dest**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_file_read16(
	_In_  struct marble_util_file *ps_file, /* file object */
	_Out_ uint16_t *p_dest				    /* destination buffer */
);

/*
 * Reads four bytes from **ps_file** and stores
 * them inside **p_dest**.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_file_read32(
	_In_  struct marble_util_file *ps_file, /* file object */
	_Out_ uint32_t *p_dest				    /* destination buffer */
);

/*
 * Retrieves the file info structure from the file
 * object.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_file_getinfo(
    _In_  struct marble_util_file *ps_file, /* file object */
    _Out_ struct _stat64 *ps_info           /* structure to receive file info */ 
);
#pragma endregion (UTIL-FILE)


/*
 * Structure representing a basic hashtable. 
 */
#pragma region UTIL-HASHTABLE
struct marble_util_htable;


/*
 * Creates a hash table and initializes its bucket array.
 * Note that the buckets themselves will not be
 * initialized until an attempt to add an element to
 * the bucket was made.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Critical_ marble_ecode_t marble_util_htable_create(
	_In_opt_           size_t nbuckets,       /* number of buckets */
	_In_opt_           marble_dtor_t fn_dtor, /* object destructor */
	/*
	 * pointer to receive the pointer
	 * to the hash table object
	 */
	_Init_(pps_htable) struct marble_util_htable **pps_htable
);

/*
 * Destroys a hash table objects, all of its buckets, and,
 * if its **pps_hashtable->mfn_dest** member is not NULL,
 * all of the objects the hash table holds using its
 * **pps_hashtable->mfn_dest()** function.
 * 
 * When the function returns, **pps_hashtable** will
 * be NULL.
 * 
 * Returns nothing.
 */
MB_API void marble_util_htable_destroy(
    _Uninit_(pps_htable) struct marble_util_htable **pps_htable /* hash table to destroy */
);

/*
 * Inserts **p_obj** into **ps_hashtable** using
 * the hash value of **pz_key**. **pz_key** should
 * be the same as the key of **p_obj** internally.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_htable_insert(
	_In_ struct marble_util_htable *ps_htable, /* hash table */
	_In_ void *p_key,                          /* **p_obj**'s key */
    _In_ size_t const keysize,                 /* key size, in bytes */
	_In_ void *p_obj                           /* object to insert */
);

/*
 * Erases an object whose key is equal to **pz_key**
 * from **ps_hashtable**. Equality is determined by the
 * return value of **fn_find**, a user-defined callback
 * function.
 * 
 * Returns NULL on error or if the element got destroyed, or
 * the element's pointer on success.
 */
MB_API void *marble_util_htable_erase(
	_In_ struct marble_util_htable *ps_htable, /* hash table */
	_In_ void *p_key,                          /* key */
    _In_ size_t const keysize,                 /* key size, in bytes */
	/*
	 * The callback function shall return non-zero if
	 * the element was found, and 'false' if not.
	 * If this parameter is NULL, the function fails
	 * and returns NULL.
	 */
	_In_ marble_find_t fn_find,
	     bool rundest                          /* destroy (found) object? */
);

/*
 * Finds an object that can be associated with
 * **pz_key**.
 * 
 * Returns the pointer to the object, or NULL
 * on error or if the object cannot be found.
 */
MB_API void *marble_util_htable_find(
	_In_ struct marble_util_htable *ps_htable, /* hash table */
	_In_ void *p_key,                          /* key to find */
    _In_ size_t const keysize,                 /* key size, in bytes */
	/*
	 * The callback function shall return non-zero if
	 * the element was found, and 'false' if not.
	 * If this parameter is NULL, the function fails
	 * and returns NULL.
	 */
	_In_ marble_find_t fn_find
);
#pragma endregion (UTIL-HASHTABLE)


/*
 * Structure representing a fixed, contiguous
 * 2-dimensional array (of pointers).
 */
#pragma region UTIL-ARRAY2D
struct marble_util_array2d;


/*
 * Creates a new, empty 2-dimensional array of pointers.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Critical_ marble_ecode_t marble_util_array2d_create(
    _In_              size_t width,                          /* initial width */
    _In_              size_t height,                         /* initial height */
    _In_opt_          marble_dtor_t fn_dtor,                 /* opt. destructor for inserted objects */
    _Init_(pps_array) struct marble_util_array2d **pps_array /* pointer to array pointer */
);

/*
 * Destroys 2D-array and frees all its resources. If the destructor
 * function passed to "marble_util_array2d_create()" was not NULL,
 * it is called on every object inside the array.
 * 
 * Returns nothing.
 */
MB_API void marble_util_array2d_destroy(
    _Uninit_(pps_array) struct marble_util_array2d **pps_array /* array to destroy */
);

/*
 * Resizes an array; due to the way the array is implemented,
 * this is a very costly operation and may change its location
 * in memory. The caller has to take care of updating all
 * references to the array.
 * If this function fails, the array will not be modified
 * and will still be valid.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Critical_ marble_ecode_t marble_util_array2d_resize(
    _Reinit_opt_(pps_array) struct marble_util_array2d **pps_array, /* array to resize */
    _In_                    size_t nwidth,                          /* new width */
    _In_                    size_t nheight                          /* new height */
);

/*
 * Inserts an object into the array. The old object will be
 * replaced. If **p_oldobj** is not NULL, it will receive the
 * pointer to the old object before it is replaced.
 * 
 * Returns 0 on success, non-zero on failure.
 */
MB_API _Success_ok_ marble_ecode_t marble_util_array2d_insert(
    _In_         struct marble_util_array2d *ps_array, /* array to modify */
                 size_t posx,                          /* x-position */
                 size_t posy,                          /* y-position */
                 bool rundest,                         /* run destructor on old object if **pp_oldobj** is NULL? */
    _In_         void *p_element,                      /* element to insert */
    _Outptr_opt_ void **pp_oldobj                      /* pointer to the old object */
);

/*
 * Erases an object from the array, optionally running the
 * destructor function if **rundest** is true.
 * 
 *         { NULL,      if **rundest** == true and there was no error
 * Returns { MB_INVPTR, if there was an error
 *         { > NULL,    if **rundest** == false and there was no error
 * 
 * Note: In the latter case (> NULL), the return value is the
 * pointer of the erased object.
 */
MB_API _Success_ptr_ void *marble_util_array2d_erase(
    _In_ struct marble_util_array2d *ps_array, /* array to erase element from */
         size_t posx,                          /* x-position */
         size_t posy,                          /* y-position */
         bool rundest                          /* run destructor on erased object? */
);

/*
 * Retrieves the object from the given array at the given
 * xy-coordinates.
 * 
 * Returns MB_INVPTR if there was an error, NULL if no
 * object is present at the given coordinates, or a
 * pointer to the object.
 */
MB_API _Success_ptr_ void *marble_util_array2d_get(
    _In_ struct marble_util_array2d *ps_array, /* array */
         size_t posx,                          /* x-position */
         size_t posy                           /* y-position */
);
#pragma endregion (UTIL-ARRAY2D)


/*
 * Initializes data used by the tools library.
 * Has to be called once when the app starts
 * in order for various utilities like the HPC
 * and the hash table to work properly.
 * 
 * Returns true on success, false on failure.
 */
MB_API bool marble_util_init(void);

/*
 * Retrieves an (integer) random number from
 * **min** to **max**, bounds inclusive.
 * 
 * Returns random number.
 */
MB_API uint64_t marble_util_rand(
    _In_range_(0, UINT64_MAX - 2) uint64_t nmin /* lower limit of random number */,
    _In_range_(0, UINT64_MAX - 1) uint64_t nmax /* upper limit of random number */
);

MB_END_HEADER


