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

void aggrSignatures (blst_p2 *agr_sig, blst_p2_affine *agr_sig_affine, blst_p2_affine a_sig_affine, blst_p2_affine b_sig_affine) {
    blst_p2 a_sig_proj, b_sig_proj;
    blst_p2_from_affine(&a_sig_proj, &a_sig_affine);
    blst_p2_from_affine(&b_sig_proj, &b_sig_affine);
    blst_p2_add_or_double(agr_sig, &a_sig_proj, &b_sig_proj);
    blst_p2_to_affine(agr_sig_affine, agr_sig);
}

void aggrPublicKeys (blst_p1 *agr_pk, blst_p1_affine *agr_pk_affine, blst_p1_affine a_pk_affine, blst_p1_affine b_pk_affine) {
    blst_p1 a_pk_proj, b_pk_proj;
    blst_p1_from_affine(&a_pk_proj, &a_pk_affine);
    blst_p1_from_affine(&b_pk_proj, &b_pk_affine);
    blst_p1_add_or_double(agr_pk, &a_pk_proj, &b_pk_proj);
    blst_p1_to_affine(agr_pk_affine, agr_pk);
}

void signVerifyTwoMessages () {
    blst_scalar a_sk, b_sk;
    uint8_t a_pk_bytes[96], b_pk_bytes[96];
    blst_p1 agr_pk;
    blst_p1_affine a_pk_affine, b_pk_affine, agr_pk_affine;
    blst_p2 agr_sig;
    blst_p2_affine a_sig_affine, b_sig_affine, agr_sig_affine;
    uint8_t a_sig_bytes[192], b_sig_bytes[192];
    uint384_t msg_a, msg_b;
    uint8_t msg_a_bytes[48], msg_b_bytes[48];
    generate_large_number384(&msg_a, 1, (unsigned int)rand());
    //generate_large_number384(&msg_b, 1, (unsigned int)rand());
    msg_b = msg_a;
    memcpy(msg_a_bytes, msg_a.chunk, 48);
    memcpy(msg_b_bytes, msg_b.chunk, 48);
    int a_valid, b_valid;
    
    generateKeys(&a_sk, a_pk_bytes, &a_pk_affine);
    generateKeys(&b_sk, b_pk_bytes, &b_pk_affine);
    signMessage(a_sk, &a_sig_affine, a_sig_bytes, msg_a_bytes);
    signMessage(b_sk, &b_sig_affine, b_sig_bytes, msg_b_bytes);
    a_valid = verifyMessage(a_pk_affine, a_sig_affine, msg_a_bytes);
    b_valid = verifyMessage(b_pk_affine, b_sig_affine, msg_b_bytes);

    aggrSignatures(&agr_sig, &agr_sig_affine, a_sig_affine, b_sig_affine);
    aggrPublicKeys(&agr_pk, &agr_pk_affine, a_pk_affine, b_pk_affine);

    uint8_t msg_agr_bytes[96];
    memcpy(msg_agr_bytes, msg_a_bytes, 48);
    memcpy(msg_agr_bytes + 48, msg_b_bytes, 48);
    int agr_valid = verifyMessage(agr_pk_affine, agr_sig_affine, msg_agr_bytes);

    printf("----------------------------------------------------- [ messages ] -----------------------------------------------------\n");
    printf("a: \t\t\t%016lx", msg_a.chunk[5]);
    for(int i = 4; i > -1; i--) {
        printf("_%016lx", msg_a.chunk[i]);
    }
    printf("\n\n");
    printf("b: \t\t\t%016lx", msg_b.chunk[5]);
    for(int i = 4; i > -1; i--) {
        printf("_%016lx", msg_b.chunk[i]);
    }
    printf("\n\n");
    printf("----------------------------------------------------- [ messages_bytes ] -----------------------------------------------------\n");
    printf("message a(hex): \t");
    for (int i = 0; i < sizeof(msg_a_bytes); i++) {
        printf("%02x", msg_a_bytes[i]);
    }
    printf("\n\n");
    printf("message b(hex): \t");
    for (int i = 0; i < sizeof(msg_b_bytes); i++) {
        printf("%02x", msg_b_bytes[i]);
    }
    printf("\n\n");
    printf("message agr(hex): \t");
    for (int i = 0; i < sizeof(msg_agr_bytes); i++) {
        printf("%02x", msg_agr_bytes[i]);
    }
    printf("\n\n");
    printf("----------------------------------------------------- [ private key ] -----------------------------------------------------\n");
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
    printf("----------------------------------------------------- [ public key ] -----------------------------------------------------\n");
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
    printf("Public Key agr(hex): \t");
    uint8_t agr_pk_bytes[96];
    blst_p1_serialize(agr_pk_bytes, &agr_pk);
    for (int i = 0; i < 96; i++) {
        printf("%02x", agr_pk_bytes[i]);
    }
    printf("\n\n");
    printf("----------------------------------------------------- [ signature ] -----------------------------------------------------\n");
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
    printf("Signature agr(hex): \t");
    uint8_t agr_sig_bytes[192];
    blst_p2_serialize(agr_sig_bytes, &agr_sig);
    for (int i = 0; i < 192; i++) {
        printf("%02x", agr_sig_bytes[i]);
    }
    printf("\n\n");
    printf("----------------------------------------------------- [ validation ] -----------------------------------------------------\n");
    if (a_valid) {
        printf("Signature a is VALID ✅");
    } else {
        printf("Signature a is INVALID ❌");
    }
    printf("\n\n");
    if (b_valid) {
        printf("Signature b is VALID ✅");
    } else {
        printf("Signature b is INVALID ❌");
    }
    printf("\n\n");
    if (agr_valid) {
        printf("Signature agr is VALID ✅");
    } else {
        printf("Signature agr is INVALID ❌");
    }
    printf("\n");
}

void signVerifySomeMessages (int num_messages) {
    blst_scalar *sks = malloc(num_messages * sizeof(blst_scalar));
    if (!sks) {
        printf("Failed to allocate memory for secret keys ❌\n");
        return;
    }
    uint8_t (*pk_bytes)[96] = malloc(num_messages * sizeof(*pk_bytes));
    if (!pk_bytes) {
        printf("Failed to allocate memory for public key bytes ❌\n");
        return;
    }
    blst_p1 agr_pk;
    blst_p1_affine agr_pk_affine, *pk_affines = malloc(num_messages * sizeof(blst_p1_affine));
    if (!pk_affines) {
        printf("Failed to allocate memory for public key affines ❌\n");
        return;
    }
    blst_p2 agr_sig;
    blst_p2_affine agr_sig_affine, *sig_affines = malloc(num_messages * sizeof(blst_p2_affine));
    if (!sig_affines) {
        printf("Failed to allocate memory for signature affines ❌\n");
        return;
    }
    uint8_t (*sig_bytes)[192] = malloc(num_messages * sizeof(*sig_bytes));
    if (!sig_bytes) {
        printf("Failed to allocate memory for signature bytes ❌\n");
        return;
    }
    uint384_t *msgs = malloc(num_messages * sizeof(uint384_t));
    if (!msgs) {
        printf("Failed to allocate memory for messages ❌\n");
        return;
    }
    uint8_t (*msg_bytes)[48] = malloc(num_messages * sizeof(*msg_bytes));
    if (!msg_bytes) {
        printf("Failed to allocate memory for message bytes ❌\n");
        return;
    }
    uint8_t *msg_agr_bytes = malloc(num_messages * 48);
    if (!msg_agr_bytes) {
        printf("Failed to allocate memory for aggregated message bytes ❌\n");
        return;
    }
    int valid = 1, valid_agr;
    /* Generate messages */
    generate_large_number384(&msgs[0], 1, (unsigned int)rand());
    memcpy(msg_bytes[0], msgs[0].chunk, 48);
    memcpy(msg_agr_bytes, msg_bytes[0], 48);
    for (int i = 1; i < num_messages; i++) {
        msgs[i] = msgs[0];
        memcpy(msg_bytes[i], msgs[i].chunk, 48);
        memcpy(msg_agr_bytes + (48 * i), msg_bytes[i], 48);
    }
    /* Generate keys */
    for (int i = 0; i < num_messages; i++) {
        generateKeys(&sks[i], pk_bytes[i], &pk_affines[i]);
    }
    /* Sign messages */
    for (int i = 0; i < num_messages; i++) {
        signMessage(sks[i], &sig_affines[i], sig_bytes[i], msg_bytes[i]);
    }
    /* Verify messages */
    for (int i = 0; i < num_messages; i++) {
        valid = valid && verifyMessage(pk_affines[i], sig_affines[i], msg_bytes[i]);
    }
    /* Aggregate signatures */
    aggrSignatures(&agr_sig, &agr_sig_affine, sig_affines[0], sig_affines[1]);
    for (int i = 2; i < num_messages; i++) {
        aggrSignatures(&agr_sig, &agr_sig_affine, agr_sig_affine, sig_affines[i]);
    }
    /* Aggregate public keys */
    aggrPublicKeys(&agr_pk, &agr_pk_affine, pk_affines[0], pk_affines[1]);
    for (int i = 2; i < num_messages; i++) {
        aggrPublicKeys(&agr_pk, &agr_pk_affine, agr_pk_affine, pk_affines[i]);
    }
    /* Verify aggregated signature */
    valid_agr = verifyMessage(agr_pk_affine, agr_sig_affine, msg_agr_bytes);
    if (valid) {
        printf("Signatures are VALID ✅");
    } else {
        printf("Signatures are INVALID ❌");
    }
    printf("\n\n");
    if (valid_agr) {
        printf("Signature aggr is VALID ✅");
    } else {
        printf("Signature aggr is INVALID ❌");
    }
    printf("\n\n");
}

int main() {
    srand(time(0));
    signVerifySomeMessages(10);
    return 0;
}