#include <stdio.h>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <ctime>
#include <string>

using namespace std;


unsigned int load_key[4] = {0x3F08A79B, 0xE25CC287, 0x93D27AB9, 0x20DEA7BF},
	mod_key[4], xor_key[4], enc_key[4], dec_key[4] = {},
	matrix_orig[10][4], matrix_enc[10][4], dec_val[10][5];

void encrypt(unsigned int dest[4], unsigned int src[4], unsigned int enc_key[4])
{
	unsigned int enc_val = 0;
	for (int i = 0; i < 4; i++) {
		dest[i] = (((/*xor_key[i] ^*/ src[i] /*^ enc_val*/) * enc_key[i]) % mod_key[i]);
		enc_val = dest[i] & 0xFFFF;
	}
}

void decrypt(unsigned int dest[4], unsigned int src[4], unsigned int dec_key[4])
{
	unsigned int dec_val = 0;
	for (int i = 0; i < 4; i++) {
		dest[i] = ((dec_key[i] * src[i]) % mod_key[i]) ^ xor_key[i] ^ dec_val;
		dec_val = src[i] & 0xFFFF;
	}
}

bool check_key(unsigned int key, unsigned int x, unsigned int y)
{
	unsigned int dec = ((key * matrix_enc[x][y]) % mod_key[y]) ^ xor_key[y] ^ dec_val[x][y];
	dec_val[x][y + 1] = matrix_enc[x][y] & 0xFFFF;
	return (matrix_orig[x][y] == dec);
}

bool recur_brutforce(unsigned int i)
{
	for (int a = 0, found = 0; a < 0x0000FFFF; a++, found = 0) {
		for (int b = 0; b < 10; b++, found++) {
			if (!check_key(a, b, i)) {
				break;
			}
		}
		if (found == 10) {
			dec_key[i] = a;
			if (i == 3) {
				//printf("key found: %x %x %x %x\n", dec_key[0], dec_key[1], dec_key[2], dec_key[3]);
				return true;//key found
			} else if (recur_brutforce(i + 1)) {
				return true;
			}
		}
	}
	return false;
}

inline void xor_keyset()
{
	for (int i = 0; i < 4; i++) {
		mod_key[i] ^= load_key[i];
		enc_key[i] ^= load_key[i];
		dec_key[i] ^= load_key[i];
		xor_key[i] ^= load_key[i];
	}
}

int main()
{
	string keyname;
	size_t keysize;
	fstream file;
	char head[6];

	cout << "Key name:" << endl << ">";
	cin >> keyname;

	try {
		file.open(keyname, fstream::in | fstream::binary);

		file.seekg(0, file.end);
		keysize = file.tellg();
		file.seekg(0, file.beg);

		if (keysize != 54) {
			cout << "Invalid key size!" << endl;
			return 0;
		}

		file.read(head, 6);
		file.read((char *)mod_key, 16);
		file.read((char *)enc_key, 16);
		file.read((char *)xor_key, 16);

		file.close();
	} catch (...) {
		cout << "Can't read key" << endl;
		return 0;
	}

	xor_keyset();

	srand(time(NULL));
	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 4; y++) {
			matrix_orig[x][y] = rand() % 0x100;
		}
		encrypt(matrix_enc[x], matrix_orig[x], enc_key);
	}

	memset(dec_val, 0, sizeof(dec_val));
	if (recur_brutforce(0)) {
		cout << "Key found" << endl;
	} else {
		cout << "Key not found" << endl;
		system("pause");
		return 0;
	}

	xor_keyset();

	try {
		keyname.append(".dec");

		file.open(keyname, fstream::out | fstream::binary);

		file.write(head, 6);
		file.write((char *)mod_key, 16);
		file.write((char *)dec_key, 16);
		file.write((char *)xor_key, 16);

		file.close();
	} catch (...) {
		cout << "Can't save key" << endl;
		return 0;
	}

	system("pause");
	return 0;
}