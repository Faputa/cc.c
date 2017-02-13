//json½âÎöÆ÷

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 1000
#define INDENT 4

typedef enum {
	NUL, FALSE, TRUE, NUM, STR, ARR, OBJ
} Type;

typedef struct Jsnode {
	char *name;
	Type type;
	struct Jsnode *next;
	union {
		float num;
		char *str;
		struct Jsnode *child;
	};
} Jsnode;

int tki;
char *tks, *p;

void next() {
	char *keyword[] = {
		"null", "false", "true"
	};
	char *trans[] = {
		"\\n", "\n",
		"\\\\", "\\",
		"\\t", "\t",
		"\\\"", "\"",
		"\\0", "\0"
	};
	tks = ""; tki = -1;
	while(*p) {
		if((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_') {
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
			printf("error1!\n"); exit(-1);
		} else if(*p >= '0' && *p <= '9') {
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
		} else if(*p == '"') {
			tki = STR;
			int len = 0; char *_p = ++p;
			while(*p && *p != '"') {
				if(*p++ == '\\') p++;
				len++;
			}
			if(*p) p++;//printf("%d\n",len);
			tks = (char*)malloc(sizeof(char) * (len+1));
			int i = 0;
			while(*_p != '"') {
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
		}
		else if(*p == '{') { tks = "{"; p++; return; }
		else if(*p == '}') { tks = "}"; p++; return; }
		else if(*p == '[') { tks = "["; p++; return; }
		else if(*p == ']') { tks = "]"; p++; return; }
		else if(*p == ':') { tks = ":"; p++; return; }
		else if(*p == ',') { tks = ","; p++; return; }
		else if(*p == ' ' || *p == '\t' || *p == '\n') p++;
		else { printf("error2!\n"); exit(-1); }
	}
}

Jsnode* newjsnode() {
	Jsnode *node = (Jsnode*)malloc(sizeof(Jsnode));
	node->name = "";
	node->next = NULL;
	node->child = NULL;
	return node;
}

void ins_obj_chird(Jsnode *node1, Jsnode *node2);
void parse_json(Jsnode *node) {
	if(!strcmp(tks, "{")) {
		next();
		node->type = OBJ;
		if(strcmp(tks, "}")) {
			while(1) {
				Jsnode *child = newjsnode();
				if(tki == STR) {
					child->name = tks;
					next();
				} else { printf("error3!\n"); exit(-1); }
				if(!strcmp(tks, ":")) next(); else { printf("error4!\n"); exit(-1); }
				switch(tki) {
					case NUL:
					case FALSE:
					case TRUE: child->type = tki; break;
					case NUM: child->type = tki; child->num = atof(tks); break;
					case STR: child->type = tki; child->str = tks; break;
					default: parse_json(child);
				}
				ins_obj_chird(node, child);
				next();
				if(!strcmp(tks, "}")) break;
				else if(!strcmp(tks, ",")) next();
				else { printf("error5!\n"); exit(-1); }
			}
		}// else node->child = NULL;
	} else if(!strcmp(tks, "[")) {
		next();
		node->type = ARR;
		if(strcmp(tks, "]")) {
			node->child = newjsnode();
			node = node->child;
			while(1) {
				switch(tki) {
					case NUL:
					case FALSE:
					case TRUE: node->type = tki; break;
					case NUM: node->type = tki; node->num = atof(tks); break;
					case STR: node->type = tki; node->str = tks; break;
					default: parse_json(node);
				}
				next();
				if(!strcmp(tks, "]")) {
					//node->next = NULL;
					break;
				} else if(!strcmp(tks, ",")) {
					node->next = newjsnode();
					node = node->next;
					next();
				} else { printf("error6!\n"); exit(-1); }
			}
		}// else node->child = NULL;
	} else { printf("error7!\n"); exit(-1); }
}

Jsnode *get_in_obj(Jsnode *node, char *name) {
	if(node->type != OBJ) { printf("error8!\n"); exit(-1); }
	for(Jsnode *i = node->child; i != NULL; i = i->next) {
		if(!strcmp(name, i->name)) return i;
	}
	return NULL;
}

Jsnode *get_in_arr(Jsnode *node, int count) {
	if(node->type != ARR) { printf("error9!\n"); exit(-1); }
	int i = 1;
	Jsnode *j = node->child;
	while(j != NULL) {
		if(i == count) return j;
		j = j->next;
		i++;
	}
	return NULL;
}

void upd_node(Jsnode *node1, Jsnode *node2) {
	if(node1 && node2) {
		node2->next = node1->next;
		*node1 = *node2;
		free(node2);
	}
}

void ins_obj_chird(Jsnode *node1, Jsnode *node2) {
	if(node1 && node2) {
		if(node1->type != OBJ) { printf("error9!\n"); exit(-1); }
		if(node1->child == NULL) {
			node1->child = node2;
		} else {
			Jsnode *i = node1;
			Jsnode *j = node1->child;
			while(j != NULL) {
				if(!strcmp(j->name, node2->name)) {
					upd_node(j, node2);
					return;
				}
				i = j;
				j = j->next;
			}
			i->next = node2;
		}
	}
}

void del_first_child(Jsnode *node);
void del_all_chird(Jsnode *node) {
	if(node) {
		switch(node->type) {
		case NUL:
		case FALSE:
		case TRUE:
		case STR: break;
		case ARR:
		case OBJ:
			while(node->child != NULL) {
				del_first_child(node);
			}
			break;
		default: printf("error12!\n"); exit(-1);
		}
	}
}

void del_first_child(Jsnode *node) {
	if(node) {
		switch(node->type) {
		case NUL:
		case FALSE:
		case TRUE:
		case STR: break;
		case ARR:
		case OBJ: {
			Jsnode *i = node->child;
			node->child = i->next;
			del_all_chird(i);
			free(i);
			break;
		}
		default: printf("error12!\n"); exit(-1);
		}
	}
}

void del_next(Jsnode *node) {
	if(node && node->next) {
		switch(node->next->type) {
		case NUL:
		case FALSE:
		case TRUE:
		case STR: {
			Jsnode *i = node->next;
			node->next = i->next;
			free(i);
			break;
		}
		case ARR:
		case OBJ: {
			Jsnode *i = node->next;
			node->next = i->next;
			del_all_chird(i);
			free(i);
			break;
		}
		default: printf("error12!\n"); exit(-1);
		}
	}
}

float get_num_in_numnode(Jsnode *node) {
	if(node->type != NUM) { printf("error10!\n"); exit(-1); }
	return node->num;
}

char* get_num_in_strnode(Jsnode *node) {
	if(node->type != STR) { printf("error11!\n"); exit(-1); }
	return node->str;
}

static void print_float(float f) {
	char buf[40], *i, *j;
	sprintf(buf, "%f", f);
	i = buf;
	while(*i != '.') i++;
	j = i + 1;
	while(*j) {
		if(*j != '0') i = j + 1;
		j++;
	}
	*i = '\0';
	printf("%s", buf);
}

static void print_indent(int indent) {
	for(int i = 0; i < indent * INDENT; i++) printf(" ");
}

static void print_endl(Jsnode *node) {
	if(node->next != NULL) printf(",\n");
	else printf("\n");
}

void print_jsnode(Jsnode *node ,int indent) {
	print_indent(indent);
	if(strcmp(node->name, "")) printf("\"%s\" : ", node->name);
	switch(node->type) {
	case NUL: printf("null"); break;
	case FALSE: printf("false"); break;
	case TRUE: printf("true"); break;
	case NUM: print_float(get_num_in_numnode(node)); break;
	case STR: printf("\"%s\"", get_num_in_strnode(node)); break;
	case OBJ: printf("{\n");
		for(Jsnode *i = node->child; i != NULL; i = i->next) print_jsnode(i, indent + 1);
		print_indent(indent); printf("}"); break;
	case ARR: printf("[\n");
		for(Jsnode *i = node->child; i != NULL; i = i->next) print_jsnode(i, indent + 1);
		print_indent(indent); printf("]"); break;
	default: printf("error12!\n"); exit(-1);
	}
	print_endl(node);
}

int main(int argc, char *argv[]) {
	FILE *fp;
	if(argc != 2) { printf("error13!\n"); exit(-1); }
	if(!(fp = fopen(argv[1], "r"))) { printf("error14!\n"); exit(-1); }
	p = (char*)malloc(MAXSIZE * sizeof(char));
	{ int i = fread(p, sizeof(char), MAXSIZE, fp); p[i] = '\0'; }
	fclose(fp);
	
	Jsnode *node = newjsnode();
	next();
	parse_json(node);//printf("%d",node->type);
	//del_all_chird(get_in_obj(node, "publisher"));
	//del_next(get_in_obj(node, "title"));
	//ins_obj_chird(get_in_obj(node, "publisher"), get_in_obj(node, "title"));
	//print_jsnode(get_in_obj(node, "title"), 0);
	print_jsnode(node, 0);
}
