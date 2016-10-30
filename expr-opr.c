//表达式计算器
//用函数递归自动形成的堆栈替换手动堆栈sp

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 1000

enum { INT };

int tki;
char *tks, *p;

void next() {
	tks = ""; tki = -1;
	while(*p) {
		if(*p >= '0' && *p <= '9') {
			int len = 0; char *_p = p;
			while(*p >= '0' && *p <= '9') {
				len++; p++;
			}
			tki = INT;
			tks = (char*)malloc(sizeof(char) * (len+1));
			strncpy(tks, _p, len);
			tks[len] = '\0';
			return;
		}
		else if(*p == '+') { tks = "+"; p++; return; }
		else if(*p == '-') { tks = "-"; p++; return; }
		else if(*p == '*') { tks = "*"; p++; return; }
		else if(*p == '/') { tks = "/"; p++; return; }
		else if(*p == '(') { tks = "("; p++; return; }
		else if(*p == ')') { tks = ")"; p++; return; }
		else { //跳过不能识别的符号
			p++;
		}
	}
}

int lev(char *opr) {
	char *oprs[] = {
		")",
		"", "+", "-",
		"", "*", "/"
	};
	int lev = 1;
	for(int i = 0; i < sizeof(oprs) / sizeof(*oprs); i++) {
		if(!strcmp(oprs[i], opr)) {
			return lev;
		} else if(!strcmp(oprs[i], "")) {
			lev++;
		}
	}
	return 0; //其他符号
}

int expr(char *last_opr) { //1 + 2 ^ 3 * 4 == (1 + (2 ^ (3) * (4)))
	int a;
	if(tki == INT) {
		a = atoi(tks);
	} else if(!strcmp(tks, "(")) {
		next();
		a = expr(")");
		if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); } //"("无法匹配到")"
	} else { printf("error!\n"); exit(-1); }
	
	next();
	while(lev(tks) > lev(last_opr)) {
		char *opr = tks;
		next();
		int b = expr(opr);
		if (!strcmp(opr, "+")) a += b;
		else if(!strcmp(opr, "-")) a -= b;
		else if(!strcmp(opr, "*")) a *= b;
		else if(!strcmp(opr, "/")) a /= b;
		else { printf("error!\n"); exit(-1); }
	}
	return a;
}

int main(int argc, char *argv[]) {
	if(argc != 2) { printf("error!\n"); exit(-1); }
	p = argv[1];
	next();
	int r = expr("");
	if(strcmp(tks, ";") && strcmp(tks, "")) { printf("error!\n"); exit(-1); }
	printf("%d\n", r);
	return 0;
}
