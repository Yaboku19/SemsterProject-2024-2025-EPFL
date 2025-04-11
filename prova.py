def mod(a_array, mod_array):
    result_array = []
    
    # Concatenazione dei blocchi in un unico grande numero
    a_concat = 0
    for a in a_array:
        a_concat = (a_concat << 64) | a
    
    mod_concat = 0
    for b in mod_array:
        mod_concat = (mod_concat << 64) | b
    
    # Iniziamo il processo di sottrazione con shifting
    shift = a_concat.bit_length() - mod_concat.bit_length()
    shift_2 = a_array[0].bit_length() - mod_array[0].bit_length()
    print("shifit = " + str(shift))
    print("shift_2 = " + str(shift_2))
    while shift >= 0:
        if a_concat >= (mod_concat << shift):
            a_concat -= (mod_concat << shift)
        shift -= 1
    
    # Dividiamo il risultato nei blocchi come array
    for i in range(len(a_array)):
        result_array.append((a_concat >> (64 * (len(a_array) - i - 1))) & ((1 << 64) - 1))

    return result_array

a_str =   "0xFa57108ff2b5497a_b401ffcc72ed439b_c38b614185430837_ee3c6956ebc1bfd7_e0fdd29032746711_169bafc1fc4b974a"
a_array = a_str.split("_")
mod_str = "0x1a0111ea397fe69a_4b1ba7b6434bacd7_64774b84f38512bf_6730d2a0f6b0f624_1eabfffeb153ffff_b9feffffffffaaab"
mod_array = mod_str.split("_")
for i in range(6):
    a_array[i] = int(a_array[i], 16)
for i in range(6):
    mod_array[i] = int(mod_array[i], 16)

print("a =\t" + hex(a_array[0]), end = "")
for i in range(1, 6):
    print("_" + hex(a_array[i])[2:], end = "")
print()
print("mod =\t" + hex(mod_array[0]), end = "")
for i in range(1, 6):
    print("_" + hex(mod_array[i])[2:], end = "")
print()
num_a = int(a_str.replace("_", ""), 16)
num_mod = int(mod_str.replace("_", ""), 16)

ris_array = mod(a_array, mod_array)
print("ris =\t" + hex(ris_array[0]), end = "")
for i in range(1, 6):
    print("_" + hex(ris_array[i])[2:], end = "")
print()

print("mod2 =\t" + hex(num_a % num_mod))

# 0x3a57108ff2b5497a_b401ffcc72ed439b_c38b614185430837_ee3c6956ebc1bfd7_e0fdd29032746711_169bafc1fc4b974a
# 0x3a57108ff2b5497a_b401ffcc72ed439b_c38b614185430837_ee3c6956ebc1bfd7_e0fdd29032746711_169bafc1fc4b974a