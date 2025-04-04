# 06235810_d2c41c67_40405cc0_95e50359_f0b8421e_5bc736c7_d408e20c_9cbb2503_60b84cc6_dc0cdcd1_5dfac7c5_0fc3960a
# 18a97cb2_6ef179c4_303ef4bd_6b8c9342_04bcebf6_9ad0f2c0_68fc16df_1637e7c4_2e4eb358_6a273120_b04f937d_6734d7de
fst_1 =    "17845520_29e56a41_9817c17e_a10c4154_70875ac2_bec64327_f7b14647_46447580_96db9172_2868700d_d34b7bab_b7a5545c"
second_1 = "0cb53e8a_9ea119fc_013abd6d_41881691_a44b17f0_6c1c459f_64aa769a_c305ddcd_febc0b20_7f15d1be_9f800e32_7413c2c0"

fst_1_split = fst_1.replace('_', '')
second_1_split = second_1.replace('_', '')
fst_number_1 = [int(part, 16) for part in fst_1_split.split('_')][0]
second_number_1 = [int(part, 16) for part in second_1_split.split('_')][0]

list_fst_1 = [int(part, 16) for part in fst_1.split('_')]
list_sdn_1 = [int(part, 16) for part in second_1.split('_')]

ris_list_1 = [0] * 12
list_rest = [0] * 12
list_sub = [0] * 12
rest = 0
for i in range(11, -1, -1):
    op1 = list_fst_1[i]
    op2 = list_sdn_1[i]
    ris = op1 - op2
    list_sub[i] = ris & 0xFFFFFFFF
    ris = ris - rest
    list_rest[i] = rest
    rest = abs(ris >> 32)
    ris = abs(ris & 0xFFFFFFFF)
    ris_list_1[i] = ris


ris_1 = abs(fst_number_1 - second_number_1)
# print("--------------------")
# print("fst_1: " + hex(fst_number_1))
# print("snd_1: " + hex(second_number_1))
# print("ris_1: " + hex(ris_1))
# print("--------------------")
print("\ta\t\tb\t\tsub\t\trest\tsub")
for i in range(11, -1, -1):
    print(str(11-i), end="\t")
    print(hex(list_fst_1[i]), end="\t")
    print(hex(list_sdn_1[i]), end="\t")
    print(hex(list_sub[i]), end="\t")
    print(hex(list_rest[i]), end="\t")
    print()




# right: 0x0acf1695_8b445045_96dd0411_5f842ac2_cc3c42d2_52a9fd88_9306cfac_833e97b2_981f8651_a9529e4f_33cb6d79_4391919c
# uno:   0x0acf1695_8b445045_96dd0411_5f842ac2_cc3c42d2_52a9fd88_9306cfac_833e97b2_981f8651_a9529e4f_33cb6d79_4391919c
# due:   0x0acf1695_8b445045_96dd0411_5f842ac2_cc3c42d2_52a9fd88_9306cfac_833e97b2_981f8651_a9529e4f_33cb6d79_4391919c
# tre:   0x0acf1695_8b445045_96dd0412_5f842ac2_cc3c42d2_52a9fd89_9306cfac_833e97b3_981f8651_a9529e4f_33cb6d79_4391919c