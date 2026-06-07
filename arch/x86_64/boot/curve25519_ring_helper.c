#include <stdint.h>
#include <stddef.h>

typedef int64_t RuntimeValue;

#define TAG_MASK    0x7ULL
#define TAG_INT     0x0ULL
#define TAG_HEAP    0x1ULL
#define TAG_SPECIAL 0x3ULL

#define ENCODE_INT(v)  ((RuntimeValue)(((uint64_t)(int64_t)(v) << 3) | TAG_INT))
#define ENCODE_PTR(p)  ((RuntimeValue)((uint64_t)(uintptr_t)(p) | TAG_HEAP))
#define DECODE_PTR(v)  ((void*)((uint64_t)(v) & ~TAG_MASK))
#define DECODE_INT(v)  ((int64_t)((uint64_t)(v) >> 3))
#define IS_INT(v)      (((uint64_t)(v) & TAG_MASK) == TAG_INT)
#define IS_HEAP(v)     (((uint64_t)(v) & TAG_MASK) == TAG_HEAP)
#define NIL_VALUE      ((RuntimeValue)TAG_SPECIAL)

typedef struct {
    uint32_t type;
    uint32_t size;
} HeapHeader;

typedef struct {
    HeapHeader   hdr;
    uint64_t     len;
    uint64_t     cap;
    RuntimeValue *items;
} RuntimeArray;

#define HEAP_ARRAY 2

extern void *malloc(size_t size);
extern void free(void *ptr);

static inline RuntimeValue *runtime_array_inline_items(RuntimeArray *a)
{
    return (RuntimeValue *)((uint8_t *)a + sizeof(RuntimeArray));
}

static inline RuntimeValue *runtime_array_items(RuntimeArray *a)
{
    if (!a) return NULL;
    return a->items ? a->items : runtime_array_inline_items(a);
}

static inline uint8_t _rv_byte(RuntimeValue v)
{
    int64_t byte_val = IS_INT(v) ? DECODE_INT(v) : (int64_t)v;
    return (uint8_t)(byte_val & 0xFF);
}

int ring_core_0_17_14__CRYPTO_memcmp(const void *a, const void *b, size_t n)
{
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    uint8_t diff = 0;
    for (size_t i = 0; i < n; i++) diff |= (uint8_t)(pa[i] ^ pb[i]);
    return diff;
}

#include "../../../../../src/compiler_rust/vendor/ring/crypto/curve25519/curve25519.c"

static const uint64_t _tls_sha512_K[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

static const uint64_t _tls_sha512_H[8] = {
    0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
    0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

static inline uint64_t _tls_sha512_rotr(uint64_t x, int n) { return (x >> n) | (x << (64 - n)); }
static inline uint64_t _tls_sha512_ch(uint64_t x, uint64_t y, uint64_t z) { return (x & y) ^ (~x & z); }
static inline uint64_t _tls_sha512_maj(uint64_t x, uint64_t y, uint64_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint64_t _tls_sha512_S0(uint64_t x) { return _tls_sha512_rotr(x, 28) ^ _tls_sha512_rotr(x, 34) ^ _tls_sha512_rotr(x, 39); }
static inline uint64_t _tls_sha512_S1(uint64_t x) { return _tls_sha512_rotr(x, 14) ^ _tls_sha512_rotr(x, 18) ^ _tls_sha512_rotr(x, 41); }
static inline uint64_t _tls_sha512_s0(uint64_t x) { return _tls_sha512_rotr(x, 1) ^ _tls_sha512_rotr(x, 8) ^ (x >> 7); }
static inline uint64_t _tls_sha512_s1(uint64_t x) { return _tls_sha512_rotr(x, 19) ^ _tls_sha512_rotr(x, 61) ^ (x >> 6); }

static void _tls_sha512_process_block(const uint8_t *block, uint64_t h[8])
{
    uint64_t w[80];
    for (int i = 0; i < 16; i++) {
        w[i] = ((uint64_t)block[i * 8 + 0] << 56) |
               ((uint64_t)block[i * 8 + 1] << 48) |
               ((uint64_t)block[i * 8 + 2] << 40) |
               ((uint64_t)block[i * 8 + 3] << 32) |
               ((uint64_t)block[i * 8 + 4] << 24) |
               ((uint64_t)block[i * 8 + 5] << 16) |
               ((uint64_t)block[i * 8 + 6] << 8) |
               ((uint64_t)block[i * 8 + 7]);
    }
    for (int t = 16; t < 80; t++) w[t] = _tls_sha512_s1(w[t - 2]) + w[t - 7] + _tls_sha512_s0(w[t - 15]) + w[t - 16];

    uint64_t a = h[0], b = h[1], c = h[2], d = h[3];
    uint64_t e = h[4], f = h[5], g = h[6], hh = h[7];
    for (int t = 0; t < 80; t++) {
        uint64_t t1 = hh + _tls_sha512_S1(e) + _tls_sha512_ch(e, f, g) + _tls_sha512_K[t] + w[t];
        uint64_t t2 = _tls_sha512_S0(a) + _tls_sha512_maj(a, b, c);
        hh = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
}

static void _tls_sha512_hash(const uint8_t *msg, uint32_t msg_len, uint8_t out[64])
{
    uint64_t bit_len_lo = ((uint64_t)msg_len) * 8ULL;
    uint32_t pad_len = 1 + 16;
    while (((msg_len + pad_len) % 128) != 0) pad_len++;
    uint32_t total_len = msg_len + pad_len;
    uint8_t *padded = (uint8_t *)malloc(total_len ? total_len : 1);
    if (!padded) {
        for (int i = 0; i < 64; i++) out[i] = 0;
        return;
    }
    for (uint32_t i = 0; i < msg_len; i++) padded[i] = msg[i];
    padded[msg_len] = 0x80;
    for (uint32_t i = msg_len + 1; i < total_len - 16; i++) padded[i] = 0;
    for (int i = 0; i < 8; i++) padded[total_len - 16 + i] = 0;
    for (int i = 0; i < 8; i++) padded[total_len - 8 + i] = (uint8_t)(bit_len_lo >> (56 - i * 8));

    uint64_t h[8];
    for (int i = 0; i < 8; i++) h[i] = _tls_sha512_H[i];
    for (uint32_t off = 0; off < total_len; off += 128) _tls_sha512_process_block(padded + off, h);
    free(padded);
    for (int i = 0; i < 8; i++) {
        for (int b = 0; b < 8; b++) out[i * 8 + b] = (uint8_t)(h[i] >> (56 - b * 8));
    }
}

int64_t rt_tls13_ring_ed25519_verify_raw(const uint8_t *msg, uint32_t msg_len,
                                         const uint8_t pk[32], const uint8_t sig[64])
{
    if (!pk || !sig) return -1;

    ge_p3 A;
    if (x25519_ge_frombytes_vartime(&A, pk) == 0) return -1;
    ge_p3 negA = A;
    fe_loose neg_x;
    fe_loose neg_t;
    fe_neg(&neg_x, &A.X);
    fe_neg(&neg_t, &A.T);
    fe_carry(&negA.X, &neg_x);
    fe_carry(&negA.T, &neg_t);

    uint32_t total = 32u + 32u + msg_len;
    uint8_t *hram_input = (uint8_t *)malloc(total ? total : 1);
    if (!hram_input) return -1;
    for (uint32_t i = 0; i < 32; i++) hram_input[i] = sig[i];
    for (uint32_t i = 0; i < 32; i++) hram_input[32 + i] = pk[i];
    for (uint32_t i = 0; i < msg_len; i++) hram_input[64 + i] = msg[i];

    uint8_t hram[64];
    _tls_sha512_hash(hram_input, total, hram);
    free(hram_input);
    x25519_sc_reduce(hram);

    ge_p2 Rcheck;
    x25519_ge_double_scalarmult_vartime(&Rcheck, hram, &negA, sig + 32);

    fe recip;
    fe x;
    fe y;
    fe_invert(&recip, &Rcheck.Z);
    fe_mul_ttt(&x, &Rcheck.X, &recip);
    fe_mul_ttt(&y, &Rcheck.Y, &recip);

    uint8_t check[32];
    fe_tobytes(check, &y);
    check[31] ^= (uint8_t)(fe_isnegative(&x) << 7);
    return ring_core_0_17_14__CRYPTO_memcmp(check, sig, 32) == 0 ? 0 : -1;
}

int64_t rt_tls13_ring_ed25519_keypair_raw(const uint8_t seed[32], uint8_t pk[32], uint8_t sk[64])
{
    if (!seed || !pk || !sk) return -1;

    uint8_t h[64];
    _tls_sha512_hash(seed, 32, h);
    x25519_sc_mask(h);

    ge_p3 A;
    x25519_ge_scalarmult_base(&A, h, 0);
    fe recip;
    fe x;
    fe y;
    fe_invert(&recip, &A.Z);
    fe_mul_ttt(&x, &A.X, &recip);
    fe_mul_ttt(&y, &A.Y, &recip);
    fe_tobytes(pk, &y);
    pk[31] ^= (uint8_t)(fe_isnegative(&x) << 7);

    for (uint32_t i = 0; i < 32; i++) {
        sk[i] = seed[i];
        sk[32 + i] = pk[i];
    }
    return 0;
}

int64_t rt_tls13_ring_ed25519_sign_raw(const uint8_t *msg, uint32_t msg_len,
                                       const uint8_t sk[64], uint8_t sig[64])
{
    if (!sk || !sig) return -1;

    uint8_t h[64];
    _tls_sha512_hash(sk, 32, h);
    uint8_t a_scalar[32];
    for (uint32_t i = 0; i < 32; i++) a_scalar[i] = h[i];
    x25519_sc_mask(a_scalar);

    uint32_t nonce_input_len = 32u + msg_len;
    uint8_t *nonce_input = (uint8_t *)malloc(nonce_input_len ? nonce_input_len : 1);
    if (!nonce_input) return -1;
    for (uint32_t i = 0; i < 32; i++) nonce_input[i] = h[32 + i];
    for (uint32_t i = 0; i < msg_len; i++) nonce_input[32 + i] = msg ? msg[i] : 0;
    uint8_t nonce[64];
    _tls_sha512_hash(nonce_input, nonce_input_len, nonce);
    free(nonce_input);
    x25519_sc_reduce(nonce);

    ge_p3 R;
    x25519_ge_scalarmult_base(&R, nonce, 0);
    fe recip;
    fe x;
    fe y;
    fe_invert(&recip, &R.Z);
    fe_mul_ttt(&x, &R.X, &recip);
    fe_mul_ttt(&y, &R.Y, &recip);
    fe_tobytes(sig, &y);
    sig[31] ^= (uint8_t)(fe_isnegative(&x) << 7);

    uint32_t hram_input_len = 64u + msg_len;
    uint8_t *hram_input = (uint8_t *)malloc(hram_input_len ? hram_input_len : 1);
    if (!hram_input) return -1;
    for (uint32_t i = 0; i < 32; i++) hram_input[i] = sig[i];
    for (uint32_t i = 0; i < 32; i++) hram_input[32 + i] = sk[32 + i];
    for (uint32_t i = 0; i < msg_len; i++) hram_input[64 + i] = msg ? msg[i] : 0;
    uint8_t hram[64];
    _tls_sha512_hash(hram_input, hram_input_len, hram);
    free(hram_input);
    x25519_sc_reduce(hram);

    x25519_sc_muladd(sig + 32, hram, a_scalar, nonce);
    return 0;
}

RuntimeValue rt_tls13_x25519_shared_secret(RuntimeValue scalar_rv, RuntimeValue point_rv)
{
    if (!IS_HEAP(scalar_rv) || !IS_HEAP(point_rv)) return NIL_VALUE;

    RuntimeArray *scalar = (RuntimeArray *)DECODE_PTR(scalar_rv);
    RuntimeArray *point = (RuntimeArray *)DECODE_PTR(point_rv);
    if (!scalar || !point || scalar->hdr.type != HEAP_ARRAY || point->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    if (scalar->len != 32 || point->len != 32) return NIL_VALUE;

    RuntimeValue *scalar_items = runtime_array_items(scalar);
    RuntimeValue *point_items = runtime_array_items(point);
    uint8_t scalar_raw[32];
    uint8_t point_raw[32];
    uint8_t out_raw[32];
    for (uint32_t i = 0; i < 32; i++) {
        scalar_raw[i] = _rv_byte(scalar_items[i]);
        point_raw[i] = _rv_byte(point_items[i]);
    }

    /* ring's generic X25519 entrypoint expects a masked scalar. The Simple
     * caller provides the raw 32-byte private key, so clamp it here to match
     * RFC 7748 and the Simple implementation. */
    scalar_raw[0] &= 248u;
    scalar_raw[31] &= 127u;
    scalar_raw[31] |= 64u;

    /* RFC 7748 ignores the top bit of the peer u-coordinate. Mask it here so
     * the fd-mode helper matches the Simple path exactly. */
    point_raw[31] &= 127u;

    x25519_scalar_mult_generic_masked(out_raw, scalar_raw, point_raw);

    RuntimeArray *out = (RuntimeArray *)malloc(sizeof(RuntimeArray) + 32u * sizeof(RuntimeValue));
    if (!out) return NIL_VALUE;
    out->hdr.type = HEAP_ARRAY;
    out->hdr.size = (uint32_t)(sizeof(RuntimeArray) + 32u * sizeof(RuntimeValue));
    out->len = 32;
    out->cap = 32;
    out->items = runtime_array_inline_items(out);
    for (uint32_t i = 0; i < 32; i++) out->items[i] = ENCODE_INT(out_raw[i]);
    return ENCODE_PTR(out);
}

RuntimeValue rt_tls13_x25519_public_key(RuntimeValue scalar_rv)
{
    if (!IS_HEAP(scalar_rv)) return NIL_VALUE;

    RuntimeArray *scalar = (RuntimeArray *)DECODE_PTR(scalar_rv);
    if (!scalar || scalar->hdr.type != HEAP_ARRAY) return NIL_VALUE;
    if (scalar->len != 32) return NIL_VALUE;

    RuntimeValue *scalar_items = runtime_array_items(scalar);
    uint8_t scalar_raw[32];
    uint8_t point_raw[32] = {9};
    uint8_t out_raw[32];
    for (uint32_t i = 0; i < 32; i++) scalar_raw[i] = _rv_byte(scalar_items[i]);

    scalar_raw[0] &= 248u;
    scalar_raw[31] &= 127u;
    scalar_raw[31] |= 64u;

    x25519_scalar_mult_generic_masked(out_raw, scalar_raw, point_raw);

    RuntimeArray *out = (RuntimeArray *)malloc(sizeof(RuntimeArray) + 32u * sizeof(RuntimeValue));
    if (!out) return NIL_VALUE;
    out->hdr.type = HEAP_ARRAY;
    out->hdr.size = (uint32_t)(sizeof(RuntimeArray) + 32u * sizeof(RuntimeValue));
    out->len = 32;
    out->cap = 32;
    out->items = runtime_array_inline_items(out);
    for (uint32_t i = 0; i < 32; i++) out->items[i] = ENCODE_INT(out_raw[i]);
    return ENCODE_PTR(out);
}

int64_t rt_tls13_ring_x25519_shared_secret_into_raw(const uint8_t scalar[32],
                                                    const uint8_t point[32],
                                                    uint8_t out[32])
{
    if (!scalar || !point || !out) return -1;

    uint8_t scalar_raw[32];
    uint8_t point_raw[32];
    for (uint32_t i = 0; i < 32; i++) {
        scalar_raw[i] = scalar[i];
        point_raw[i] = point[i];
    }

    scalar_raw[0] &= 248u;
    scalar_raw[31] &= 127u;
    scalar_raw[31] |= 64u;
    point_raw[31] &= 127u;

    x25519_scalar_mult_generic_masked(out, scalar_raw, point_raw);
    return 0;
}
