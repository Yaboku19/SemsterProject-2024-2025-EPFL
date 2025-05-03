#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "blst.h"
#include <string.h>
#include <stdbool.h>
#include "header/struct.h"
#include "header/generation.h"

#define NUM_MESSAGES 100
#define DST "BLS_SIG_DST"

void generateKeys (blst_scalar *sk, blst_p1 *pk, uint8_t *pk_bytes, blst_p1_affine* pk_affine) {
    uint8_t ikm[32];
    for (int i = 0; i < 32; i++) {
        ikm[i] = rand() % 256;
    }
    blst_keygen(sk, ikm, 32, NULL, 0);
    blst_sk_to_pk2_in_g1(pk_bytes, pk_affine, sk);
    pk_bytes[0] &= ~0x20;
    blst_p1_from_affine(pk, pk_affine);
}

void signMessage (blst_scalar sk, uint8_t *message, blst_p2 *sig, uint8_t *sig_bytes, blst_p2_affine *sig_affine) {
    blst_p2 msg_point;
    blst_hash_to_g2(&msg_point, message, sizeof(message), (uint8_t *)DST, strlen(DST), NULL, 0);
    blst_sign_pk_in_g1(sig, &msg_point, &sk);
    blst_p2_serialize(sig_bytes, sig);
    blst_p2_to_affine(sig_affine, sig);
    if (!blst_p2_affine_in_g2(sig_affine)) {
        printf("Signature failed group check ❌\n");
        return;
    }
}

void aggrPublicKeysInPairs2 (blst_p1 *agr_pk, blst_p1 *pks, int num) {
    int new_n = num;
    while (new_n > 1) {
        int groups = new_n / 2;
        for (int k = 0; k < groups; k ++) {
            blst_p1_add(&pks[k], &pks[k * 2], &pks[(k * 2) + 1]);
        }
        if (groups * 2 < new_n) {
            pks[groups] = pks[groups * 2];
            new_n = groups + 1;
        } else {
            new_n = groups;
        }
    }
    *agr_pk = pks[0];
}

void aggrPublicKeysFourByFour2 (blst_p1 *agr_pk, blst_p1 *pks, int num) {
    blst_four_p1_add(agr_pk, pks, num);
}

int main () {
    blst_p1 agr_pk;
    blst_p1 pks[NUM_MESSAGES];
    blst_scalar sks[NUM_MESSAGES];
    uint8_t pk_bytes[NUM_MESSAGES][96];
    blst_p1_affine pk_affines[NUM_MESSAGES];
    for (int i = 0; i < NUM_MESSAGES; i++) {
        //generateKeys(&sks[i], &pks[i], pk_bytes[i], &pk_affines[i]);
        memset(&pks[i], 0xFFFFFFFFFFF, sizeof(blst_p1));
    }
    aggrPublicKeysInPairs2(&agr_pk, pks, NUM_MESSAGES);
    aggrPublicKeysFourByFour2(&agr_pk, pks, NUM_MESSAGES);
}