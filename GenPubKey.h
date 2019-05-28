

void get_priv_key(string &file_name, uint8_t* priv_key, size_t size)
{
	string err;
	FILE* f = fopen(file_name.c_str(), "rb");
	if (f == NULL) {
		err = "File with private key does not exist";
		throw err;
	}
	//uint8_t* buf = new uint8_t[size];
	size_t count = fread(priv_key, 1, size, f);
	if (count != size) {
		err = "Wrong private key";
		throw err;
	}
	fclose(f);
//	cout << "private Key : " << endl;
//	for (size_t i = 0; i < size; i++) {
//		printf("%02x", priv_key[i]);
//	}
	//cout << endl << "count = " << count << endl;
	return;	
}

//вспомогательная функция для написаня восходящего возведения в степень
vector <bool> make_order(size_t size, uint8_t* priv_key)
{
	int shift = 511;
	uint512_t p_key = 0;
	for (int i = size - 1; i > -1; i--) {
		uint512_t tmp = priv_key[i];
		p_key = p_key ^ (tmp << (8 * (size - i - 1)));
	}
	while (!(p_key >> shift))
		shift--;

	vector <bool> order(shift);	
	for (int j = shift - 1; j > -1; j--) {
		order[shift -j - 1] = (bool)((p_key >> j) & 1);
	}
	return order;
}


//нахождение обратного a^p-2 == a^-1;
uint1024_t inverse(uint1024_t elem, const uint1024_t &pow, const uint1024_t &mod)
{
	if (pow == 1)
		return elem;
	uint1024_t tmp = inverse(elem, pow / 2, mod) % mod;
	uint1024_t buf = (tmp * tmp) % mod;
	if (pow & 1) {
		return (buf * elem) % mod;
	}
	return buf;
}

//в x1 y1 результат суммирования, точки на скрученной кривой эдвардса
void Sum_Point(uint1024_t &x1, uint1024_t &y1, uint1024_t x2, uint1024_t y2, const Sequence &pset)
{
	uint1024_t P = pset.p;
	uint1024_t alpha = (x1 * y2) % P;
	uint1024_t beta = (y1 * x2) % P;
	uint1024_t gamma = (pset.d * ((alpha * beta) % P)) % P;
	uint1024_t x3 = (alpha + beta) % P;
	uint1024_t inv = inverse((1 + gamma) % P, P - 2, P);
	x3 = (x3 * inv) % P;
	uint1024_t y3 = ((y1 * y2) % P + (P - (pset.e * ((x1 * x2) % P) % P))) % P;
	inv = inverse((1 + (P - gamma)) % P, P - 2, P);
	y3 = (y3 * inv) % P;
	x1 = x3;
	y1 = y3;
	return;
}
//Q = kP; k == priv_key
void gen_pub_key(uint8_t* priv_key, uint8_t*  pub_key[], size_t size, const Sequence& pset)
{
	vector <bool> order = make_order(size, priv_key);

	int ord_size = order.size();
	uint1024_t u = pset.u;
	uint1024_t v = pset.v;
	
	for (int i = 0; i < ord_size; i++) {
		if (order[i]) {
			Sum_Point(u, v, u, v, pset);
			Sum_Point(u, v, pset.u, pset.v, pset);
		}
		else {
			Sum_Point(u, v, u, v, pset);
		}
	}
	
	for (int i = size - 1; i > -1; i--) {
		uint16_t tmp = 8 * (size - i - 1);
		pub_key[0][i] = (uint8_t)((u >> tmp) & 255);
		pub_key[1][i] = (uint8_t)((v >> tmp) & 255);
	}
	
	return;
}

//Q = kP k == priv_key
vector <uint1024_t> gen_pub_key(uint8_t* priv_key, size_t size, const Sequence& pset)
{
	vector <bool> order = make_order(size, priv_key);

	int ord_size = order.size();
	uint1024_t u = pset.u;
	uint1024_t v = pset.v;
	
	for (int i = 0; i < ord_size; i++) {
		if (order[i]) {
			Sum_Point(u, v, u, v, pset);
			Sum_Point(u, v, pset.u, pset.v, pset);
		}
		else {
			Sum_Point(u, v, u, v, pset);
		}
	}
	vector <uint1024_t> point= {u, v};
	return point;
}