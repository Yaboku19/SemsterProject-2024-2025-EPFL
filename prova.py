def mul_mont_n(ret, a, b, p, n0, n):
    """
    Implementa la funzione `mul_mont_n` in Python.

    Args:
        ret (list[int]): Array di output.
        a (list[int]): Primo operando (array di parole).
        b (list[int]): Secondo operando (array di parole).
        p (list[int]): Modulo (array di parole).
        n0 (int): Inverso moltiplicativo di -p mod R.
        n (int): Numero di parole.
    """
    assert n != 0 and n % 2 == 0, "n deve essere diverso da 0 e pari"

    tmp = [0] * (n + 1)
    limbx = 0
    mask = 0
    borrow = 0
    mx = 0
    hi = 0
    carry = 0

    # Primo ciclo: calcola tmp[i] = b[0] * a[i] + hi
    mx = b[0]
    for i in range(n):
        limbx = (mx * a[i]) + hi
        tmp[i] = limbx & 0xFFFFFFFFFFFFFFFF  # LIMB_T_BITS = 64
        hi = limbx >> 64
    tmp[n] = hi

    # Calcola mx = n0 * tmp[0]
    mx = n0 * tmp[0]

    # Ciclo principale
    for j in range(n):
        # Calcola tmp[i] = mx * p[i] + tmp[i] + carry
        limbx = (mx * p[0]) + tmp[0]
        hi = limbx >> 64
        for i in range(1, n):
            limbx = (mx * p[i] + hi) + tmp[i]
            tmp[i - 1] = limbx & 0xFFFFFFFFFFFFFFFF
            hi = limbx >> 64
        limbx = tmp[n] + hi + carry
        tmp[n - 1] = limbx & 0xFFFFFFFFFFFFFFFF
        carry = limbx >> 64

        if j + 1 == n:
            break

        # Calcola tmp[i] = b[j+1] * a[i] + tmp[i] + hi
        mx = b[j + 1]
        hi = 0
        for i in range(n):
            limbx = (mx * a[i] + hi) + tmp[i]
            tmp[i] = limbx & 0xFFFFFFFFFFFFFFFF
            hi = limbx >> 64
        mx = n0 * tmp[0]
        limbx = hi + carry
        tmp[n] = limbx & 0xFFFFFFFFFFFFFFFF
        carry = limbx >> 64

    # Sottrazione condizionale
    for i in range(n):
        limbx = tmp[i] - (p[i] + borrow)
        ret[i] = limbx & 0xFFFFFFFFFFFFFFFF
        borrow = (limbx >> 64) & 1

    mask = carry - borrow

    # Se necessario, copia tmp in ret
    for i in range(n):
        ret[i] = (ret[i] & ~mask) | (tmp[i] & mask)


# Esempio di utilizzo
if __name__ == "__main__":
    # # Parametri di esempio
    # n = 6  # Numero di parole (384 bit = 6 parole da 64 bit)
    # a = [0x15f65ec3fa80e493, 0x5c071a97a256ec6d, 0x77ce585370525745,
    #      0x5f48985753c758ba, 0xebf4000bc40c0002, 0x760900000002fffd]
    # b = [0x15f65ec3fa80e493, 0x5c071a97a256ec6d, 0x77ce585370525745,
    #      0x5f48985753c758ba, 0xebf4000bc40c0002, 0x760900000002fffd]
    # p = [0x1a0111ea397fe69a, 0x4b1ba7b6434bacd7, 0x64774b84f38512bf,
    #      0x6730d2a0f6b0f624, 0x1eabfffeb153ffff, 0xb9feffffffffaaab]
    # n0 = 0x89d3256894d213cc  # Esempio di n0
    # ret = [0] * n

    # # Esegui la moltiplicazione di Montgomery
    # mul_mont_n(ret, a, b, p, n0, n)

    # # Stampa il risultato
    # print("Risultato:", [hex(x) for x in ret])
    a = [
        [
            [
                1,
                2,
                3,
                4,
                5,
                6
            ],
            [
                7,
                8,
                9,
                10,
                11,
                12
            ]
        ],
        [
            [
                1,
                2,
                3,
                4,
                5,
                6
            ],
            [
                7,
                8,
                9,
                10,
                11,
                12
            ]
        ],
        [
            [
                1,
                2,
                3,
                4,
                5,
                6
            ],
            [
                7,
                8,
                9,
                10,
                11,
                12
            ]
        ],
        [
            [
                1,
                2,
                3,
                4,
                5,
                6
            ],
            [
                7,
                8,
                9,
                10,
                11,
                12
            ]
        ]
    ]

    b = [
        [
            [0, 0, 0, 0],
            [7, 7, 7, 7]
        ], [
            [1, 1, 1, 1],
            [8, 8, 8, 8]
        ], [
            [2, 2, 2, 2],
            [9, 9, 9, 9]
        ], [
            [3, 3, 3, 3],
            [10, 10, 10, 10]
        ], [
            [4, 4, 4, 4],
            [11, 11, 11, 11]
        ], [
            [5, 5, 5, 5],
            [12, 12, 12, 12]
        ]
    ]

    for i in range(4):
        print("a["+str(i)+"][0] = "+str(a[i][0][0])+str(a[i][0][1]),end="_")
        for j in range(2, 6, 2):
            print(str(a[i][0][j])+str(a[i][0][j+1]),end="_")
        print()
    
    for i in range(4):
        print("a["+str(i)+"][1] = "+str(a[i][1][0])+str(a[i][1][1]),end="_")
        for j in range(2, 6, 2):
            print(str(a[i][1][j])+str(a[i][1][j+1]),end="_")
        print()
    
    for i in range(4):
        print("a["+str(i)+"][0] = "+str(a[i][0][0])+str(a[i][0][1]),end="_")
        for j in range(2, 6, 2):
            print(str(a[i][0][j])+str(a[i][0][j+1]),end="_")
        print()
    
    for i in range(4):
        print("a["+str(i)+"][1] = "+str(b[0][1][i])+str(b[1][1][i]),end="_")
        for j in range(2, 6, 2):
            print(str(b[j][1][i])+str(b[j+1][1][i]),end="_")
        print()