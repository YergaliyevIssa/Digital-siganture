#include <string>
#include <iostream>
#include <vector>
#include <stdio.h>
#include "param.h"

using std::vector;	
using std::string;
using std::cout;
using std::endl;

#include "GenPubKey.h"
//file_name[0] путь до файла с приватным ключом
//file_name[1] путь до файла куда публичный ключ будет записан
//false 512 бит, true 256 бит
bool get_mode(int argc, char* argv[], vector <string> &file_name)
{
	string err;
	if (argc < 2) {
		err = "Undefined name of file";
		throw err;
	}
	else if (argc > 4) {
		err = "Too much parametrs";
		throw err;
	}
	if (argc == 2) {
		file_name[0] = argv[1];
		return false;
	}
	else if (argc == 3) {
		string str = argv[1];
		if (str == "-s") {
			file_name[0] = argv[2];
			return true;
		}
		file_name[0] = argv[1];
		file_name[1] = argv[2];
		return false;
	}
	else {
		string str = argv[1];
		if (str == "-s") {
			file_name[0] = argv[2];
			file_name[1] = argv[3];
			return true;
		}
		err = "Wrong parametrs";
		throw err;
	}
	return false;
}

int main(int argc, char *argv[])
{
	vector <string> file_name(2, "file.pub");
	uint8_t* pub_key[2];
	uint8_t* priv_key; 
	size_t size = 64;
	Sequence pset(ParamSet512);
	try {
		bool mode = get_mode(argc, argv, file_name);
		if (mode) {
			size = 32;
			pset = ParamSet256;
		}
		cout << "size = " << size << endl;
		priv_key = new uint8_t[size];
		pub_key[0] = new uint8_t[size];
		pub_key[1] = new uint8_t[size];

		get_priv_key(file_name[0], priv_key, size);
		
		gen_pub_key(priv_key, pub_key, size, pset);
		
		cout << "Public key : " << endl;
		for (int j = 0; j < 2; j++) {
			for (size_t i = 0; i < size; i++)
				printf("%02x", pub_key[j][i]);
			cout << endl;
		}
		FILE* f = fopen(file_name[1].c_str(), "wb");
		fwrite(pub_key[0], 1, size, f);
		fwrite(pub_key[1], 1, size, f);
		fclose(f);
		delete []priv_key;
		delete []pub_key[0];
		delete []pub_key[1];
	}
	catch(string err) {
		std::cerr << err << endl;
	}
	catch(...) {
		std::cerr << "Unexpected error" << endl;
	}
	return 0;
}