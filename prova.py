#    1972b7d8_1caacb3c_7936d83e_186ca72c_0450caf0_ac57b2c3_1cfdd1c2_489dfd3e_3d1e9cf5_b265b226_5d7160ff_c1915ece
a = "13feb392_7ca422fd_49d66ce7_036e2ffb_034c68e0_d3b8ed55_64a10eb2_3853b29c_55045b46_a393b867_44c0632a_86f49ff2"
b = "073ba603_24611aaa_d32185e0_0b7f86ac_f55c161b_20f05f91_cbe9de6c_bb7d4e83_ea93fe24_d4641e9e_461620bd_9e4841bb"

a_arr = a.split("_")
b_arr = b.split("_")
for i in range(12):
    a_arr[i] = int(a_arr[i], 16)
    b_arr[i] = int(b_arr[i], 16)
print("normal------")
print("{" + hex(a_arr[10]) + hex(a_arr[11])[2 :], end = "")
for i in range(9, -1, -2):
    print(", " + hex(a_arr[i-1]) + hex(a_arr[i])[2 :], end = "")
print("};")
print("{" + hex(b_arr[10]) + hex(b_arr[11])[2 :], end = "")
for i in range(9, -1, -2):
    print(", " + hex(b_arr[i-1]) + hex(b_arr[i])[2 :], end = "")
print("};")
print("other---------")
print("{\n\t{" + hex(a_arr[11]) + ", "+ hex(a_arr[11]) + ", "+ hex(a_arr[11]) + ", "+ hex(a_arr[11]) + "}", end = "")
for i in range(10, -1, -1):
    print(",\n\t{" + hex(a_arr[i]) + ", "+ hex(a_arr[i]) + ", "+ hex(a_arr[i]) + ", "+ hex(a_arr[i]) + "}" , end = "")
print("\n};")
print("{\n\t{" + hex(b_arr[11]) + ", "+ hex(b_arr[11]) + ", "+ hex(b_arr[11]) + ", "+ hex(b_arr[11]) + "}", end = "")
for i in range(10, -1, -1):
    print(",\n\t{" + hex(b_arr[i]) + ", "+ hex(b_arr[i]) + ", "+ hex(b_arr[i]) + ", "+ hex(b_arr[i]) + "}" , end = "")
print("\n};")