all: genkey conv sign verify

genkey : genpkey.cpp
	g++ -Wall -O2 genpkey.cpp -o genkey

conv : convpkey.cpp
	g++ -Wall -O2 convpkey.cpp -o conv

sign : sign.cpp
	g++ -O2 sign.cpp -o sign -Wall

verify : verify.cpp
	g++ -O2 verify.cpp -o verify

 
