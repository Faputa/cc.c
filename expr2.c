//表达式计算器
//递归下降分析
//遇到终结符必须主动移动tk
//!!!error!!!

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 1000

enum { INT };

int tki, *sp;
char *tks, *p;

void expr(void);

void next(void) {
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

void atom(void) { //atom -> int | "(" expr ")"
	if(tki == INT) {
		*sp = atoi(tks);
		next();
	} else if(!strcmp(tks, "(")) {
		next();
		expr();
		if(!strcmp(tks, ")")) next(); else { printf("error!\n"); exit(-1); } //"("无法匹配到")"
	} else if(!strcmp(tks, "-")) {
		next();
		atom();
		*sp = -*sp;
	} else { printf("error!\n"); exit(-1); }
}

void muldiv(void) { //muldiv -> atom ["*" muldiv | "/" muldiv]
	atom();
	if(!strcmp(tks, "*")) {
		sp++;
		next();
		muldiv();
		int opr2 = *sp;
		int opr1 = *--sp;
		*sp = opr1 * opr2;
	} else if(!strcmp(tks, "/")) {
		sp++;
		next();
		muldiv();
		int opr2 = *sp;
		int opr1 = *--sp;
		*sp = opr1 / opr2;
	}
}

void addsub(void) { //addsub -> muldiv ["+" addsub | "-" addsub]
	muldiv();
	if(!strcmp(tks, "+")) {
		sp++;
		next();
		addsub();
		int opr2 = *sp;
		int opr1 = *--sp;
		*sp = opr1 + opr2;
	} else if(!strcmp(tks, "-")) {
		sp++;
		next();
		addsub();
		int opr2 = *sp;
		int opr1 = *--sp;
		*sp = opr1 - opr2;
	}
}

void expr(void) { //expr -> addsub
	addsub();
}

int main(int argc, char *argv[]) {
	if(argc != 2) { printf("error!\n"); exit(-1); }
	sp = (int*)malloc(MAXSIZE * sizeof(int));
	p = argv[1];
	next();
	expr();
	if(strcmp(tks, ";") && strcmp(tks, "")) { printf("error!\n"); exit(-1); }
	printf("%d\n", *sp);
	return 0;
}
