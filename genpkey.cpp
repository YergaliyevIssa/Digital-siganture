#include <iostream>
#include <string>
#include "param.h"
#include "rand.h"
using std::string;
using std::cout;
using std::endl;
#include "Private.h"


bool get_mode(int argc, char* argv[]) {
	string err;
	if (argc > 3) {
		err = "Too much parametrs";
		throw err;
	}
	if (argc < 2) {
		err = "Undefined name of file";
		throw err;
	}
	if (argc == 2) 
		return false;
	else {
		string str = argv[1];
		if (str == "-s")
			return true;
		err = "Wrong parametr";
		throw err;  
	}
	return false;
}

FILE* open_file(bool p, Sequence &Param, char* argv[])
{
	string err;
	FILE* f;
	if (p) {
		Param = ParamSet256;
		f = fopen(argv[2], "wb");
	}
	else {
		Param = ParamSet512;
		f = fopen(argv[1], "wb");
	}
	if (f == NULL) {
		err = "Cannot open file";
		throw err;
	}
	return f;
}


int main(int argc, char* argv[])
{
	uint8_t* d;
	size_t size;
	Sequence pset(p1, a1, b1, e1, d1, m1, q1, x1, y, u1, v1);
	try {
		bool  mode = get_mode(argc, argv);
		FILE* f = open_file(mode, pset, argv);
		if (mode)
			size = 32;
		else 
			size = 64;
		d = new uint8_t[size];
		genpvkey(d, pset, size);
		fwrite(d, 1, size, f);
		fclose(f);
		for (size_t i = 0; i < size; i++)
			printf("%02x", d[i]);
		cout << endl;
		delete []d;
	}
	catch(string err) {
		std::cerr << err << endl;
		return 1;
	}
	catch(...) {
		std::cerr << "Unexpected error" << endl;
		return 1;
	}

	return 0;
}