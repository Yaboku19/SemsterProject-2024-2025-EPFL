#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "blst.h"
#include <string.h>
#include <stdbool.h>
#include "header/struct.h"
#include "header/generation.h"

#define DST "BLS_SIG_DST" 

void generateKeys (blst_scalar *sk, uint8_t *pk_bytes, blst_p1_affine *pk_affine) {
    uint8_t ikm[32];
    for (int i = 0; i < 32; i++) {
        ikm[i] = rand() % 256;
    }

    blst_keygen(sk, ikm, 32, NULL, 0);
    blst_sk_to_pk2_in_g1(pk_bytes, pk_affine, sk);

    if (!blst_p1_affine_in_g1(pk_affine)) {
        printf("Public key failed group check ❌\n");
        return;
    }
    pk_bytes[0] &= ~0x20;
}

void signMessage (blst_scalar sk, blst_p2_affine *sig_affine, uint8_t *sig_bytes, uint8_t *message) {

    blst_p2 msg_point;
    blst_hash_to_g2(&msg_point, message, sizeof(message), (uint8_t *)DST, strlen(DST), NULL, 0);

    blst_p2 signature;
    blst_sign_pk_in_g1(&signature, &msg_point, &sk);

    blst_p2_serialize(sig_bytes, &signature);

    blst_p2_to_affine(sig_affine, &signature);

    if (!blst_p2_affine_in_g2(sig_affine)) {
        printf("Signature failed group check ❌\n");
        return;
    }
}

int verifyMessage (blst_p1_affine pk_affine, blst_p2_affine sig_affine, uint8_t *message) {

    BLST_ERROR verify_result = blst_core_verify_pk_in_g1(
        &pk_affine, &sig_affine, true, message, sizeof(message),
        (uint8_t *)DST, strlen(DST), NULL, 0
    );

    if (verify_result != BLST_SUCCESS) {
        printf("blst_core_verify_pk_in_g1 failed ❌\n");
        return 0;
    }

    blst_pairing *pairing = malloc(blst_pairing_sizeof());
    if (!pairing) {
        printf("Failed to allocate pairing structure ❌\n");
        return 0;
    }

    blst_pairing_init(pairing, true, (uint8_t *)DST, strlen(DST));
    blst_pairing_aggregate_pk_in_g1(pairing, &pk_affine, &sig_affine, message, sizeof(message), NULL, 0);
    blst_pairing_commit(pairing);
    int valid = blst_pairing_finalverify(pairing, NULL);
    free(pairing);
    return valid;
}

int main() {
    srand(time(NULL));
    blst_scalar a_sk, b_sk;
    uint8_t a_pk_bytes[96], b_pk_bytes[96];
    blst_p1_affine a_pk_affine, b_pk_affine;
    blst_p2_affine a_sig_affine, b_sig_affine;
    uint8_t a_sig_bytes[192], b_sig_bytes[192];
    uint384_t a, b;
    uint8_t a_bytes[48], b_bytes[48];
    memcpy(a_bytes, a.chunk, 48);
    memcpy(b_bytes, b.chunk, 48);
    int a_valid, b_valid;
    generate_large_number384(&a, 1);
    generate_large_number384(&b, 1);

    generateKeys(&a_sk, a_pk_bytes, &a_pk_affine);
    generateKeys(&b_sk, b_pk_bytes, &b_pk_affine);
    signMessage(a_sk, &a_sig_affine, a_sig_bytes, a_bytes);
    signMessage(b_sk, &b_sig_affine, b_sig_bytes, b_bytes);
    a_valid = verifyMessage(a_pk_affine, a_sig_affine, a_bytes);
    b_valid = verifyMessage(b_pk_affine, b_sig_affine, b_bytes);

    uint8_t agg_msg[96]; // 48 bytes per ciascun messaggio
    memcpy(agg_msg, a_bytes, 48);
    memcpy(agg_msg + 48, b_bytes, 48);

    // Aggregazione delle firme
    blst_p2 agr_sig;
    blst_p2_add_or_double(&agr_sig, (blst_p2 *)&a_sig_affine, (blst_p2 *)&b_sig_affine);
    blst_p2_affine agr_sig_affine;
    blst_p2_to_affine(&agr_sig_affine, &agr_sig);
    
    // Aggregazione delle chiavi pubbliche
    blst_p1 agr_pk;
    blst_p1_add_or_double(&agr_pk, (blst_p1 *)&a_pk_affine, (blst_p1 *)&b_pk_affine);
    blst_p1_affine agr_pk_affine;
    blst_p1_to_affine(&agr_pk_affine, &agr_pk);

    // Verifica della firma aggregata
    int agr_valid = verifyMessage(agr_pk_affine, agr_sig_affine, agg_msg);


    // Output Results
    printf("a: \t\t0x%016lx", a.chunk[5]);
    for(int i = 4; i > -1; i--) {
        printf("_%016lx", a.chunk[i]);
    }
    printf("\n\n");
    printf("b: \t\t0x%016lx", b.chunk[5]);
    for(int i = 4; i > -1; i--) {
        printf("_%016lx", b.chunk[i]);
    }
    printf("\n\n");

    printf("Private Key a(hex): \t");
    for (int i = 0; i < sizeof(a_sk.b); i++) {
        printf("%02x", a_sk.b[i]);
    }
    printf("\n\n");
    printf("Private Key b(hex): \t");
    for (int i = 0; i < sizeof(a_sk.b); i++) {
        printf("%02x", b_sk.b[i]);
    }
    printf("\n\n");

    printf("Public Key a(hex): \t");
    for (int i = 0; i < 96; i++) {
        printf("%02x", a_pk_bytes[i]);
    }
    printf("\n\n");
    printf("Public Key b(hex): \t");
    for (int i = 0; i < 96; i++) {
        printf("%02x", b_pk_bytes[i]);
    }
    printf("\n\n");
    printf("Aggr Public Key (hex): \t");
    uint8_t agg_pk_bytes[96];
    blst_p1_serialize(agg_pk_bytes, &agr_pk);
    for (int i = 0; i < 96; i++) {
        printf("%02x", agg_pk_bytes[i]);
    }
    printf("\n\n");
    printf("Signature a(hex): \t");
    for (int i = 0; i < 192; i++) {
        printf("%02x", a_sig_bytes[i]);
    }
    printf("\n\n");
    printf("Signature b(hex): \t");
    for (int i = 0; i < 192; i++) {
        printf("%02x", b_sig_bytes[i]);
    }
    printf("\n\n");
    printf("Aggr Signature (hex): \t");
    uint8_t agg_sig_bytes[192];
    blst_p2_serialize(agg_sig_bytes, &agr_sig);
    for (int i = 0; i < 192; i++) {
        printf("%02x", agg_sig_bytes[i]);
    }
    printf("\n\n");

    if (a_valid) {
        printf("Signature a is VALID ✅\n");
    } else {
        printf("Signature a is INVALID ❌\n");
    }
    if (b_valid) {
        printf("Signature b is VALID ✅\n");
    } else {
        printf("Signature b is INVALID ❌\n");
    }
    if (agr_valid) {
        printf("Aggr Signature is VALID ✅\n");
    } else {
        printf("Aggr Signature is INVALID ❌\n");
    }
    return 0;
}