//��������������
//�ܹ�����ָ�롢���顢����
//ǰ�÷��ŵݹ��ȡ�����÷���ѭ����ȡ
//ʶ��ģʽ��˵���� ������ [;]

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
		else { //��������ʶ��ķ���
			p++;
		}
	}
}

Type* deriv_type(int base, Type *rely, int count) { //��������
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
		printf("ָ��");
		print_type(type -> rely);
		printf("��ָ��");
	} else if(type -> base == ARR) {
		printf("ӵ��%d������Ϊ", type -> count);
		print_type(type -> rely);
		printf("��Ԫ�ص�����");
	} else if(type -> base == FUN) {
		printf("��Ҫ%d�������ҷ���ֵΪ", type -> count);
		print_type(type -> rely);
		printf("�ĺ���");
	} else if(type -> base == INT) {
		printf("����");
	} else if(type -> base == CHAR) {
		printf("�ַ���");
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
	return 0; //��������
}

Type* declarator(Type *type); //ǰ������
int* complex(char *last_opr, int *cpx) { //�������ͷ���
	//ǰ�÷���
	if(!strcmp(tks, "*")) { //ָ��
		next();
		cpx = complex("*", cpx);
		cpx++;
		*cpx++ = PTR;
	} else if(!strcmp(tks, "(")) { //����
		next();
		cpx = complex(")", cpx);
		if(strcmp(tks, ")")) { printf("error!\n"); exit(-1); } //"("�޷�ƥ�䵽")"
		next();
	} else if(tki == ID) {
		if(!name) name = tks;
		//(id - 1) -> name = tks;
		next();
	} else { printf("error!\n"); exit(-1); }
	
	//next();
	//���÷���
	while(lev(tks) > lev(last_opr)) {
		if(!strcmp(tks, "[")) { //����
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
		} else if(!strcmp(tks, "(")) { //����
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
	int cpxsk[BUFSIZE]; //��������ջ
	int *cpx = cpxsk; //��������ջջ��ָ��
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
	printf("%sΪ", name);
	print_type(type);
	printf("\n");
	return 0;
}
