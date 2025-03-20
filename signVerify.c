#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "blst.h"
#include <string.h>
#include <stdbool.h>

#define MESSAGE "He, BLS!"  // Message to be signed
#define DST "BLS_SIG_DST" 

void generateKeys (blst_scalar *sk, uint8_t *pk_bytes, blst_p1_affine *pk_affine) {
    uint8_t ikm[32];
    srand(time(NULL));
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

void signMessage (blst_scalar sk, blst_p2_affine *sig_affine, uint8_t *sig_bytes) {
    // Hash the message to G2
    blst_p2 msg_point;
    blst_hash_to_g2(&msg_point, (uint8_t *)MESSAGE, strlen(MESSAGE), (uint8_t *)DST, strlen(DST), NULL, 0);

    // Sign the hashed message
    blst_p2 signature;
    blst_sign_pk_in_g1(&signature, &msg_point, &sk);

    // Serialize signature
    blst_p2_serialize(sig_bytes, &signature);

    // Deserialize signature for verification
    blst_p2_to_affine(sig_affine, &signature);

    // Verify that signature is in correct group
    if (!blst_p2_affine_in_g2(sig_affine)) {
        printf("Signature failed group check ❌\n");
        return;
    }
}

int verifyMessage (blst_p1_affine pk_affine, blst_p2_affine sig_affine) {
    // First, try direct verification with blst_core_verify_pk_in_g1
    BLST_ERROR verify_result = blst_core_verify_pk_in_g1(
        &pk_affine, &sig_affine, true, (uint8_t *)MESSAGE, strlen(MESSAGE),
        (uint8_t *)DST, strlen(DST), NULL, 0
    );

    if (verify_result != BLST_SUCCESS) {
        printf("blst_core_verify_pk_in_g1 failed ❌\n");
    }
    // Verification using pairing
    blst_pairing *pairing = malloc(blst_pairing_sizeof());
    if (!pairing) {
        printf("Failed to allocate pairing structure ❌\n");
        return 1;
    }

    blst_pairing_init(pairing, true, (uint8_t *)DST, strlen(DST));
    blst_pairing_aggregate_pk_in_g1(pairing, &pk_affine, &sig_affine, (uint8_t *)MESSAGE, strlen(MESSAGE), NULL, 0);
    blst_pairing_commit(pairing);
    int valid = blst_pairing_finalverify(pairing, NULL);
    free(pairing);
    return valid;
}

int main() {
    blst_scalar sk;
    uint8_t ikm[32];
    uint8_t pk_bytes[96];
    blst_p1_affine pk_affine;
    blst_p2_affine sig_affine;
    uint8_t sig_bytes[192];

    generateKeys(&sk, pk_bytes, &pk_affine);
    signMessage(sk, &sig_affine, sig_bytes);
    int valid = verifyMessage(pk_affine, sig_affine);

    // Output Results
    printf("Private Key (hex): ");
    for (int i = 0; i < sizeof(sk.b); i++) {
        printf("%02x", sk.b[i]);
    }
    printf("\n");

    printf("Public Key (hex): ");
    for (int i = 0; i < 96; i++) {
        printf("%02x", pk_bytes[i]);
    }
    printf("\n");

    printf("Signature (hex): ");
    for (int i = 0; i < 192; i++) {
        printf("%02x", sig_bytes[i]);
    }
    printf("\n");

    if (valid) {
        printf("Signature is VALID ✅\n");
    } else {
        printf("Signature is INVALID ❌\n");
    }

    return 0;
}



// int main () {
//     uint384_t msg;
//     generate_large_number384(&msg, 1);
//     printf("msg = \t0x%016lx", msg.chunk[5]);
//     for(int i = 4; i > -1; i--) {
//         printf("_%016lx", msg.chunk[i]);
//     }
//     printf("\n");
// }