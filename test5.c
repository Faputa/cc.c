#include "test.h"

int f(int a[], int b) {
	return a[0] + b;
}
int main() {
	int a[10][10] = {{},{100,200,300}};
	print(a[1][2]);
	endl();
	int *b = &a[1][1];
	print(*b);
	endl();
	int (*c)[10] = &a[1];
	print((*c)[0]);
	endl();
	print(*(*c + 0));
	endl();
	int (*d)(int a[], int b) = &f;
	print((*d)(a[1], 25));
	endl();
	print(d(a[1], 25));
}