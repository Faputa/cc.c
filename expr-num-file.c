/*
 * 表达式计算器
 * 浮点型
 * 从文件读取
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 1000

enum { NUM };

int tki;
float *sp;
char *tks, *p;

void next() {//printf("aaa-%c-aaa\n",*p);
	tks = ""; tki = -1;
	while(*p) {
		if(*p >= '0' && *p <= '9') {
			int len = 0; char *_p = p;
			while(*p >= '0' && *p <= '9') {
				len++; p++;
				if(*p == '.') {
					len++; p++;
					while(*p >= '0' && *p <= '9') {
						len++; p++;
					}
					break;
				}
			}
			tki = NUM;
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
	if(tki == NUM) {
		*sp = atof(tks);
	} else if(!strcmp(tks, "(")) {
		next();
		expr(")");
		if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); } //"("无法匹配到")"
	} else { printf("error!\n"); exit(-1); }
	
	next();
	while(lev(tks) > lev(last_opr)) {
		char *opr = tks;
		sp++;
		next();
		expr(opr);
		float opr2 = *sp;
		float opr1 = *--sp;
		if (!strcmp(opr, "+")) *sp = opr1 + opr2;
		else if(!strcmp(opr, "-")) *sp = opr1 - opr2;
		else if(!strcmp(opr, "*")) *sp = opr1 * opr2;
		else if(!strcmp(opr, "/")) *sp = opr1 / opr2;
		else { printf("error!\n"); exit(-1); }
	}
}

void print_float(float f) {
	char bf[40], *i, *j;
	sprintf(bf, "%f", f);
	i = bf;
	while(*i != '.') i++;
	j = i + 1;
	while(*j) {
		if(*j != '0') i = j + 1;
		j++;
	}
	*i = '\0';
	printf("%s", bf);
}

int main(int argc, char *argv[]) {
	FILE *fp;
	if(argc != 2) { printf("error!\n"); exit(-1); }
	if(!(fp = fopen(argv[1], "r"))) { printf("error!\n"); exit(-1); }
	p = (char*)malloc(MAXSIZE * sizeof(char));
	{ int i = fread(p, sizeof(char), MAXSIZE, fp); p[i] = '\0'; }
	fclose(fp);
	
	sp = (float*)malloc(MAXSIZE * sizeof(float));
	next();
	expr("");
	if(strcmp(tks, ";") && strcmp(tks, "")) { printf("error!\n"); exit(-1); }
	//printf("%f\n", *sp);
	print_float(*sp);
	return 0;
}
