#include "cc.h"
#include <stdio.h>
#include <malloc.h>

static Type *ty, *tys;
static Type **argtyls, **argtylss;

void type_init(void) {
	tys = ty = (Type*)malloc(MAXSIZE * sizeof(Type));
	argtylss = argtyls = (Type**)malloc(MAXSIZE * sizeof(Type*));
}

static Type** getargtyls(int count) {
	if(count == 0) return NULL;
	Id *_id = id - count;
	Type **_argtyls = argtylss;
	int i = 0;
	while(_argtyls < argtyls) {
		i = 0;
		while(i < count && i < argtyls - _argtyls) {
			if(_id[i].type != _argtyls[i]) break;
			i++;
		}
		if(i == count) return _argtyls;
		else if(i == argtyls - _argtyls) break;
		else _argtyls++;
	}
	while(i < count) {
		*argtyls++ = _id[i++].type;
	}
	return _argtyls;
}

int typesize(Type *type) {
	if(type -> base == INT) return 1;
	else if(type -> base == PTR) return 1;
	else if(type -> base == ARR) return typesize(type -> rely) * type -> count;
	return 0;
}

Type* deriv_type(int base, Type *rely, int count) { //��������
	if(rely == NULL) {
		if(base == INT) {
			for(Type *i = tys; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == NULL) return i;
			}
			ty -> base = base;
			ty -> rely = NULL;
			return ty++;
		} else { printf("error7!\n"); exit(-1); }
	} else {
		if(base == PTR) {
			for(Type *i = tys; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == rely) return i;
			}
			ty -> base = base;
			ty -> rely = rely;
			return ty++;
		} else if(base == ARR) {
			if(rely -> base == FUN) { printf("error8!\n"); exit(-1); }
			for(Type *i = tys; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == rely
				&& i -> count == count) return i;
			}
			ty -> base = base;
			ty -> rely = rely;
			ty -> count = count;
			return ty++;
		} else if(base == FUN) {// || base == API) {
			if(rely -> base == FUN || rely -> base == ARR) { printf("error9!\n"); exit(-1); }
			Type **argtyls = getargtyls(count);
			for(Type *i = tys; i < ty; i++) {
				if(i -> base == base
				&& i -> rely == rely
				&& i -> count == count
				&& i -> argtyls == argtyls) return i;
			}
			ty -> base = base;
			ty -> rely = rely;
			ty -> count = count;
			ty -> argtyls = argtyls;
			return ty++;
		} else { printf("error10!\n"); exit(-1); }
	}
}

static void _print_type(Type *type) {
	if(type -> base == PTR) {
		printf("ָ��");
		_print_type(type -> rely);
		printf("��ָ��");
	} else if(type -> base == ARR) {
		printf("ӵ��%d������Ϊ", type -> count);
		_print_type(type -> rely);
		printf("��Ԫ�ص�����");
	} else if(type -> base == FUN) {
		//printf("��Ҫ%d�������ҷ���ֵΪ", type -> count);
		for(int i = 0; i < type -> count; i++) {
			printf("��%d������Ϊ", i + 1);
			_print_type(type -> argtyls[i]);
			printf("��");
		}
		printf("����ֵΪ");
		_print_type(type -> rely);
		printf("�ĺ���");
	} else if(type -> base == API) {
		printf("��Ҫ%d�������ҷ���ֵΪ", type -> count);
		_print_type(type -> rely);
		printf("��API");
	} else if(type -> base == INT) {
		printf("����");
	}
}

void print_type(Id *this_id) {
	printf("%sΪ", this_id -> name);
	_print_type(this_id -> type);
	//printf("\n");
}
