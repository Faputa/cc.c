//词法分析

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 10000

char *p, *tks;
int *d, tki;
enum {
	//keyword
	Int, If, Else, While, Do, For, Return,
	//type
	INT,
	//other
	ID
};

void next() {//printf("%c\n",*p);
	char *keyword[] = {
		"int", "if","else","while","do","for","return"
	};
	char *point[] = {
		"==",">=","<=","&&","||","+=","-=","*=","/=",
		"+","-","*","/","%","=",">","<","!","^","&","|","(",")","{","}","[","]","?",":",",",";"
	};
	tks = ""; tki = -1;
	while(*p) {
		if(!strncmp(p, "//", 2) || *p == '#') { while(*p != '\n') p++; p++; }
		else if(!strncmp(p, "/*", 2)) { while(strncmp(p, "*/", 2)) p++; p+=2; }
		else if((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_') {
			int len = 0; char *_p = p;
			while((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_') {
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
			tks[len] = '\0';//printf("%s\n",tks);
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
					p += strlen(point[i]);//printf("%s\n",tks);
					return;
				}
			}
			p++;
		}
	}
}
int main(int argc, char *argv[]) {
	FILE *fp = fopen(argv[1], "r");
	p = (char*)malloc(MAXSIZE * sizeof(char));
	int i = fread(p, sizeof(char), MAXSIZE, fp);
	p[i] = '\0';printf("%s",p);
	fclose(fp);
	next();
	while(strcmp(tks, "") || tki != -1) {
		if(strcmp(tks, "")) printf("%s", tks);
		if(tki != -1) printf("\t%d", tki);
		printf("\n");
		next();
	}
}