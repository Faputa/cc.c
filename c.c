/*
 * 设计摘要:
 * 1. tk——>tki，表示符号中可以用int存放的信息；tkv——>tks，表示符号中可以用string存放的信息；tki和tks可以一起使用
 * 2. 符号栈存放名字、字符串、作用域标记
 * 3. 符号栈存放字符串时：name指向字符串，is_str为1;else为0
 * 4. if：符号栈存放存放标记：全局符号标记：mark==GLO；函数符号标记：mark==FUN；局部符号标记：mark==LOC；else：mark==-1
 * 5. 必须为全局变量设置data段的理由：无法确定全局变量关于bp的偏移量
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 10000

/* type */
typedef struct Type {
	int base;
	struct Type *rely;
} Type;

typedef struct {
	Type *type;
	int is_var; //是否是变量
	int is_const; //lvalue一定const，const不一定lvalue，lvalue不能被寻址，const不一定能被寻址，字符串具有const属性但不具有lvalue属性
	//int is_lvalue;
} Er;

typedef struct {
	char *name;
	Type *type;
	int offset;
	int class;
	int csmk; //class mark
	int is_str;
} Id;

/* global */
Type *ty, *tyls;
Id *id, *idls;
char *p, *d, *tks;
int *e, *emit, tki;

/* integer */
enum {
	//keyword
	Int, Char,
	If, Else, While, Return,
	Main, Printf,
	//type
	INT, CHAR, STR, FUN,
	//opcode
	PUSHI, PUSHC,
	//other integer
	ID, GLO, LOC, ARG,
};

/* function declare */
void next();
void setid(Type *type);
Id* getid();
void infunc();
void outfunc();
void inlocal();
void outlocal();
Type* specifier();
Type* declarator(Type *base);
void declare(int env);
void stmt();
int lev(char *opr);
Er expr(char *last_opr);

/* function define */
void next() {//printf("%d\n",*p);
	char *keyword[] = {
		"int","char",
		"if","else","while","return",
		"main","printf"
	};
	char *point[] = {
		"==","&&","||","+=","-=","*=","/=",
		"+","-","*","/","%","=","!","^","&","|","(",")","{","}","[","]","?",":",",",";"
	};
	char *trans[] = {
		"\\n","\n",
		"\\\\","\\",
		"\\t","\t",
		"\\\"","\"",
		"\\0","\0"
	};

	while(*p != '\0') {
		tks = ""; tki = -1;
		if(!strncmp(p, "//", 2) || *p == '#') { while(*p != '\n') p++; p++; }
		else if(!strncmp(p, "/*", 2)) { while(strncmp(p, "*/", 2)) p++; p+=2; }
		else if((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_') {
			int len = 0; char *_p = p;
			while((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_') {
				len++; p++;
			}
			for(int i = 0; i < sizeof(keyword) / sizeof(*keyword); i++) {
				if(strlen(keyword[i]) == len && !strncmp(keyword[i], _p, len)) {
					tki = i;
					return;
				}
			}
			tki = ID;
			tks = (char*)malloc(sizeof(char) * (len+1));
			strncpy(tks, _p, len);
			tks[len] = '\0';
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
		} else if(*p == '"') {
			tki = STR;
			int len = 0; char *_p = ++p;
			while(*p != '"' && *p != '\0') {
				if(*p++ =='\\') p++;
				len++;
			}
			if(*p != '\0') p++;//printf("%d\n",len);
			tks = (char*)malloc(sizeof(char) * (len+1));
			int i = 0; while(*_p != '"') {
				if(*_p == '\\') {
					for(int j = 0; j < sizeof(trans) / sizeof(*trans); j+=2) {
						if(!strncmp(trans[j], _p, strlen(trans[j]))) {
							tks[i] = *trans[j+1];
							_p += strlen(trans[j]);
							break;
						}
					}
				} else {
					tks[i] = *_p++;
				}
				i++;
			}
			tks[i] = '\0';
			return;
		} else if(*p == '\'') {
			tki = CHAR;
			if(*++p != '\\') {
				tks = (char*)malloc(sizeof(char) * 2);
				tks[0] = *p; tks[1] = '\0';
			} else {
				for(int i = 0; i < sizeof(trans) / sizeof(*trans); i+=2) {
					if(!strncmp(trans[i], p, strlen(trans[i]))) {
						tks = trans[i+1];
						break;
					}
				}
			}
			while(*p != '\'' && *p != '\0') p++;
			if(*p != '\0') p++;
		} else {
			for(int i = 0; i < sizeof(point) / sizeof(*point); i++) {
				if(!strncmp(point[i], p, strlen(point[i]))) {
					tks = point[i];
					p += strlen(point[i]);
					return;
				}
			}
			p++;
		}
	}
}

void setid(Type *type) {
	for(Id *i = id++; i -> csmk == -1; i--) {
		if(!strcmp(tks, i -> name) && !(i -> is_str)) { printf("error!\n"); exit(-1); }
	}
	
	id -> name = tks;
	id -> type = type;
	id -> csmk = -1;
	id -> is_str = 0;
	
	if((id-1) -> csmk == LOC
	|| (id-1) -> csmk == FUN
	|| (id-1) -> csmk == GLO)
		id -> class = (id-1) -> csmk;
	else 
		id -> class = (id-1) -> class;
	
	Id *last_id = id - 1; while(last_id -> csmk == LOC) { last_id--; }
	if(last_id -> csmk == GLO && last_id -> csmk == FUN) {
		id -> offset = 0;
	} else {
		id -> offset = last_id -> offset;
		if(last_id -> type == INT) id -> offset += sizeof(int);
		else if(last_id -> type == CHAR) id -> offset += sizeof(char);
		else if(last_id -> type == FUN) id -> offset += 0;
		else if(last_id -> type == PTR) id -> offset += sizeof(void*);
		else if(last_id -> is_str) id -> offset += strlen(last_id -> name) + 1;
	}
}

Id* sgetstr() {
	Id *i = idls + 1;
	while(i -> csmk == -1) {
		if(!strcmp(tks, i -> name) && i -> is_str) return i;
		i++;
	}
	
	i = ++id;
	i -> name = tks;
	i -> type = deriv_type(PTR, NULL);
	i -> is_str = 1;
	i -> csmk = -1;
	
	while(i -> class != GLO) {
		Id _i = *i;
		*i = *(i-1);
		*i-- = _i;
		
		if((i-1) -> csmk == LOC
		|| (i-1) -> csmk == FUN
		|| (i-1) -> csmk == GLO)
			i -> class = (i-1) -> csmk;
		else
			i -> class = (i-1) -> class;
	}
	
	if((i-1) -> csmk == GLO) {
		i -> offset = 0;
	} else {
		i -> offset = (i-1) -> offset;
		if((i-1) -> type == INT) id -> offset += sizeof(int);
		else if((i-1) -> type == CHAR) id -> offset += sizeof(char);
		else if((i-1) -> type == FUN) id -> offset += 0;
		else if((i-1) -> type == PTR) id -> offset += sizeof(void*);
		else if((i-1) -> is_str) id -> offset += strlen((i-1) -> name) + 1;
	}
	
	return i;
}

Id* getid() {
	for(Id *i = id; i != idls; i--) {
		if(i -> csmk == -1 && !(i -> is_str) && !strcmp(tks, i -> name)) return i;
	}
	printf("error!\n"); exit(-1);
}

void infunc() {
	(++id) -> csmk = FUN;
}
void outfunc() {
	while(id -> csmk != FUN) id--;
	id--;
}

void inlocal() {
	(++id) -> csmk = LOC;
}

void outlocal() {
	while(id -> csmk != LOC) {
		*e++ = POP;
		id--;
	}
	id--;
}

int lev(char *opr) { //优先级越高lev越大，其他符号lev为0
	char *oprs[] = {
		"", ")", //规定"("不是运算符
		"", "+", "-",
		"", "*", "/"
		"", "&", "ptr"
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

Er expr(char *last_opr) { //1 + 2 ^ 3 * 4 == (1 + (2 ^ (3) * (4)))
	Er er = {NULL, 0, 0};
	if(tki == INT) {
		er.is_const = 1;
		er.type = deriv_type(INT, NULL);
		*e++ = RTI;
		*e++ = atoi(tks);
	} else if(tki == CHAR) {
		er.is_const = 1;
		er.type = deriv_type(CHAR, NULL);
		*e++ = RTC;
		*e++ = tki[0];
	} else if(tki == STR) {
		//type = get_lit_type(PTR);
		//setid(type);
		//strcpy(d + getid() -> offset, tks);
		Id *this_id = sgetstr();
		er.type = this_id -> type;
		er.is_const = 1;
		strcpy(d + this_id -> offset, tks);
		*e++ = RTI;
		*e++ = getid() -> offset;
	} else if(tki == ID) {
		Id *this_id = getid();
		er.type = this_id -> type;
		er.is_const = 0;
		if(er.type -> base == FUN) {
			er.type = er.type -> rely;
			int callee = this_id -> offset;
			next(); if(!strcmp(tks, "(")) { printf("error!\n"); exit(-1); }
			next();
			*e++ = PUSH;
			int *_e = e++; //push next ip
			*e++ = PUSH_BP;
			while(strcmp(tks, ")")) { //参数入栈
				Er er = expr("");
				if(er.type -> type != (++this_id) -> type) { printf("error!\n"); exit(-1); } //参数类型检查
				if(er.type -> base == INT || er.type == PTR) *e++ = PUSHI_RT;
				else if(er.type -> base == CHAR) *e++ = PUSHC_RT;
				else { printf("error!\n"); exit(-1); }
				next();
				if(!strcmp(tks, ",")) next();
			}
			*_e = e - emit + 2; //set next ip
			*e++ = CALL; //jmp
			*e++ = callee;
		} else if(er.type -> base == INT) {
			*e++ = RTIDI;
			*e++ = this_id -> offset;
			*e++ = this_id -> class == GLO ? : ; //根据偏移量获取真实地址
			er.is_var = 1;
		} else if(er.type -> base == CHAR) {
			*e++ = RTIDC;
			*e++ = this_id -> offset;
			*e++ = this_id -> class == GLO ? : ; //根据偏移量获取真实地址
			er.is_var = 1;
		} else if(er.type -> base == PTR) {
			*e++ = RTIDI;
			*e++ = this_id -> offset;
			*e++ = this_id -> class == GLO ? : ; //根据偏移量获取真实地址
			er.is_var = 1;
		}
	} else if(!strcmp(tks, "(")) {
		next();
		er = expr(")");
	} else if(!strcmp(tks, "*")) {
		er = expr("ptr");
		if(er.type -> base == PTR) {
			er.type = er.type -> rely;
			er.is_const = 0;
			*e++ = //TODO
		} else { printf("error!\n"); exit(-1); }
	} else if(!strcmp(tks, "&")) {
		er = expr("&");
		if(!er.is_const) {
			er.type = deriv_type(PTR, er.type);
			er.is_const = 1;
			*e++ = //TODO
		} else { printf("error!\n"); exit(-1); }
	} else { printf("error!\n"); exit(-1); }
	
	next();
	while(lev(tks) > lev(last_opr)) {
		*e++ = PUSHR;
		char *opr = tks;
		if(!strcmp(opr, ")")) { printf("error!\n"); exit(-1); } //此时last_opr为""，表示")"无法匹配到"("
		next();
		Er r = expr(opr); //此后tks一定会指向next_opr，用于下次循环
		if(r.type != er.type) { printf("error!\n"); exit(-1); }
		if(r.is_var && strcmp(tks, "=")) *e++ = ; //根据真实地址取值
		
		if (!strcmp(opr, "+")) *e++ = ADD;
		else if(!strcmp(opr, "*")) *e++ = MUL;
		else if(!strcmp(opr, "-")) *e++ = SUB;
		else if(!strcmp(opr, "/")) *e++ = DIV;
		else if(!strcmp(opr, "=")) {
			if(!er.is_const) *e++ = ASS;
		}
		er.is_const = 1; //计算过后改写const属性
	}
	return er;
}

Type* specifier() { //说明符
	if(tki != INT && tki != CHAR) return NULL;
/*	for(Type *i = tyls; i < ty; i++) {
		if(i -> base == tki
		&& i -> rely == NULL) return i;
	}
	ty -> base = tki;
	ty -> rely = NULL;
	return ty++;*/
	return deriv_type(tki, NULL);
}

Type* declarator(Type *base) { //声明符
	if(strcmp(tks, "*") && tki != ID) return NULL;
	Type *i = tyls;
	Type *type = base;
	while(!strcmp(tks, "*")) {
		while(i < ty) {
			if(i -> rely == type) {
				type = i;
				break;
			}
			i++;
		}
		if(i >= ty) {
			ty -> base = PTR;
			ty -> rely = type;
			type = ty++;
		}
	}
	if(tki != ID)) { printf("error!\n"); exit(-1); }
	setid(type);
	return type;
}

Type* deriv_type(int base, Type *rely) {
	if(rely == NULL) {
		if(base == STR) {
			for(Type *i = tyls; i < ty; i++) {
				if(i -> base == PTR
				&& i -> rely == deriv_type(CHAR, NULL)) return i;
			}
			Type *rely = deriv_type(CHAR, NULL);
			ty -> base = PTR;
			ty -> rely = rely;
			return ty++;
		} else if(base == INT || base == CHAR) {
			for(Type *i = tyls; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == NULL) return i;
			}
			ty -> base = base;
			ty -> rely = NULL;
			return ty++;
		}
	} else {
		if(base == PTR || base == FUN) {
			for(Type *i = tyls; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == rely) return i;
			}
			ty -> base = base;
			ty -> rely = rely;
			return ty++;
		}
	}
	printf("error!\n"); exit(-1);
}
/*
void eval() {
	
}
*/
void declare(int env) {
	if(env == GLO) {
		Type *spec_type = specifier();
		if(!spec_type) { printf("error!\n"); exit(-1); }
		next();
		Type *type = declarator(spec_type);
		if(!type) { printf("error!\n"); exit(-1); }
		next();
		if(!strcmp(tks, "(")) {
			//id -> type -> base = FUN;
			//id -> type -> rely = type;
			id -> type = deriv_type(FUN, type);
			id -> offset = e - emit;
			infunc();
			*e++ = MOV_SP_BP;
			declare(ARG);
			*e++ = SUB_BP;
			*e++ = id -> offset;
			if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); }
			next();
			if(strcmp(tks, "{")) { printf("error!\n"); exit(-1); }
			stmt();
			if(strcmp(tks, "}")) { printf("error!\n"); exit(-1); }
			*e++ = MOV_BP_SP;
			*e++ = POP_BP;
			*e++ = POP_IP;
			outfunc();
		} else {
			if(!strcmp(tks, "=")) {
				next();
				if(type == deriv_type(INT, NULL)) {
					if(tki != INT) { printf("error!\n"); exit(-1); }
					*(int*)(d + id -> offset) = atoi(tki);
				} else if (type == deriv_type(CHAR, NULL)) {
					if(tki != CHAR) { printf("error!\n"); exit(-1); }
					*(d + id -> offset) = tki[0];
				} else { printf("error!\n"); exit(-1); }
				next();
			} else {
				if(type == deriv_type(INT, NULL)) *(int*)(d + id -> offset) = 0;
				else if(type == deriv_type(CHAR, NULL)) *(d + id -> offset) = '\0';
			}
			while(!strcmp(tks, ",")) {
				next();
				type = declarator(spec_type);
				if(!type) { printf("error!\n"); exit(-1); }
				next();
				if(!strcmp(tks, "=")) {
					next();
					if(type == deriv_type(INT, NULL)) {
						if(tki != INT) { printf("error!\n"); exit(-1); }
						*(int*)(d + id -> offset) = atoi(tki);
					} else if (type == deriv_type(CHAR, NULL)) {
						if(tki != CHAR) { printf("error!\n"); exit(-1); }
						*(d + id -> offset) = tki[0];
					} else { printf("error!\n"); exit(-1); }
					next();
				} else {
					if(type == deriv_type(INT, NULL)) *(int*)(d + id -> offset) = 0;
					else if(type == deriv_type(CHAR, NULL)) *(d + id -> offset) = '\0';
					*e++ = 0;
				}
			}
			if(strcmp(tks, ";")) { printf("error!\n"); exit(-1); }
		}
	} else if(env == LOC) {
		Type *spec_type = specifier();
		if(!spec_type) { printf("error!\n"); exit(-1); }
		next();
		Type *type;
		while(type = declarator(spec_type)) {
			next();
			if(!strcmp(tks, "=")) {
				next();
				if(type != expr("")) { printf("error!\n"); exit(-1); }
				if(type -> base == INT || type -> base == PTR) *e++ = PUSHI_RT;
				else if (type -> base == CHAR) *e++ = PUSHC_RT;
				else { printf("error!\n"); exit(-1); }
				next();
			}
			if(!strcmp(tks, ";")) break;
			else if(!strcmp(tks, ",")) next();
			else { printf("error!\n"); exit(-1); }
		}
	} else if(env == ARG) {
		next();
		Type *spec_type, *type;
		while(spec_type = specifier()) {
			next();
			type = declarator(spec_type);
			if(!type) { printf("error!\n"); exit(-1); }/* 参数由调用者压入栈
			if(type -> base == INT || type -> base == PTR) {
				next(); if(tki != ID) { printf("error!\n"); exit(-1); }
				setid(INT);
				*e++ = PUSHI;
				*e++ = 0;
			} else if(tki == CHAR) {
				next(); if(tki != ID) { printf("error!\n"); exit(-1); }
				setid(CHAR);
				*e++ = PUSHC;
				*e++ = 0;
			}*/
			next();
			if(!strcmp(tks, ",")) next();
		}
	}
}

void stmt() {
	if(!strcmp(tks, "{")) {
		next();
		while(strcmp(tks, "}")) {
			if(tki == INT && tki == CHAR) declare(LOC);
			else stmt();
			next();
		}
	} else if(tki == IF) {
		next(); if(strcmp(tks, "(")) { printf("error!\n"); exit(-1); }
		next(); expr("");
		next(); if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); }
		next();
		*e++ = CJMP;
		int *_e1 = e++;
		if(!strcmp(tks, "{")) {
			inlocal();
			stmt();
			outlocal();
		} else {
			stmt();
		}
		char *_p = p;
		next();
		if(tki == ELSE) {
			*e++ = JMP;
			int *_e2 = e++;
			*_e1 = e - emit;
			if(!strcmp(tks, "{")) {
				inlocal();
				stmt();
				outlocal();
			} else {
				stmt();
			}
			*_e2 = e - emit;
		} else {
			*_e1 = e - emit;
			p = _p;
		}
	} else if(tki == WHILE) {
		int *_e1 = e;
		next(); if(strcmp(tks, "(")) { printf("error!\n"); exit(-1); }
		next(); expr("");
		next(); if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); }
		next();
		*e++ = CJMP;
		int *_e2 = e++;
		if(!strcmp(tks, "{")) {
			inlocal();
			stmt();
			outlocal();
		} else {
			stmt();
		}
		*e++ = JMP;
		*e++ = _e1 - emit;
		*_e2 = e - emit;
	} else if(tki == RETURN) {
		next(); expr("");
	} else {
		it(strcmp(tks, ";")) expr("");
	}
}

int main(int argc, char *argv[]) {
	FILE *fp = fopen(argv[1], "r");

	char* src = p = (char*)malloc(MAXSIZE * sizeof(char)); //源码流
	idls = id = (Id*)malloc(MAXSIZE * sizeof(Id)); //符号栈
	emit = e = (char*)malloc(MAXSIZE * sizeof(char)); //代码段
	char *data = d = (char*)malloc(MAXSIZE * sizeof(char)); //数据段
	
	fread(p, sizeof(char), MAXSIZE, fp);//printf("%s",p);
	fclose(fp);
	(id++) -> csmk = GLO;
	while(*p != '\0') {
		next();
		declare(GLO);
	}
	
	//print
	
	//run..

	return 0;
}
