#include "dupliguard/sha256.h"

#include <string.h>

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32U - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR((x), 2) ^ ROTR((x), 13) ^ ROTR((x), 22))
#define EP1(x) (ROTR((x), 6) ^ ROTR((x), 11) ^ ROTR((x), 25))
#define SIG0(x) (ROTR((x), 7) ^ ROTR((x), 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR((x), 17) ^ ROTR((x), 19) ^ ((x) >> 10))

static const uint32_t constants[64] = {
    0x428a2f98U,0x71374491U,0xb5c0fbcfU,0xe9b5dba5U,0x3956c25bU,0x59f111f1U,0x923f82a4U,0xab1c5ed5U,
    0xd807aa98U,0x12835b01U,0x243185beU,0x550c7dc3U,0x72be5d74U,0x80deb1feU,0x9bdc06a7U,0xc19bf174U,
    0xe49b69c1U,0xefbe4786U,0x0fc19dc6U,0x240ca1ccU,0x2de92c6fU,0x4a7484aaU,0x5cb0a9dcU,0x76f988daU,
    0x983e5152U,0xa831c66dU,0xb00327c8U,0xbf597fc7U,0xc6e00bf3U,0xd5a79147U,0x06ca6351U,0x14292967U,
    0x27b70a85U,0x2e1b2138U,0x4d2c6dfcU,0x53380d13U,0x650a7354U,0x766a0abbU,0x81c2c92eU,0x92722c85U,
    0xa2bfe8a1U,0xa81a664bU,0xc24b8b70U,0xc76c51a3U,0xd192e819U,0xd6990624U,0xf40e3585U,0x106aa070U,
    0x19a4c116U,0x1e376c08U,0x2748774cU,0x34b0bcb5U,0x391c0cb3U,0x4ed8aa4aU,0x5b9cca4fU,0x682e6ff3U,
    0x748f82eeU,0x78a5636fU,0x84c87814U,0x8cc70208U,0x90befffaU,0xa4506cebU,0xbef9a3f7U,0xc67178f2U
};

static void transform(dg_sha256_context *ctx, const uint8_t data[64]) {
    uint32_t m[64], a, b, c, d, e, f, g, h;
    for (size_t i = 0; i < 16; ++i) {
        size_t j = i * 4;
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j + 1] << 16) |
               ((uint32_t)data[j + 2] << 8) | (uint32_t)data[j + 3];
    }
    for (size_t i = 16; i < 64; ++i) m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];
    for (size_t i = 0; i < 64; ++i) {
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + constants[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

void dg_sha256_init(dg_sha256_context *ctx) {
    ctx->data_length = 0; ctx->bit_length = 0;
    ctx->state[0]=0x6a09e667U; ctx->state[1]=0xbb67ae85U; ctx->state[2]=0x3c6ef372U; ctx->state[3]=0xa54ff53aU;
    ctx->state[4]=0x510e527fU; ctx->state[5]=0x9b05688cU; ctx->state[6]=0x1f83d9abU; ctx->state[7]=0x5be0cd19U;
}

void dg_sha256_update(dg_sha256_context *ctx, const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        ctx->data[ctx->data_length++] = data[i];
        if (ctx->data_length == 64) {
            transform(ctx, ctx->data); ctx->bit_length += 512; ctx->data_length = 0;
        }
    }
}

void dg_sha256_final(dg_sha256_context *ctx, uint8_t digest[DG_SHA256_DIGEST_SIZE]) {
    size_t i = ctx->data_length;
    ctx->data[i++] = 0x80;
    if (i > 56) { while (i < 64) ctx->data[i++] = 0; transform(ctx, ctx->data); i = 0; }
    while (i < 56) ctx->data[i++] = 0;
    ctx->bit_length += (uint64_t)ctx->data_length * 8;
    for (size_t j = 0; j < 8; ++j) ctx->data[63-j] = (uint8_t)(ctx->bit_length >> (j*8));
    transform(ctx, ctx->data);
    for (i = 0; i < 32; ++i) digest[i] = (uint8_t)(ctx->state[i/4] >> (24 - (i%4)*8));
    memset(ctx, 0, sizeof(*ctx));
}

void dg_sha256_hex(const uint8_t digest[DG_SHA256_DIGEST_SIZE], char output[DG_SHA256_HEX_SIZE]) {
    static const char digits[] = "0123456789abcdef";
    for (size_t i = 0; i < DG_SHA256_DIGEST_SIZE; ++i) {
        output[i*2] = digits[digest[i] >> 4]; output[i*2+1] = digits[digest[i] & 0x0f];
    }
    output[64] = '\0';
}
