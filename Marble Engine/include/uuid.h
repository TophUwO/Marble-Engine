#pragma once

#include <def.h>
#include <platform.h>

#include <stdint.h>
#include <memory.h>

#pragma intrinsic(__rdtsc)


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
inline void marble_uuid_generate(
    _Out_ marble_uuid_t *p_uuid /* destination buffer */
) {
    if (p_uuid == NULL)
        return;

    /*
     * Generate two new pseudo-random numbers
     * from this seed.
     */
    for (int i = 0; i < 2; i++) {
        uint64_t ts, x, seed;

        /*
         * Get the current number of clocks since
         * the last reset.
         */
        ts = __rdtsc();

        /*
         * Use the xorshift64* generator to create
         * a bit of pseudo-randomness.
         */
        x    = ts;
        x   ^= x >> 12;
        x   ^= x << 25;
        x   ^= x >> 27;
        seed = x * 0x2545F4914F6CDD1DULL;

        p_uuid->m_i64[i] = seed;
    }

    /*
     * Set the required bits to designate the
     * UUID as a v4 UUID.
     * 
     * For more information:
     *     RFC4122: https://www.rfc-editor.org/rfc/rfc4122#section-4.4.
     */
    p_uuid->m_i8[6] = 0x40 | p_uuid->m_i8[6] & 0x0F;
    p_uuid->m_i8[8] = 0x80 | p_uuid->m_i8[8] & 0x3F;
}

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
inline void marble_uuid_tostr(
    _Out_ char *pz_dest,        /* destination buffer */
    _In_  marble_uuid_t *p_uuid /* source UUID */
) {
    static char const *const glpz_hexdigits = "0123456789abcdef"; 

    if (p_uuid == NULL || pz_dest == NULL) {
        if (pz_dest != NULL)
            memset(pz_dest, 0, 37);

        return;
    }

    for (int i = 0; i < 16; i++) {
        /* Insert hyphen in between groups. */
        if (i >= 4 && i <= 10 && i % 2 == 0)
            *pz_dest++ = '-';

        /* Convert the next byte. */
        *pz_dest++ = glpz_hexdigits[p_uuid->m_i8[i] >> 4];
        *pz_dest++ = glpz_hexdigits[p_uuid->m_i8[i] & 0x0F];
    }

    /* Append NUL-terminator. */
    pz_dest[36] = '\0';
}

/*
 * Copies the UUID in **p_src** to **p_dst**.
 * 
 * Returns nothing.
 */
inline void marble_uuid_copy(
    _Out_ marble_uuid_t *p_dst,      /* destination buffer */
    _In_  marble_uuid_t const *p_src /* source buffer */
) {
    if (p_dst == NULL || p_src == NULL)
        return;

    memcpy_s(p_dst, sizeof *p_dst, p_src, sizeof *p_src);
}


MB_END_HEADER


