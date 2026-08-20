#ifndef DUPLIGUARD_SHA256_H
#define DUPLIGUARD_SHA256_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DG_SHA256_DIGEST_SIZE 32
#define DG_SHA256_HEX_SIZE 65

typedef struct {
    uint8_t data[64];
    uint32_t state[8];
    uint64_t bit_length;
    size_t data_length;
} dg_sha256_context;

void dg_sha256_init(dg_sha256_context *context);
void dg_sha256_update(dg_sha256_context *context, const uint8_t *data, size_t length);
void dg_sha256_final(dg_sha256_context *context, uint8_t digest[DG_SHA256_DIGEST_SIZE]);
void dg_sha256_hex(const uint8_t digest[DG_SHA256_DIGEST_SIZE], char output[DG_SHA256_HEX_SIZE]);

#ifdef __cplusplus
}
#endif

#endif
