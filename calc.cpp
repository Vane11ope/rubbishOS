#include <bits/stdc++.h>
using namespace std;

int main() {
	unsigned short someHex, otherHex;
	someHex = 0x1000;
	otherHex = someHex^0xFFFF;
	cerr << someHex << endl;
	someHex = ~someHex;
	printf("%04X\n", someHex);
	printf("%04X", otherHex);
	return 0;
}
