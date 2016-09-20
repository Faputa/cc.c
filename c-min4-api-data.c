/*
 * ���ժҪ:
 * 1.tki����ʾ�����п�����int��ŵ���Ϣ��tks����ʾ�����п�����string��ŵ���Ϣ��tki��tks����һ��ʹ��
 * 2.����ջ������֡��ַ�������������
 * 3.����ջ����ַ���ʱ��nameָ���ַ�����typeΪSTR
 * 4.����ջ���ȫ�ַ��ű�ǣ�csmk==GLO���������ű�ǣ�csmk==FUN���ֲ����ű�ǣ�csmk==LOC
 * 5.����Ϊȫ�ֱ�������data�ε����ɣ��޷�ȷ��ȫ�ֱ�������bp��ƫ����
 * 6.���ݶΡ���ջ�Ρ�����Ρ��Ĵ�����������Ϊsizeof(int)
 * 7.�������÷�����
 * 8.�����˽���api
 * 9.����ͳһ��data��
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 10000

/* type */
typedef struct {
	char *name;
	int type;
	int offset;
	int class;
	int csmk; //class mark
} Id;

/* global */
Id *id, *idls;
char *p, *tks;
int *e, *emit, *data, tki;

/* integer */
enum {
	//keyword
	Int, If, Else, While, Do, For, Return,
	//type
	INT, FUN, API,
	//other integer
	ID, GLO, LOC,// ARG,
	//opcode
	PUSH, POP, SET, INC, DEC, JMP, JZ, MOV, ADD, SUB, MUL, DIV, MOD, ASS, EQ, GT, LT, AND, OR, NOT, AG, AL, VAL, EXIT,
	PRINT, ENDL, SPACE, SCAN,
	//reg
	IP = 0, BP, SP, AX
};

/* function declare */
void next();
void setid(char *tks, int type);
Id* getid(char *tks);
//void infunc();
//void outfunc();
void inblock();
void outblock();
void declare(int env);
void stmt();
int lev(char *opr);
int expr(char *last_opr);

/* function define */
void next() {
	char *keyword[] = {
		"int", "if", "else", "while", "do", "for", "return"
	};
	char *point[] = {
		"==", ">=", "<=", "&&", "||", "+=", "-=", "*=", "/=",
		"+", "-", "*", "/", "%", "=", ">", "<", "!", "^", "&", "|", "(", ")", "{", "}", "[", "]", "?", ":", ",", ";"
	};
	tks = ""; tki = -1;
	while(*p) {
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

void setid(char *tks, int type) {
	for(Id *i = id++; i -> csmk == ID; i--) {
		if(!strcmp(tks, i -> name)) { printf("error!\n"); exit(-1); }
	}
	
	id -> name = tks;
	id -> type = type;
	id -> csmk = ID;
	
	Id *last_id = id - 1;
	if(last_id -> csmk != ID) {
		id -> class = last_id -> csmk;
	} else {
		id -> class = last_id -> class;
	}
	
	while(last_id -> csmk == LOC || last_id -> type == FUN || last_id -> type == API) last_id--; //����֤����type����FUN��APIʱcsmkһ������ID
	if(id -> class == GLO) {
		if(last_id -> csmk == GLO) {
			if(type == INT) id -> offset = MAXSIZE - 1;
		} else {
			if(type == INT) id -> offset = last_id -> offset - 1;
		}
	} else {
		if(last_id -> csmk == FUN || last_id -> offset < 0) { //offsetС��0Ϊ�������Ҵ�ʱcsmk������ΪLOC
			id -> offset = 0;
		} else {
			if(last_id -> type == INT) id -> offset = last_id -> offset + 1;
		}
	}
	/*for(Id*i=idls;i<=id;i++){ //��ӡ���ű�
		if(i->csmk==GLO)printf("_GLO");
		else if(i->csmk==FUN)printf("_FUN");
		else if(i->csmk==LOC)printf("_LOC");
		else printf("_%s",i->name);
		printf("_%d_\n",i->offset);
	}printf("\n");*/
}

Id* getid(char *tks) {
	for(Id *i = id; i >= idls; i--) {
		if(i -> csmk == ID && !strcmp(tks, i -> name)) return i;
	}
	printf("error!\n"); exit(-1);
}

void inblock() {
	(++id) -> csmk = LOC;
}

void outblock() {
	while(id -> csmk != LOC) {
		//*e++ = POP; *e++ = AX;
		//*e++ = DEC; *e++ = SP; *e++ = 1;
		id--;
	}
	id--;
}

int lev(char *opr) { //���ȼ�Խ��levԽ����������levΪ0
	char *oprs[] = {
		")",
		"", "&&", "||", "!",
		"", "==", "!=",
		"", ">", "<", ">=", "<=",
		"", "+", "-",
		"", "*", "/", "%"
	};
	int lev = 1;
	for(int i = 0; i < sizeof(oprs) / sizeof(*oprs); i++) {
		if(!strcmp(oprs[i], opr)) {
			return lev;
		} else if(!strcmp(oprs[i], "")) {
			lev++;
		}
	}
	return 0; //��������
}

int expr(char *last_opr) { //1 + 2 ^ 3 * 4 == (1 + (2 ^ (3) * (4)))
	int type;
	int is_lvalue = 0;
	if(tki == INT) {
		type = INT;
		*e++ = SET; *e++ = AX; *e++ = atoi(tks);
		next();
	} else if(tki == ID) {
		Id *this_id = getid(tks);
		type = this_id -> type;
		if(type == FUN) {
			type = INT;
			int callee = this_id -> offset;
			next(); if(strcmp(tks, "(")) { printf("error!\n"); exit(-1); }
			next();
			//*e++ = PUSH; *e++ = BP;
			int argc = 0;
			while(strcmp(tks, ")")) { //������ջ
				argc++;
				int type = expr("");
				//if(type != (++this_id) -> type) { printf("error!\n"); exit(-1); } //�������
				if(type == INT) {
					*e++ = PUSH; *e++ = AX;
				} else { printf("error!\n"); exit(-1); }
				if(!strcmp(tks, ",")) next();
			}
			*e++ = SET; *e++ = AX; int *_e = e++; //push next ip
			//*e++ = AF; //����ƫ������ȡ������ַ
			*e++ = PUSH; *e++ = AX;
			*e++ = JMP; *e++ = callee; //jmp
			*_e = e - emit; //set next ip
			*e++ = DEC; *e++ = SP; *e++ = argc;
		} else if(type == API) {
			type = INT;
			int callee = this_id -> offset;
			next(); if(strcmp(tks, "(")) { printf("error!\n"); exit(-1); }
			next();
			while(strcmp(tks, ")")) { //������ջ
				int type = expr("");
				if(type == INT) {
					*e++ = PUSH; *e++ = AX;
				} else { printf("error!\n"); exit(-1); }
				if(!strcmp(tks, ",")) next();
			}
			*e++ = callee;
		} else if(type == INT) {
			*e++ = this_id -> class == GLO ? AG: AL; *e++ = this_id -> offset;
			is_lvalue = 1;
		}
		next();
	} else if(!strcmp(tks, "(")) {
		next();
		expr(")");
		if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); }
		next();
	} else if(!strcmp(tks, "!")) {
		next();
		expr("!");
		*e++ = NOT;
	} else { printf("error!\n"); exit(-1); }
	
	if(!strcmp(tks, "=")) {
		*e++ = PUSH; *e++ = AX;
		next();
		expr("");
		if(is_lvalue) *e++ = ASS; else { printf("error!\n"); exit(-1); }
	} else {
		if(is_lvalue) *e++ = VAL;
		while(lev(tks) > lev(last_opr)) {
			char *opr = tks;
			*e++ = PUSH; *e++ = AX;
			next();
			if(type != expr(opr)) { printf("error!\n"); exit(-1); }
			if (!strcmp(opr, "+")) *e++ = ADD;
			else if(!strcmp(opr, "-")) *e++ = SUB;
			else if(!strcmp(opr, "*")) *e++ = MUL;
			else if(!strcmp(opr, "/")) *e++ = DIV;
			else if(!strcmp(opr, "%")) *e++ = MOD;
			else if(!strcmp(opr, "==")) *e++ = EQ;
			else if(!strcmp(opr, ">")) *e++ = GT; //greater than
			else if(!strcmp(opr, "<")) *e++ = LT; //less than
			else if(!strcmp(opr, "!=")) {
				*e++ = EQ;
				*e++ = NOT;
			}
			else if(!strcmp(opr, ">=")) {
				*e++ = LT;
				*e++ = NOT;
			}
			else if(!strcmp(opr, "<=")) {
				*e++ = GT;
				*e++ = NOT;
			}
			else if(!strcmp(opr, "&&")) *e++ = AND;
			else if(!strcmp(opr, "||")) *e++ = OR;
			else { printf("error!\n"); exit(-1); }
		}
	}
	return type;
}

void stmt() {
	if(!strcmp(tks, "{")) {
		inblock();
		next();
		while(strcmp(tks, "}")) {
			if(tki == Int) declare(LOC);
			else stmt();
			next();
		}
		outblock();
	} else if(tki == If) {
		next(); if(strcmp(tks, "(")) { printf("error!\n"); exit(-1); }
		next(); expr(")");
		if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); }
		next();
		*e++ = JZ; int *_e1 = e++;
		stmt();
		char *_p = p;
		next();
		if(tki == Else) {
			*e++ = JMP; int *_e2 = e++;
			*_e1 = e - emit;
			next();
			stmt();
			*_e2 = e - emit;
		} else {
			*_e1 = e - emit;
			p = _p;
		}
	} else if(tki == Do) {
		int *_e1 = e;
		next();
		stmt();
		next();
		if(tki == While) next(); else { printf("error!\n"); exit(-1); }
		if(strcmp(tks, "(")) { printf("error!\n"); exit(-1); }
		next();
		expr(")");
		if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); }
		next();
		if(strcmp(tks, ";")) { printf("error!\n"); exit(-1); }
		*e++ = JZ; int *_e2 = e++;
		*e++ = JMP; *e++ = _e1 - emit;
		*_e2 = e - emit;
	} else if(tki == For) {
		inblock();
		next();
		if(!strcmp(tks, "(")) next(); else { printf("error!\n"); exit(-1); }
		if(strcmp(tks, ";")) {
			if(tki == Int) declare(LOC);
			else expr("");
			if(strcmp(tks, ";")) { printf("error!\n"); exit(-1); }
		}
		next();
		int *_e1 = e;
		int *_e4;
		if(strcmp(tks, ";")) {
			expr("");
			if(strcmp(tks, ";")) { printf("error!\n"); exit(-1); }
			*e++ = JZ; _e4 = e++;
		}
		*e++ = JMP; int *_e3 = e++; 
		next();
		int *_e2 = e;
		if(strcmp(tks, ")")) {
			expr(")");
			if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); }
			*e++ = JMP; *e++ = _e1 - emit;
		}
		next();
		*_e3 = e - emit;
		stmt();
		*e++ = JMP; *e++ = _e2 - emit;
		*_e4 = e - emit;
		outblock();
	} else if(tki == While) {
		int *_e1 = e;
		next(); if(strcmp(tks, "(")) { printf("error!\n"); exit(-1); }
		next(); expr(")");
		if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); }
		next();
		*e++ = JZ; int *_e2 = e++;
		stmt();
		*e++ = JMP; *e++ = _e1 - emit;
		*_e2 = e - emit;
	} else if(tki == Return) {
		next(); expr("");
		*e++ = MOV; *e++ = SP; *e++ = BP; //sp = bp
		*e++ = POP; *e++ = BP;
		*e++ = POP; *e++ = IP;
	} else {
		if(strcmp(tks, ";")) {
			expr("");
			if(strcmp(tks, ";")) { printf("error!\n"); exit(-1); }
		}
	}
}

void declare(int env) {
	static int varc;
	if(env == GLO) {//printf("-%s-\n",tks);
		int type;
		if(tki == Int) type = INT;
		else { printf("error!\n"); exit(-1); }
		next();
		if(tki != ID) { printf("error!\n"); exit(-1); }
		setid(tks, type);
		next();
		if(!strcmp(tks, "(")) {
			varc = 0;
			id -> type = FUN;
			id -> offset = e - emit;
			(++id) -> csmk = FUN; //infunc
			*e++ = PUSH; *e++ = BP;
			*e++ = MOV; *e++ = BP; *e++ = SP; //bp = sp
			*e++ = INC; *e++ = SP; int *_e = e++;
			//declare(ARG);
			next();
			int argc = 0;
			if(strcmp(tks, ")")) {
				while(1) {
					argc++;
					int type;
					if(tki == Int) type =INT;
					else { printf("error!\n"); exit(-1); }
					next();
					if(tki == ID) setid(tks, type);
					else { printf("error!\n"); exit(-1); }
					next();
					if(!strcmp(tks, ")")) break;
					else if(!strcmp(tks, ",")) next();
					else { printf("error!\n"); exit(-1); }
				}
			}
			for(Id *i = id; i -> csmk != FUN; i--) i -> offset -= argc + 2;
			next();
			if(strcmp(tks, "{")) { printf("error!\n"); exit(-1); }
			next();
			while(strcmp(tks, "}")) {
				if(tki == Int) declare(LOC);
				else stmt();
				next();
			}
			*_e = varc;
			*e++ = MOV; *e++ = SP; *e++ = BP; //sp = bp
			*e++ = POP; *e++ = BP;
			*e++ = POP; *e++ = IP;
			while(id -> csmk != FUN) id--; id--; //outfunc
		} else {
			while(1) {
				if(!strcmp(tks, "=")) {
					next();
					if(type == INT) {
						if(tki != INT) { printf("error!\n"); exit(-1); }
						*(data + id -> offset) = atoi(tks);
					} else { printf("error!\n"); exit(-1); }
					next();
				} else {
					if(type == INT) *(data + id -> offset) = 0;
				}
				if(!strcmp(tks, ";")) break;
				else if(!strcmp(tks, ",")) {
					next(); if(tki != ID) { printf("error!\n"); exit(-1); }
					setid(tks, type);
					next();
				} else { printf("error!\n"); exit(-1); }
			}
		}
	} else if(env == LOC) {
		int type;
		if(tki == Int) type = INT;
		else { printf("error!\n"); exit(-1); }
		next();
		while(1) {
			varc++;
			if(tki != ID) { printf("error!\n"); exit(-1); }
			setid(tks, type);
			next();
			if(!strcmp(tks, "=")) {
				next();
				if(type == INT) {
					//if(tki != INT) { printf("error!\n"); exit(-1); }
					*e++ = AL; *e++ = id -> offset;
					*e++ = PUSH; *e++ = AX;
					if(type != expr("")) { printf("error10!\n"); exit(-1); }//*e++ = SET; *e++ = AX; *e++ = atoi(tks);
					*e++ = ASS;
				} else { printf("error!\n"); exit(-1); }
				//next();
			}
			if(!strcmp(tks, ";")) break;
			else if(!strcmp(tks, ",")) next();
			else { printf("error!\n"); exit(-1); }
		}
	}
}

int* print_emit(int *i) {
	#define PRINT_REG(x) { \
		if(x == IP) printf("IP "); \
		else if(x == BP) printf("BP "); \
		else if(x == SP) printf("SP "); \
		else if(x == AX) printf("AX "); \
	}
	if(*i == PUSH) {
		printf("PUSH ");
		if(*++i >= IP && *i <= AX) PRINT_REG(*i)
		else printf("[%d]", *i);
	} else if(*i == POP) {
		printf("POP  ");
		if(*++i >= IP && *i <= AX) PRINT_REG(*i)
		else printf("[%d]", *i);
	} else if(*i == SET) {
		printf("SET  ");
		if(*++i >= IP && *i <= AX) PRINT_REG(*i)
		else printf("[%d]", *i);
		printf("%d", *++i);
	} else if(*i == INC) {
		printf("INC  ");
		if(*++i >= IP && *i <= AX) PRINT_REG(*i)
		else printf("[%d]", *i);
		printf("%d", *++i);
	} else if(*i == DEC) {
		printf("DEC  ");
		if(*++i >= IP && *i <= AX) PRINT_REG(*i)
		else printf("[%d]", *i);
		printf("%d", *++i);
	} else if(*i == JMP) {
		printf("JMP  ");
		printf("[%d]", *++i);
	} else if(*i == JZ) {
		printf("JZ   ");
		printf("[%d]", *++i);
	} else if(*i == MOV) {
		printf("MOV  ");
		if(*++i >= IP && *i <= AX) PRINT_REG(*i)
		else printf("[%d]", *i);
		if(*++i >= IP && *i <= AX) PRINT_REG(*i)
		else printf("[%d]", *i);
	}
	else if(*i == ADD) printf("ADD");
	else if(*i == SUB) printf("SUB");
	else if(*i == MUL) printf("MUL");
	else if(*i == DIV) printf("DIV");
	else if(*i == MOD) printf("MOD");
	else if(*i == ASS) printf("ASS");
	else if(*i == EQ) printf("EQ");
	else if(*i == GT) printf("GT");
	else if(*i == LT) printf("LT");
	else if(*i == AND) printf("AND");
	else if(*i == OR) printf("OR");
	else if(*i == NOT) printf("NOT");
	else if(*i == AG) {
		printf("AG   ");
		printf("%d", *++i);
	} else if(*i == AL) {
		printf("AL   ");
		printf("%d", *++i);
	} else if(*i == VAL) printf("VAL");
	else if(*i == PRINT) printf("PRINT");
	else if(*i == ENDL) printf("ENDL");
	else if(*i == SPACE) printf("SPACE");
	else if(*i == SCAN) printf("SCAN");
	else if(*i == EXIT) printf("EXIT");
	else printf("[%d]", i - emit);
	return i;
	#undef PRINT_REG
}

int main(int argc, char *argv[]) {
	int src = 0, debug = 0;
	char *fname = NULL;
	FILE *fp;
	if(argc < 2) { printf("error!\n"); exit(-1); }
	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-s")) {
			src = 1;
		} else if(!strcmp(argv[i], "-d")) {
			debug = 1;
		} else {
			char *j;
			if(j = strrchr(argv[i], '.')) {
				if(!strcmp(j, ".c")) fname = argv[i];
				else { printf("error!\n"); exit(-1); }
			} else { printf("error!\n"); exit(-1); }
		}
	}
	if(!fname) { printf("error!\n"); exit(-1); }
	if(!(fp = fopen(fname, "r"))) { printf("error!\n"); exit(-1); }

	p = (char*)malloc(MAXSIZE * sizeof(char));
	idls = id = (Id*)malloc(MAXSIZE * sizeof(Id));
	emit = e = (int*)malloc(MAXSIZE * sizeof(int));
	data = (int*)malloc(MAXSIZE * sizeof(int));

	{ int i = fread(p, sizeof(char), MAXSIZE, fp); p[i] = '\0'; }//printf("%s",p);
	fclose(fp);
	
	//into code
	*e++ = SET; *e++ = AX; int *_exit = e++;
	*e++ = PUSH; *e++ = AX;
	//*e++ = PUSH; *e++ = BP;
	*e++ = JMP; int *_main = e++;
	*_exit = e - emit; *e++ = EXIT;
	
	id -> csmk = GLO; //inglobal
	char *api[] = {
		"print", "endl", "space", "scan"
	};
	for(int i = 0; i < sizeof(api) / sizeof(*api); i++) {
		setid(api[i], API);
		id -> offset = PRINT + i;
	}
	next();
	while(strcmp(tks, "") || tki != -1) {
		declare(GLO);
		next();
	}
	*_main = getid("main") -> offset;
	
	//print
	if(src) {
		for(int *i = emit; i < e; i++) {
			printf("%d\t", i - emit);
			i = print_emit(i);
			printf("\n");
		}
	}
	
	//run..
	*(SP + data) = *(BP + data) = AX + 1; //sp = AX + 1;
	*(IP + data) = 0;//int *ax = NULL; //ip = 0;
	while(1) {
		if(debug) {
			printf("\n_%d_%d_%d_%d_\t", *(IP + data), *(BP + data), *(SP + data), *(data + AX));
			print_emit(emit + *(IP + data));
		}
		int i = *(emit + (*(IP + data))++);
		if(i == PUSH) {
			int opr = *(emit + (*(IP + data))++);
			*(data + (*(SP + data))++) = *(data + opr);//printf(" %d",*(data+*(SP+data)-1));
		} else if(i == POP) {
			int opr = *(emit + (*(IP + data))++);
			*(data + opr) = *(data + (--*(SP + data)));//printf(" %d",*(data+*(SP+data)));
		} else if(i == SET) {
			int opr1 = *(emit + (*(IP + data))++);
			int opr2 = *(emit + (*(IP + data))++);
			*(data + opr1) = opr2;
		} else if(i == INC) {
			int opr1 = *(emit + (*(IP + data))++);
			int opr2 = *(emit + (*(IP + data))++);
			*(data + opr1) += opr2;
		} else if(i == DEC) {
			int opr1 = *(emit + (*(IP + data))++);
			int opr2 = *(emit + (*(IP + data))++);
			*(data + opr1) -= opr2;
		} else if(i == JMP) {
			int opr = *(emit + (*(IP + data))++);
			*(IP + data) = opr;
		} else if(i == JZ) { //jump if zero
			int opr = *(emit + (*(IP + data))++);
			if(!*(data + AX)) *(IP + data) = opr;
		} else if(i == MOV) {
			int opr1 = *(emit + (*(IP + data))++);
			int opr2 = *(emit + (*(IP + data))++);
			*(data + opr1) = *(data + opr2);
		} else if(i == ADD) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr1 + opr2;
		} else if(i == SUB) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr2 - opr1;
		} else if(i == MUL) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr1 * opr2;
		} else if(i == DIV) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr2 / opr1;
		} else if(i == MOD) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr2 % opr1;
		} else if(i == ASS) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + opr2) = opr1;
		} else if(i == EQ) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr2 == opr1;
		} else if(i == GT) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr2 > opr1;
		} else if(i == LT) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr2 < opr1;
		} else if(i == AND) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr2 && opr1;
		} else if(i == OR) {
			int opr1 = *(data + AX);
			int opr2 = *(data + (--*(SP + data)));
			*(data + AX) = opr2 || opr1;
		} else if(i == NOT) {
			int opr = *(data + AX);
			*(data + AX) = !opr;
		} else if(i == AG) { //address global
			int opr = *(emit + (*(IP + data))++);
			*(data + AX) = opr;
		} else if(i == AL) { //address local
			int opr = *(emit + (*(IP + data))++);
			*(data + AX) = *(BP + data) + opr;//ax = bp + ax
		} else if(i == VAL) {
			int opr = *(data + AX);
			*(data + AX) = *(data + opr);
		} else if(i == PRINT) {
			int opr = *(data + (--*(SP + data)));
			printf("%d", opr);
		} else if(i == ENDL) {
			printf("\n");
		} else if(i == SPACE) {
			printf(" ");
		} else if(i == SCAN) {
			int opr;
			scanf("%d", &opr);
			*(data + AX) = opr;
		} else if(i == EXIT) {
			break;
		}//if(ax)printf(" >>%d",*ax);
	}
	//printf("\n%d\n",*(data + AX));
	return 0;
}