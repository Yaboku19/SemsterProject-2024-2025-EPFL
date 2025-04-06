def my_mul ():
    # 0xdcde021b_5d0b4330_c0713aff_f539bb85_ad3939cd_b174edc4_705cdbb2_f6be0176_dac30e06_151c7cc3_a4cbbb51_1c41bc5c
    # 0x1a0111ea_397fe69a_4b1ba7b6_434bacd7_64774b84_f38512bf_6730d2a0_f6b0f624_1eabfffe_b153ffff_b9feffff_ffffaaab
    # 0xdcde021b_5d0b4330_c0713aff_f539bb85ad3939cdb174edc4705cdbb2f6be0176dac30e06151c7cc3a4cbbb511c41bc5c
    a = [0xdcde021b, 0x5d0b4330, 0xc0713aff, 0xf539bb85, 0xad3939cd, 0xb174edc4, 0x705cdbb2, 0xf6be0176, 0xdac30e06, 0x151c7cc3, 
        0xa4cbbb51, 0x1c41bc5c]
    b = [0x1a0111ea, 0x397fe69a, 0x4b1ba7b6, 0x434bacd7, 0x64774b84, 0xf38512bf, 0x6730d2a0, 0xf6b0f624, 0x1eabfffe, 0xb153ffff,
        0xb9feffff, 0xffffaaab]
    a.reverse()
    b.reverse()
    ris = [0] * 24
    carry = 0
    for i in range(12):
        for j in range(12):
            temp = abs(a[i]) * abs(b[j])
            temp += abs(ris[i + j]) + abs(carry)
            temp = abs(temp)
            carry = temp >> 32
            ris[i + j] = temp & 0xffffffff
    ris[23] = carry
    for i in range(23, -1, -1):
        print(hex(ris[i]),  end="_")
    print("\n-----------------------")
def montgomery_multiplication(a, b, p):
    """
    Implementa la moltiplicazione di Montgomery.

    Args:
        a (int): Primo operando (384 bit).
        b (int): Secondo operando (384 bit).
        p (int): Modulo (384 bit).

    Returns:
        int: Risultato della moltiplicazione di Montgomery.
    """
    # Calcola R = 2^384
    R = 1 << 384

    # Calcola n0 come l'inverso moltiplicativo di -p mod R
    n0 = -pow(p, -1, R) % R

    # Calcola il prodotto t = a * b
    t = a * b
    print("t = " + hex(t))

    # Calcola m = (t * n0) mod R
    m = (t * n0) & (R - 1)
    # Calcola u = (t + m * p) / R
    u = (t + m * p) >> 384
    print("m * p = " +  hex((m * p)))
    print("m * p + t= " +  hex((t + m * p)))
    print("-"*50)
    # Se u >= p, riduci modulo p
    if u >= p:
        u -= p

    return u



# Modulo p (384 bit)
p = 0x1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab
a_dash = "0f1fbe0ef913ef77_f35cc0cc24d16fa6_5c6390814f4e2c71_b12bdd1490201e07_05c5f2214542a91e_ffd93a7995b82354".replace("_", "")
b_dash = "02ec6d906e960b93_45a5fa5428a901c9_a4cf9964ee7c8c24_c56f01f1dc303c59_3e729f97c6a99335_4a729d6536d71097".replace("_", "")
ris_a = "169bafc1fc4b974a_e0fdd29032746711_ee3c6956ebc1bfd7_c38b614185430837_b401ffcc72ed439b_3a57108ff2b5497a".replace("_", "")
# Operandi a e b (384 bit ciascuno)
a = int(a_dash, 16)
b = int(b_dash, 16)

# Esegui la moltiplicazione di Montgomery
result = montgomery_multiplication(a, b, p)

# Stampa il risultato
print(f"Risultato c: {hex(result)}")
print(f"Risultato a: 0x{ris_a}")
#   0x169bafc1fc4b974a_e0fdd29032746711_ee3c6956ebc1bfd7_c38b614185430837_b401ffcc72ed439b_3a57108ff2b5497a
#   0x169bafc1fc4b974a_e0fdd29032746711_ee3c6956ebc1bfd7_c38b614185430837_b401ffcc72ed439b_3a57108ff2b5497a
# 0x1_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000 R
#   0xceb06106_feaafc94_68b316fe_e268cf58_19ecca0e_8eb2db4c_16ef2ef0_c8e30b48_286adb92_d9d113e8_89f3fffc_fffcfffd n0
# 24         23       22       21       20       19       18       17       16       15       14       13       12       11       10       9        8        7        6        5        4        3        2        1      
# 0x166f7889_9c3c079e_ba07531b_3de980c9_b050c874_e465d6e6_39908285_8228e7bb_99718130_ed8f6f91_07028e34_a8c04870_fc20badd_e69e7b4f_d59fa588_a566864d_eb227f00_6ec3f14c_567bc239_977a3b94_4634390a_0942878f_beccdfaf_caa1e974
# 0x166f7889_9c3c079e_ba07531b_3de980c9_b050c874_e465d6e6_39908285_8228e7bb_99718130_ed8f6f91_07028e34_a8c04870_fc20badd_e69e7b4f_d59fa588_a566864d_eb227f00_6ec3f14c_567bc239_977a3b94_4634390a_0942878f_beccdfaf_caa1e974
# 0x166f7889_92c87f3d_a67b0535_25009d5b_9eb83f3d_d25f38e1_2e269bfc_69189393_8338c7b2_eb6a744e_f6452cda_a5e17d14_0594433e_fa2ac935_ee8888f6_b6ff0f84_fd291d05_7a2dd7d5_6f8c1661_adb2f512_4859344c_19ffe8e9_c1abab0c_caa1e974