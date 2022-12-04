#include <application.h>

#pragma intrinsic(__rdtsc)


void marble_uuid_generate(
    _Out_ marble_uuid_t *p_uuid
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

void marble_uuid_tostr(
    _Out_ char *pz_dest,
    _In_  marble_uuid_t *p_uuid
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

void marble_uuid_copy(
    _Out_ marble_uuid_t *pu_dst,
    _In_  marble_uuid_t const *pu_src 
) {
    if (pu_dst == NULL || pu_src == NULL)
        return;

    marble_system_cpymem(pu_dst, pu_src, sizeof *pu_dst);
}


