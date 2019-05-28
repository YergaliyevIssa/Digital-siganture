
uint1024_t gen_1024(uint8_t* t, size_t size)
{
	uint1024_t ans = 0;
	for (size_t i = 0; i < size; i++) {
		uint1024_t tmp = t[i];
		ans = ans ^ (tmp << (8 * i));
	}
	return ans;
}

uint1024_t gen_1024_g(uint8_t* t, size_t size)
{
	uint1024_t ans = 0;
	for (size_t i = 0; i < size; i++) {
		uint1024_t tmp = t[size - i - 1];
		ans = ans ^ (tmp << (8 * i));
	}
	return ans;
}

void genpvkey(uint8_t* d, Sequence & param, size_t size)
{
	uint8_t* t;
	uint1024_t T;
	t = new uint8_t[size];
	// в t[0] хранится самый млдаший разряд
	do {
		rand_bytes(t, size);
		T = gen_1024(t, size);
		//cout << std::hex << T << endl;
	} while (T == 0 || T >= param.q);
	memcpy(d, t, size);
	delete []t;
	for (size_t i = 0; i < size / 2; i++) { 
		std::swap(d[i], d[size - i - 1]); // теперь d[0] лежит старший байт
	}
	return;
}
