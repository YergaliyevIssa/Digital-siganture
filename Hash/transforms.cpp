#include "transforms.h"


using std::cout;
using std::endl;
using std::hex;


uint8_t power(int j) // возводит 2^j
{
	return (1 << j);
}


size_t data_read(FILE *f, context &ctx) // читает порциями по 64 байта 
{
    size_t size = 0;
    uint8_t data[64] = {0};
    size = fread(data, sizeof(uint8_t), 64, f);   
    for (size_t i = 0; i < size; i++) 
    	ctx.data[i] = data[i];

    return size;
}

void sum_512(uint8_t* tmp, uint8_t* M) //суммирует по модулю 2^512
{
	uint64_t buf;
	bool L[512], R[512], overflow = false;
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 8; j++) {
			L[8 * i + j] = (tmp[i] >> j) & 1;
			R[8 * i + j] = (M[i] >> j) & 1;
		}
	for (int i = 0; i < 512; i++) {
		if (L[i] == R[i]) {
			if (overflow) 
				L[i] = true;
			else 
				L[i] = false;
			if (R[i])
				overflow = true;
			else
				overflow = false;
		}
		else {
			if (overflow)
				L[i] = false;
			else
				L[i] = true;
			if (L[i])
				overflow = false;
			else
				overflow = true;
		}
	}
	for (int i = 0; i < 64; i++) {
		buf = 0;
		for (int j = 0; j < 8; j++) {
			buf += power(j) * L[8 * i + j];
		}
		tmp[i] = buf;
	}

}

void add(uint8_t* data, uint8_t size) 
{
	data[size] = 1;
	for (int i = size + 1; i < 64; i++)
		data[i] = 0;
}


void X(uint8_t* K, uint8_t* a)
{
	for (int i = 0; i < 64; i++)
		a[i] = K[i] ^ a[i];
	return; 
}

void S(uint8_t* a) // перестановка по значению 64 байта вектор
{
//	uint64_t tmp, permutation = 0;
	for (int i = 0; i < 64; i++) {
		a[i] = nonlinear_bi[a[i]];
	}
	return;
}

void P(uint8_t* a) // перествновка по индексу 64 байта вектор
{
	uint8_t* array = new uint8_t[64];
	for (int i = 0; i < 64; i++)
		array[i] = a[i];
	for (int i = 0; i < 64; i++)
		a[i] = array[tau[i]];
	delete[] array;
	return;
}


void mp(uint8_t* b) //0 - 7 умножение 8 байт вектор
{
	uint64_t c = 0;
	bool bits[64];
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			bits[8 * i + j] = ((b[i] >> j) & 1);  
	for (int i = 0; i < 64; i++)
		c = c ^ (bits[i] * matrix_a[63 - i]);
	for (int i = 0; i < 8; i++) {
		b[i] = (c >> (8 * i)) & 255;
	}
}

void L(uint8_t* a) // умножение на матрицу 64 байта вектор
{
	uint8_t tmp[8];
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			tmp[j] = a[8 * i + j];  
		}
		mp(tmp);
		for (int j = 0; j < 8; j++) {
			a[8 * i + j] = tmp[j];
		}
	}
	return;
}


void E(uint8_t* K, uint8_t* m) // возможны, ошибка с индексацией
{
//	uint8_t tmp[64];
	for (int i = 0; i < 12; i++) {
		X(K, m);
		S(m);
		P(m);
		L(m);
		for (int j = 0; j < 64; j++)
			K[j] = K[j] ^ C[i][63 - j]; // здесь
		S(K);
		P(K);
		L(K);
	//	for (int i = 0; i < 64; i++)
	//		printf("%02x", K[63 - i]);
	//	cout << endl;
	}
	X(K, m);
//	for (int i = 0; i < 64; i++)
//		printf("%02x", m[63 - i]);
//	cout << endl;
}

void gN(uint8_t* h, uint8_t* m, uint8_t* N) // возможны ошибки с индексацией
{
	uint8_t tmp[64], res[64];
	for (int i = 0; i < 64; i++)
		tmp[i] = N[i] ^ h[i];
	S(tmp);
	P(tmp);
	L(tmp);
	for (int i = 0; i < 64; i++) {
		res[i] = m[i];
	}
	E(tmp, res);
	for (int i = 0; i < 64; i++) {
		res[i] = res[i] ^ m[i] ^ h[i];
		h[i] = res[i];
	}
	return;
}







