def mul_mont_n_blst_fedele(a, b, p, n0, n):
    R_mask = (1 << 64) - 1

    tmp = [0] * (n + 1)
    ret = [0] * n

    mx = b[0]
    hi = 0
    for i in range(n):
        limbx = mx * a[i] + hi
        tmp[i] = limbx & R_mask
        hi = limbx >> 64
    tmp[n] = hi

    mx = (n0 * tmp[0]) & R_mask

    carry = 0
    j = 0
    while True:
        limbx = mx * p[0] + tmp[0]
        hi = limbx >> 64
        for i in range(1, n):
            limbx = mx * p[i] + hi + tmp[i]
            tmp[i - 1] = limbx & R_mask
            hi = limbx >> 64
        limbx = tmp[n] + hi + carry
        tmp[n - 1] = limbx & R_mask
        carry = limbx >> 64

        j += 1
        if j == n:
            break

        mx = b[j]
        hi = 0
        for i in range(n):
            limbx = mx * a[i] + tmp[i] + hi
            tmp[i] = limbx & R_mask
            hi = limbx >> 64
        limbx = hi + carry
        tmp[n] = limbx & R_mask
        carry = limbx >> 64

        mx = (n0 * tmp[0]) & R_mask

    borrow = 0
    for i in range(n):
        limbx = tmp[i] - p[i] - borrow
        if limbx < 0:
            limbx += 1 << 64
            borrow = 1
        else:
            borrow = 0
        ret[i] = limbx & R_mask

    mask = (carry - borrow) & R_mask
    for i in range(n):
        ret[i] = (ret[i] & ~mask) | (tmp[i] & mask)

    return ret

a =     "fa52f0bce9da131_b87a037fb3a18d64_6f11a0500ad1069d_cbd55d333484d9bd_1a22ab07b4ac8ab2_62c5e5f7e04e7df0"
b =     "15f65ec3fa80e493_5c071a97a256ec6d_77ce585370525745_5f48985753c758ba_ebf4000bc40c0002_760900000002fffd"
prime = "1a0111ea397fe69a_4b1ba7b6434bacd7_64774b84f38512bf_6730d2a0f6b0f624_1eabfffeb153ffff_b9feffffffffaaab"
a_arr = a.split("_")
b_arr = b.split("_")
prime_arr = prime.split("_")
a_arr.reverse()
b_arr.reverse()
prime_arr.reverse()
for i in range(6):
    a_arr[i] = int(a_arr[i], 16)
    b_arr[i] = int(b_arr[i], 16)
    prime_arr[i] = int(prime_arr[i], 16)
n0 = 0x89f3fffcfffcfffd

# 0x89f3fffcfffcfffd

a_num = int(a.replace("_", ""), 16)
prime_num = int(prime.replace("_", ""), 16)
print("a =\t" + hex(a_num))
print("prime =\t" + hex(prime_num))
print("ris =\t" + hex(a_num % 3))
