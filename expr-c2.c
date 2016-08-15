//表达式计算器字节码版

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 1000

enum {
	//type
	INT,
	//opcode
	PUSH ,POP, SET, INC, DEC, JMP, JZ, MOV, ADD, SUB, MUL, DIV,
	//reg
	IP = 0, BP, SP, AX
};

int tki, *e, *emit;
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

void expr(char *last_opr) { //1 + 2 ^ 3 * 4 == (1 + (2 ^ (3) * (4)))
	if(tki == INT) {
		//*sp = atoi(tks);
		*e++ = SET;
		*e++ = AX;
		*e++ = atoi(tks);
	} else if(!strcmp(tks, "(")) {
		next();
		expr(")");
		if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); } //"("无法匹配到")"
	} else { printf("error!\n"); exit(-1); }
	
	next();
	while(lev(tks) > lev(last_opr)) {
		//sp++;
		*e++ = PUSH;
		*e++ = AX;
		char *opr = tks;
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
	
	int *store = (int*)malloc(MAXSIZE * sizeof(int));
	*(SP + (int**)store) = store + AX + 1; //sp = store + AX + 1;
	*(IP + (int**)store) = emit; //ip = emit;
	while(*(IP + (int**)store) < e) {
		int *ip = (*(IP + (int**)store))++;
		if(*ip == PUSH) {
			int opr = *(*(IP + (int**)store))++;
			*(*(SP + (int**)store))++ = *(store + opr);
		} else if(*ip == POP) {
			int opr = *(*(IP + (int**)store))++;
			*(store + opr) = *--*(SP + (int**)store);
		} else if(*ip == SET) {
			int opr1 = *(*(IP + (int**)store))++;
			int opr2 = *(*(IP + (int**)store))++;
			*(store + opr1) = opr2;
		} else if(*ip == INC) {
			int opr1 = *(*(IP + (int**)store))++;
			int opr2 = *(*(IP + (int**)store))++;
			*(store + opr1) += opr2;
		} else if(*ip == DEC) {
			int opr1 = *(*(IP + (int**)store))++;
			int opr2 = *(*(IP + (int**)store))++;
			*(store + opr1) -= opr2;
		} else if(*ip == JMP) {
			int opr = *(*(IP + (int**)store))++;
			*(IP + (int**)store) = emit + opr;
		} else if(*ip == JZ) {
			int opr = *(*(IP + (int**)store))++;
			if(!*(store + AX)) *(IP + (int**)store) = emit + opr;
		} else if(*ip == MOV) {
			int opr1 = *(*(IP + (int**)store))++;
			int opr2 = *(*(IP + (int**)store))++;
			*(store + opr1) = *(store + opr2);
		} else if(*ip == ADD) {
			int opr1 = *(store + AX);
			int opr2 = *--*(SP + (int**)store);
			*(store + AX) = opr1 + opr2;
		} else if(*ip == SUB) {
			int opr1 = *(store + AX);
			int opr2 = *--*(SP + (int**)store);
			*(store + AX) = opr2 - opr1;
		} else if(*ip == MUL) {
			int opr1 = *(store + AX);
			int opr2 = *--*(SP + (int**)store);
			*(store + AX) = opr1 * opr2;
		} else if(*ip == DIV) {
			int opr1 = *(store + AX);
			int opr2 = *--*(SP + (int**)store);
			*(store + AX) = opr2 / opr1;
		}
	}
	printf("%d\n", *(store + AX));
	return 0;
}
