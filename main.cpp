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

	for (int i = 0; i < 4; i++) {
		bool found = false;
		for (int a = 0; a < 0xFFFF; a++) {
			if ((enc_key[i] * a % mod_key[i]) == 1) {
				dec_key[i] = a;
				found = true;
				break;
			}
		}
		if (!found) {
			cout << "Key not found" << endl;
			return 0;
		}
	}

	cout << "Key found" << endl;

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

	return 0;
}