//复杂声明解析器
//能够解析指针、数组、函数
//前置符号递归读取，后置符号循环读取
//识别模式：说明符 声明符 [;]

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 1000
#define BUFSIZE 100

typedef struct Type {
	int base;
	int count;
	struct Type *rely;
} Type;

enum {
	//type
	INT, CHAR, PTR, ARR, FUN,
	//keyword
	Int, Char,
	//other integer
	ID
};

Type *ty, *tyls;
char *p, *name = NULL, *tks;
int tki;

void next() {
	tks = ""; tki = -1;
	while(*p) {
		if((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_') {
			int len = 0; char *_p = p;
			while((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_') {
				len++; p++;
			}
			tks = (char*)malloc(sizeof(char) * (len+1));
			strncpy(tks, _p, len);
			tks[len] = '\0';
			if(!strcmp(tks, "int")) tki = Int;
			else if(!strcmp(tks, "char")) tki = Char;
			else tki = ID;
			return;
		} else if(*p >= '0' && *p <= '9') {
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
		else if(*p == ')') { tks = ")"; p++; return; }
		else if(*p == '*') { tks = "*"; p++; return; }
		else if(*p == '[') { tks = "["; p++; return; }
		else if(*p == '(') { tks = "("; p++; return; }
		else if(*p == ']') { tks = "]"; p++; return; }
		else if(*p == ',') { tks = ","; p++; return; }
		else if(*p == ';') { tks = ";"; p++; return; }
		else { //跳过不能识别的符号
			p++;
		}
	}
}

Type* deriv_type(int base, Type *rely, int count) { //类型生成
	if(rely == NULL) {
		if(base == INT || base == CHAR) {
			for(Type *i = tyls; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == NULL) return i;
			}
			ty -> base = base;
			ty -> rely = NULL;
			return ty++;
		}/* else if(base == STR) {
			for(Type *i = tyls; i < ty; i++) {
				if(i -> base == PTR
				&& i -> rely == deriv_type(CHAR, NULL)) return i;
			}
			Type *rely = deriv_type(CHAR, NULL);
			ty -> base = PTR;
			ty -> rely = rely;
			return ty++;
		}*/
	} else {
		if(base == PTR) {
			for(Type *i = tyls; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == rely) return i;
			}
			ty -> base = base;
			ty -> rely = rely;
			return ty++;
		} else if(base == ARR) {
			if(rely -> base == FUN) { printf("error!\n"); exit(-1); }
			for(Type *i = tyls; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == rely
				&& i -> count == count) return i;
			}
			ty -> base = base;
			ty -> rely = rely;
			ty -> count = count;
			return ty++;
		} else if(base == FUN) {
			if(rely -> base == FUN || rely -> base == ARR) { printf("error!\n"); exit(-1); }
			for(Type *i = tyls; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == rely
				&& i -> count == count) return i;
			}
			ty -> base = base;
			ty -> rely = rely;
			ty -> count = count;
			return ty++;
		}
	}
	printf("error!\n"); exit(-1);
}

void print_type(Type *type) {
	if(type -> base == PTR) {
		printf("指向");
		print_type(type -> rely);
		printf("的指针");
	} else if(type -> base == ARR) {
		printf("拥有%d个类型为", type -> count);
		print_type(type -> rely);
		printf("的元素的数组");
	} else if(type -> base == FUN) {
		printf("需要%d个参数且返回值为", type -> count);
		print_type(type -> rely);
		printf("的函数");
	} else if(type -> base == INT) {
		printf("整型");
	} else if(type -> base == CHAR) {
		printf("字符型");
	}
}

Type* specifier() {
	if(tki == Int) {
		next();
		return deriv_type(INT, NULL, 0);
	} else if(tki == Char) {
		next();
		return deriv_type(CHAR, NULL, 0);
	} else { printf("error!\n"); exit(-1); }
}

int lev(char *opr) {
	char *oprs[] = {
		")",
		"", "*",
		"", "[", "("
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

Type* declarator(Type *type); //前置声明
int* complex(char *last_opr, int *cpx) { //复杂类型分析
	//前置符号
	if(!strcmp(tks, "*")) { //指针
		next();
		cpx = complex("*", cpx);
		cpx++;
		*cpx++ = PTR;
	} else if(!strcmp(tks, "(")) { //括号
		next();
		cpx = complex(")", cpx);
		if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); } //"("无法匹配到")"
		next();
	} else if(tki == ID) {
		if(!name) name = tks;
		//(id - 1) -> name = tks;
		next();
	} else { printf("error!\n"); exit(-1); }
	
	//next();
	//后置符号
	while(lev(tks) > lev(last_opr)) {
		if(!strcmp(tks, "[")) { //数组
			next();
			int count = 0;
			if(strcmp(tks, "]")) {
				if(tki == INT) count = atoi(tks);
				else { printf("error!\n"); exit(-1); }
				next();
			}
			*cpx++ = count;
			*cpx++ = ARR;
			if(strcmp(tks, "]")) { printf("error!\n"); exit(-1); }
		} else if(!strcmp(tks, "(")) { //函数
			int count = 0;
			next();
			if(strcmp(tks, ")")) {
				while(1) {
					count++;
					Type *type = specifier();
					declarator(type);
					if(!strcmp(tks, ")")) break;
					else if(!strcmp(tks, ",")) next();
					else { printf("error!\n"); exit(-1); }
				}
			}
			*cpx++ = count;
			*cpx++ = FUN;
		} else { printf("error!\n"); exit(-1); }
		next();
	}
	return cpx; //update cpx
}

Type* declarator(Type *type) {
	//if(strcmp(tks, "*") && strcmp(tks, "(") && tki != ID) { printf("error!\n"); exit(-1); }
	//Id *this_id = id++;
	int cpxsk[BUFSIZE]; //复杂类型栈
	int *cpx = cpxsk; //复杂类型栈栈顶指针
	cpx = complex("", cpx);
	while(cpx > cpxsk) {
		int base = *--cpx;
		int count = *--cpx;
		type = deriv_type(base, type, count);
	}
	//setid(this_id, type);
	return type;
}

int main(int argc, char *argv[]) {
	if(argc != 2) { printf("error!\n"); exit(-1); }
	tyls = ty = (Type*)malloc(MAXSIZE * sizeof(Type));
	p = argv[1];
	next();
	Type *type = specifier();
	type = declarator(type);
	if(strcmp(tks, ";") && strcmp(tks, "")) { printf("error!\n"); exit(-1); }
	printf("%s为", name);
	print_type(type);
	printf("\n");
	return 0;
}
