
#include "transforms.h"
#include <vector>
#include <string>

using std::vector;
using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::cerr;

void make_digit(uint8_t* tmp, uint64_t digit)
{
	for (int i = 0; i < 64; i++)
		tmp[i] = 0;
	tmp[0] = digit & 255;
	tmp[1] = (digit >> 8) & 255;
}


void update(context &ctx)
{
	uint8_t tmp[64] = {0};
	gN(ctx.hash, ctx.data, ctx.N);
	make_digit(tmp, 8 * ctx.data_size);
	sum_512(ctx.N, tmp);
	sum_512(ctx.check_sum, ctx.data);
	return;
}
 
void hash(context &ctx, char* arg)
{
	
	uint8_t N0[64] = {0}; 
	FILE* f = fopen(arg, "rb");
	if (f == NULL) {
		string err = "Cannot open file";
		throw err; 
	}
	ctx.data_size = data_read(f, ctx);
	while (ctx.data_size >= 0) {
		if (ctx.data_size < 64) {
			add(ctx.data, ctx.data_size);
			update(ctx);
			gN(ctx.hash, ctx.N, N0);
			break;
		}
		else if (ctx.data_size == 64){
			update(ctx);
			ctx.data_size = data_read(f, ctx);
		}
	} 
	fclose(f);
	gN(ctx.hash, ctx.check_sum, N0);
}

int main(int argc, char* argv[])
{
	context ctx(true);
	try {
		if (argc != 2) {
			string err = "Wrong number of parametrs";
			throw err;
		}
		hash(ctx, argv[1]);
	}
	catch (string err) {
		cerr << err << endl;
		return 1;
	}
	catch (...){
		cerr << "Unexpected error" << endl;
		return 1;
	}
	for (int i = 0; i < 32; i++) {
		std::swap(ctx.hash[i], ctx.hash[63 - i]);
	}
	//for (int i = 0; i < 32; i++)
	//	printf("%02x", ctx.hash[i]);
	//cout << endl;
	FILE* f = fopen("hash256", "wb");
	fwrite(ctx.hash, 1, 32, f);
	fclose(f);
	return 0;
}