#pragma once


#define SIZE_PRIVKEY_BYTES				(32U)
#define SIZE_PRIVKEY_WIF				(52U)
#define SIZE_HASH160					(20U)


#define FILE_PATH_RESULT "Found_Key.csv"

#pragma pack(push, 1)
struct retStruct {
	unsigned int found;
	unsigned char priv_key[SIZE_PRIVKEY_BYTES];
};
#pragma pack(pop)

