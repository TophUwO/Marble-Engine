#pragma once

#include <api.h>


MB_BEGIN_HEADER


typedef union {
    uint64_t m_i64[2];
    uint8_t  m_i8[16];
} marble_uuid_t;


/*
 * Generates a new v4 UUID ((pseudo-)randomly generated) and
 * stores the result in **p_uuid**. Use "marble_uuid_tostr()"
 * to convert the result to a string.
 * 
 * Returns nothing.
 */
MB_API void marble_uuid_generate(
    _Out_ marble_uuid_t *p_uuid /* destination buffer */
);

/*
 * Converts a given UUID to its canonical string representation.
 * The result is written to **pz_dest**. This buffer should be
 * at least 37 bytes in size. The NUL-terminator is automatically
 * appended.
 * If **p_uuid** is NULL, the function zeroes **pz_buffer** and
 * returns.
 *
 * Returns nothing.
 */
MB_API void marble_uuid_tostr(
    _Out_ char *pz_dest,        /* destination buffer */
    _In_  marble_uuid_t *p_uuid /* source UUID */
);

/*
 * Copies the UUID in **p_src** to **p_dst**.
 * 
 * Returns nothing.
 */
MB_API void marble_uuid_copy(
    _Out_ marble_uuid_t *pu_dst,      /* destination buffer */
    _In_  marble_uuid_t const *pu_src /* source buffer */
);


MB_END_HEADER


