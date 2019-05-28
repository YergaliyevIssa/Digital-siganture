#include <iostream>
#include <string>
#include <vector>
#include "param.h"
#include "rand.h"
using std::vector;
using std::string;
using std::cout;
using std::endl;

#include "GenPubKey.h"
#include "Private.h"
//false 512 бит, true 256 бит
// file_name[0] == priv_key, file_name[1] == file, file_name[2] == рез-тат ЭЦП
bool get_mode(int argc, char* argv[], vector <string> &file_name)
{
	string err;
	if (argc > 5) {
		err = "Too much parametrs";
		throw err;
	}
	if (argc < 3) {
		err = "Not enough parametrs";
		throw err;	
	}
	if (argc == 3) {
		file_name[0] = argv[1];
		file_name[1] = argv[2];
		return false;
	}
	if (argc == 4) {
		string tmp = argv[1];
		if (tmp == "-s") {
			file_name[0] = argv[2];
			file_name[1] = argv[3];
			return true;
		}
		file_name[0] = argv[1];
		file_name[1] = argv[2];
		file_name[2] = argv[3];
		return false;
	}
	string tmp = argv[1];
	if (tmp != "-s") {
		err = "Wrong parametr";
		throw err;
	}
	file_name[0] = argv[2];
	file_name[1] = argv[3];
	file_name[2] = argv[4];

	return true;
}

//x , y будут лежать координаты в форме Вейерштрасса
void from_Edwards_to_Weierstrass(uint1024_t &x, uint1024_t &y, const uint1024_t &u, const uint1024_t &v, 
								const Sequence& pset)
{
	uint1024_t P = pset.p;
	uint1024_t s = (((pset.e + (P - pset.d)) % P) * inverse(4, P - 2, P)) % P;
	uint1024_t t = (((pset.e + pset.d) % P) * inverse(6, P - 2, P)) % P;
	uint1024_t tmp = (((s * (1 + v)) % P) * inverse(1 + (P - v), P - 2, P)) % P;
	x = (tmp + t) % P;
	y = (tmp * inverse(u, P - 2, P)) % P;
	return;
} 

void just_func(size_t size, uint1024_t &K, uint1024_t &D, uint8_t* priv_key, uint8_t* for_rand)
{
	for (size_t i = 0; i < size; i++) {
		uint1024_t tmp = priv_key[size - 1 - i];
		D = D ^ (tmp << (8 * i));
		tmp = for_rand[size - 1 - i];
		K = K ^ (tmp << (8 * i));
	}
	return;
}

uint1024_t get_hash(bool mode, string &file_name, size_t size)
{
	uint8_t* e = new uint8_t[size];
	if (mode) {
		string exec = "./g256sum ";
		exec += file_name;
		std::system(exec.c_str());
		FILE* file = fopen("hash256", "rb");
		fread(e, 1, 32, file);
		fclose(file);
	}
	else {
		string exec = "./g512sum ";
		exec += file_name;
		std::system(exec.c_str());
		FILE* file = fopen("hash512", "rb");
		fread(e, 1, 64, file);
		fclose(file);	
	}

	uint1024_t E = 0;
	for (int i = 0; i < size; i++) {
		uint1024_t tmp = e[size - i - 1];
		E = E ^ (tmp << (8 * i));
	}
	delete []e;
	return E;
}

int main(int argc, char* argv[])
{

	vector <string> file_name(3, "file.crt");
	size_t size = 64;
	uint8_t* priv_key;
	uint8_t* for_rand;
	Sequence pset(ParamSet512);
	try {
		bool mode = get_mode(argc, argv, file_name);
		if (mode) {
			pset = ParamSet256;
			size = 32;
		}
		priv_key = new uint8_t[size];
		for_rand = new uint8_t[size];
		
		get_priv_key(file_name[0], priv_key, size); // в из файла берется ключ, и копируется в priv_key
		
		uint1024_t S = 0;
		uint1024_t R = 0;
		uint1024_t D = 0, K = 0;
		uint1024_t E = get_hash(mode, file_name[1],size);
		E = E % pset.q;
		if (E == 0)
			E = 1;
		while (S == 0) {
			while (R == 0) {
				genpvkey(for_rand, pset, size); //рандомим size байт в for_rand
				vector <uint1024_t> point = gen_pub_key(for_rand, size, pset); //pset.p * for_rand
				uint1024_t x, y;
				from_Edwards_to_Weierstrass(x, y, point[0], point[1], pset);
				R = x % pset.q;
			}
			just_func(size, K, D, priv_key, for_rand);
			S = ((R * D) % pset.q + (K * E) % pset.q) % pset.q;
		 
		}
	
		uint8_t* r_vector = new uint8_t[size];
		uint8_t* s_vector = new uint8_t[size];
		for (size_t i = 0; i < size; i++) {
			r_vector[i] = (uint8_t)((R >> (8 * (size - 1 - i))) & 0xff);
			s_vector[i] = (uint8_t)((S >> (8 * (size - 1 - i))) & 0xff);
		}
		for (size_t i = 0; i < size; i++)
			printf("%02x", r_vector[i]);
		cout << endl;
		for (size_t i = 0; i < size; i++)
			printf("%02x", s_vector[i]);
		cout << endl;
		FILE* f = fopen(file_name[2].c_str(), "wb");
		fwrite(r_vector, 1, size, f);
		fwrite(s_vector, 1, size, f);
		fclose(f);
		delete []priv_key;
		delete []r_vector;
		delete []s_vector;
		delete []for_rand;
	}
	catch (string &err) {
		std::cerr << err << endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unexpected error" << endl;
		return 1;
	}
	return 0;
}
