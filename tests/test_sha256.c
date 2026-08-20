#include "dupliguard/sha256.h"
#include <stdio.h>
#include <string.h>

static int check(const char *input, const char *expected) {
    dg_sha256_context ctx; uint8_t digest[32]; char hex[65];
    dg_sha256_init(&ctx); dg_sha256_update(&ctx,(const uint8_t*)input,strlen(input)); dg_sha256_final(&ctx,digest); dg_sha256_hex(digest,hex);
    if(strcmp(hex,expected)!=0){fprintf(stderr,"SHA-256 mismatch for '%s': %s\n",input,hex);return 1;} return 0;
}
int main(void){
    int failed=0;
    failed|=check("","e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    failed|=check("abc","ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    failed|=check("The quick brown fox jumps over the lazy dog","d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
    if(!failed) {
        puts("SHA-256 vectors: OK");
    }
    return failed;
}
