#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ===== SHA-256 base (igual idea del ejercicio anterior) ===== */
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} sha256_ctx_t;

static const uint32_t k256[64] = {
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
    0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
    0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
    0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
    0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
    0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32U - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x) (ROTR((x), 2U) ^ ROTR((x), 13U) ^ ROTR((x), 22U))
#define BSIG1(x) (ROTR((x), 6U) ^ ROTR((x), 11U) ^ ROTR((x), 25U))
#define SSIG0(x) (ROTR((x), 7U) ^ ROTR((x), 18U) ^ ((x) >> 3U))
#define SSIG1(x) (ROTR((x), 17U) ^ ROTR((x), 19U) ^ ((x) >> 10U))

static void sha256_transform(sha256_ctx_t *ctx, const uint8_t block[64]) {
    uint32_t w[64];

    for (int i = 0; i < 16; ++i) {
        w[i] = ((uint32_t)block[i * 4] << 24U) |
               ((uint32_t)block[i * 4 + 1] << 16U) |
               ((uint32_t)block[i * 4 + 2] << 8U) |
               ((uint32_t)block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        w[i] = SSIG1(w[i - 2]) + w[i - 7] + SSIG0(w[i - 15]) + w[i - 16];
    }

    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + BSIG1(e) + CH(e, f, g) + k256[i] + w[i];
        uint32_t t2 = BSIG0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha256_init(sha256_ctx_t *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667U;
    ctx->state[1] = 0xbb67ae85U;
    ctx->state[2] = 0x3c6ef372U;
    ctx->state[3] = 0xa54ff53aU;
    ctx->state[4] = 0x510e527fU;
    ctx->state[5] = 0x9b05688cU;
    ctx->state[6] = 0x1f83d9abU;
    ctx->state[7] = 0x5be0cd19U;
}

static void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64U) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512U;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(sha256_ctx_t *ctx, uint8_t out[32]) {
    uint32_t i = ctx->datalen;

    ctx->data[i++] = 0x80U;
    if (i > 56U) {
        while (i < 64U) ctx->data[i++] = 0x00U;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56U) ctx->data[i++] = 0x00U;

    ctx->bitlen += (uint64_t)ctx->datalen * 8ULL;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8U);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16U);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24U);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32U);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40U);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48U);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56U);

    sha256_transform(ctx, ctx->data);

    for (i = 0; i < 4; ++i) {
        out[i]      = (uint8_t)((ctx->state[0] >> (24U - i * 8U)) & 0xFFU);
        out[i + 4]  = (uint8_t)((ctx->state[1] >> (24U - i * 8U)) & 0xFFU);
        out[i + 8]  = (uint8_t)((ctx->state[2] >> (24U - i * 8U)) & 0xFFU);
        out[i + 12] = (uint8_t)((ctx->state[3] >> (24U - i * 8U)) & 0xFFU);
        out[i + 16] = (uint8_t)((ctx->state[4] >> (24U - i * 8U)) & 0xFFU);
        out[i + 20] = (uint8_t)((ctx->state[5] >> (24U - i * 8U)) & 0xFFU);
        out[i + 24] = (uint8_t)((ctx->state[6] >> (24U - i * 8U)) & 0xFFU);
        out[i + 28] = (uint8_t)((ctx->state[7] >> (24U - i * 8U)) & 0xFFU);
    }
}

static void sha256_once(const uint8_t *data, size_t len, uint8_t out[32]) {
    sha256_ctx_t ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, out);
}

static void bytes_to_hex(const uint8_t *in, size_t len, char *out, size_t out_sz) {
    static const char *hex = "0123456789abcdef";
    if (out_sz < len * 2U + 1U) return;
    for (size_t i = 0; i < len; ++i) {
        out[i * 2] = hex[(in[i] >> 4U) & 0x0FU];
        out[i * 2 + 1U] = hex[in[i] & 0x0FU];
    }
    out[len * 2U] = '\0';
}

/*
 * HMAC-SHA256 estándar:
 * H(K XOR opad, H(K XOR ipad, msg))
 */
static void hmac_sha256(const uint8_t *key, size_t key_len,
                        const uint8_t *msg, size_t msg_len,
                        uint8_t out[32]) {
    uint8_t k0[64];
    memset(k0, 0, sizeof(k0));

    if (key_len > 64U) {
        sha256_once(key, key_len, k0);
    } else {
        memcpy(k0, key, key_len);
    }

    uint8_t ipad[64];
    uint8_t opad[64];
    for (size_t i = 0; i < 64U; ++i) {
        ipad[i] = (uint8_t)(k0[i] ^ 0x36U);
        opad[i] = (uint8_t)(k0[i] ^ 0x5cU);
    }

    uint8_t inner_digest[32];
    sha256_ctx_t ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, ipad, sizeof(ipad));
    sha256_update(&ctx, msg, msg_len);
    sha256_final(&ctx, inner_digest);

    sha256_init(&ctx);
    sha256_update(&ctx, opad, sizeof(opad));
    sha256_update(&ctx, inner_digest, sizeof(inner_digest));
    sha256_final(&ctx, out);
}

int main(void) {
    const uint8_t *key = (const uint8_t *)"key";
    const uint8_t *msg = (const uint8_t *)"abc";
    const uint8_t *msg_tampered = (const uint8_t *)"abC";

    uint8_t mac1[32];
    uint8_t mac2[32];
    char hex1[65];
    char hex2[65];

    hmac_sha256(key, strlen((const char *)key), msg, strlen((const char *)msg), mac1);
    hmac_sha256(key, strlen((const char *)key), msg_tampered, strlen((const char *)msg_tampered), mac2);

    bytes_to_hex(mac1, sizeof(mac1), hex1, sizeof(hex1));
    bytes_to_hex(mac2, sizeof(mac2), hex2, sizeof(hex2));

    int changed = (strcmp(hex1, hex2) != 0) ? 1 : 0;

    printf("mac=%s tampered_mac=%s changed=%d\n", hex1, hex2, changed);

    const char *expected = "9c196e32dc0175f86f4b1cb89289d661"
                           "9de6bee699e4c378e68309ed97a1a6ab";

    return (strcmp(hex1, expected) == 0 && changed == 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}
