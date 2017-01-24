//表达式计算器字节码版

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 1000

enum {
	INT,
	PUSH ,POP, SET, ADD, SUB, MUL, DIV
};

int tki, *e, *emit;
char *tks, *p;

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

int lev(char *opr) {
	char *oprs[] = {
		")",
		"", "+", "-",
		"", "*", "/"
	};
	int lev = 1;
	for(int i = 0; i < sizeof(oprs) / sizeof(*oprs); i++) {
		if(!strcmp(oprs[i], opr)) return lev;
		else if(!strcmp(oprs[i], "")) lev++;
	}
	return 0; //其他符号
}

void expr(char *last_opr) { //1 + 2 ^ 3 * 4 == (1 + (2 ^ (3) * (4)))
	if(tki == INT) {
		//*sp = atoi(tks);
		*e++ = SET; *e++ = atoi(tks);
		next();
	} else if(!strcmp(tks, "(")) {
		next();
		expr(")");
		if(!strcmp(tks, ")")) next(); else { printf("error!\n"); exit(-1); } //"("无法匹配到")"
	} else if(!strcmp(tks, "-")) {
		next();
		*e++ = SET; *e++ = 0;
		*e++ = PUSH;
		expr("-");
		*e++ = SUB;
	} else { printf("error!\n"); exit(-1); }
	
	while(lev(tks) > lev(last_opr)) {
		char *opr = tks;
		//sp++;
		*e++ = PUSH;
		next();
		expr(opr);
		//int opr2 = *sp;
		//int opr1 = *--sp;
		if (!strcmp(opr, "+")) *e++ = ADD;//*sp = opr1 + opr2;
		else if(!strcmp(opr, "-")) *e++ = SUB;//*sp = opr1 - opr2;
		else if(!strcmp(opr, "*")) *e++ = MUL;//*sp = opr1 * opr2;
		else if(!strcmp(opr, "/")) *e++ = DIV;//*sp = opr1 / opr2;
		else { printf("error!\n"); exit(-1); }
	}
}

int main(int argc, char *argv[]) {
	if(argc != 2) { printf("error!\n"); exit(-1); }
	emit = e = (int*)malloc(MAXSIZE * sizeof(int));
	p = argv[1];
	next();
	expr("");
	if(strcmp(tks, ";") && strcmp(tks, "")) { printf("error!\n"); exit(-1); }
	
	int *sp, *ip = emit;
	sp = (int*)malloc(MAXSIZE * sizeof(int));
	while(ip < e) {
		if(*ip == PUSH) sp++;
		else if(*ip == POP) sp--;
		else if(*ip == SET) *sp = *++ip;
		else if(*ip == ADD) {
			int opr2 = *sp;
			int opr1 = *--sp;
			*sp = opr1 + opr2;
		} else if(*ip == SUB) {
			int opr2 = *sp;
			int opr1 = *--sp;
			*sp = opr1 - opr2;
		} else if(*ip == MUL) {
			int opr2 = *sp;
			int opr1 = *--sp;
			*sp = opr1 * opr2;
		} else if(*ip == DIV) {
			int opr2 = *sp;
			int opr1 = *--sp;
			*sp = opr1 / opr2;
		}
		ip++;
	}
	printf("%d\n", *sp);
	return 0;
}
