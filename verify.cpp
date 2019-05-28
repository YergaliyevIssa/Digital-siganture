#include <iostream>
#include <string>
#include <vector>
#include "param.h"
#include "rand.h"
using std::string;
using std::vector;
using std::cout;
using std::endl;

#include "GenPubKey.h"
#include "Private.h"


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

//false 512 бит, true 256 бит
// file_name[0] == public_key, file_name[1] == file, file_name[2] == рез-тат ЭЦП
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

vector <uint1024_t> get_signature(const string &file_name, size_t size, const uint1024_t& Q)
{
	string err;
	FILE* f = fopen(file_name.c_str(), "rb");
	if (f == NULL) {
		err = "FIle with signature does not exist";
		throw err;
	}
	uint8_t* s_vector = new uint8_t[size];
	uint8_t* r_vector = new uint8_t[size];
	size_t _size = fread(s_vector, 1, size, f);
	if (_size != size) {
		err = "Signature is not valid";
		throw err;
	}
	_size = fread(r_vector, 1, size, f);
	if (_size != size) {
		err = "Signature is not valid";
		throw err;
	}
	fclose(f);
	uint1024_t S = gen_1024_g(s_vector, size);
	uint1024_t R = gen_1024_g(r_vector, size);
	vector <uint1024_t> sign = {S, R};
	if ((S == 0 || S == Q) || (R == 0 || R == Q)) {
		err = "Signature is not valid";
		throw err;
	} 

 	return sign;
 }

uint1024_t get_hash(bool mode, string &file_name, size_t size)
{
	uint8_t* e = new uint8_t[size];
	string err = "Wrong hash value";
	if (mode) {
		string exec = "./g256sum ";
		exec += file_name;
		std::system(exec.c_str());
		FILE* file = fopen("hash256", "rb");
		size_t _size = fread(e, 1, 32, file);
		if (_size != 32) {
			throw err;
		}
		fclose(file);
	}
	else {
		string exec = "./g512sum ";
		exec += file_name;
		std::system(exec.c_str());
		FILE* file = fopen("hash512", "rb");
		size_t _size = fread(e, 1, 64, file);
		if (_size != 64) {
			throw err;
		}
		fclose(file);	
	}

	uint1024_t E = 0;
	for (size_t i = 0; i < size; i++) {
		uint1024_t tmp = e[size - i - 1];
		E = E ^ (tmp << (8 * i));
	}
	delete []e;
	return E;
}

vector <uint1024_t> get_pub_key(string &file_name, size_t size)
{
	string err;
	FILE* f = fopen(file_name.c_str(), "rb");
	if (f == NULL) {
		err = "File with public key does not exits";
		throw err;
	}
	err = "Wrong public key";
	uint8_t* x = new uint8_t[size];
	uint8_t* y = new uint8_t[size];

	size_t _size = fread(x, 1, size, f);
	_size = fread(y, 1, size, f);
	if (_size != size) {
		throw err;		
	}
	fclose(f);
	
	
	uint1024_t X = gen_1024_g(x, size);
	uint1024_t Y = gen_1024_g(y, size);
	delete []x;
	delete []y;

	vector <uint1024_t> Q = {X, Y};
	//cout << std::hex << Q[0] << endl << Q[1] << endl;
	return Q; 
}


int main(int argc, char  *argv[])
{
	vector <string> file_name(3, "file.crt");
 	size_t size = 64;
	Sequence pset(ParamSet512);
	try {
		bool mode = get_mode(argc, argv, file_name);
		if (mode) {
			size = 32;
			pset = ParamSet256;
		}
		vector <uint1024_t> sign = get_signature(file_name[2], size, pset.q); 
		//sing[0] == R, sign[1] == S
		uint1024_t E = get_hash(mode, file_name[1], size) % pset.q;
		if (E == 0)
			E = 1;
		uint1024_t V = inverse(E, pset.q - 2, pset.q);

		uint1024_t Z1 = (sign[1] * V) % pset.q; 
		uint1024_t Z2 = pset.q - (sign[0] * V) % pset.q;
		uint8_t* z1_vector = new uint8_t[size];
		uint8_t* z2_vector = new uint8_t[size];
		for (size_t i = 0; i < size; i++) {
			uint16_t tmp = 8 * i;
			z1_vector[size - i - 1] = (uint8_t)((Z1 >> tmp) & 0xff);
			z2_vector[size - i - 1] = (uint8_t)((Z2 >> tmp) & 0xff);
		}
		vector <uint1024_t> pub_key = get_pub_key(file_name[0], size);

		vector <uint1024_t> C1 = gen_pub_key(z1_vector, size, pset);
		pset.u = pub_key[0];
		pset.v = pub_key[1];
		vector <uint1024_t> C2 = gen_pub_key(z2_vector, size, pset);
		Sum_Point(C1[0], C1[1], C2[0], C2[1], pset);
		uint1024_t X = 0, Y = 0;
		from_Edwards_to_Weierstrass(X, Y, C1[0], C1[1], pset);
		uint1024_t R = X % pset.q;
		
		if (R == sign[0]) {
			cout << "Signature is valid" << endl;
		}
		else {
			cout << "Signature is not valid" << endl;
		}

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