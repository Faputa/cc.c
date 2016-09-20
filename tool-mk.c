//将文件中的所有"error"替换成"error"+数字
//支持读取多个文件

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 10000
#define BUFSIZE 100

int main(int argc, char *argv[]) {
	char *src[BUFSIZE], *emit[BUFSIZE], *fname[BUFSIZE];
	FILE *fp[BUFSIZE];
	int reset = 0;
	for(int i = 0; i < sizeof(fname) / sizeof(*fname); i++) fname[i] = NULL;
	if(argc < 2) { printf("error!\n"); exit(-1); }
	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-r")) {
			reset = 1;
		} else {
			int j = 0;
			while(fname[j]) {
				if(!strcmp(argv[i], fname[j])) break;
				j++;
			}
			fname[j] = argv[i];
		}
	}
	
	for(int i = 0; fname[i]; i++) { //read
		if(!(fp[i] = fopen(fname[i], "r"))) { printf("error!\n"); exit(-1); }
		fseek(fp[i], 0, SEEK_END);
		int fsize = ftell(fp[i]);
		fseek(fp[i], 0, SEEK_SET);
		
		char *p = src[i] = (char*)malloc(fsize * sizeof(char));
		char *e = emit[i] = (char*)malloc((fsize + MAXSIZE) * sizeof(char)); emit[i][0] = '\0';
		
		{ int j = fread(p, sizeof(char), fsize, fp[i]); p[j] = '\0'; }
		fclose(fp[i]);
		
		char buf[BUFSIZE], *str = "error";
		static int count = 0;
		int len = strlen(str);
		while(*p) {
			if(!strncmp(p, str, len)) {
				p += len;
				while(*p >= '0' && *p <= '9') p++;
				if(reset) {
					sprintf(buf, "%s", str);
				} else {
					sprintf(buf, "%s%d", str, ++count);
				}
				strcat(emit[i], buf);
				e += strlen(buf);
				//sprintf(emit, "%s%s%d", emit, str, ++count); //效率低，废弃
				//e = emit + strlen(emit);
			} else {
				*e++ = *p++;
				*e = '\0';
			}
		}
	}
		
	for(int i = 0; fname[i]; i++) { //write
		//打印
		//printf("%s", emit);
		
		if(!reset) {//备份原文件
			int len;
			char buf[BUFSIZE], *fe;
			if(fe = strrchr(fname[i], '.')) {
				len = fe - fname[i];
				strncpy(buf, fname[i], len);
			} else {
				len = strlen(fname[i]);
				strncpy(buf, fname[i], len);
				fe = "";
			}
			int count = 0;
			do {
				buf[len] = '\0';
				sprintf(buf, "%s%s%d%s%s", buf, "(", ++count, ")", fe);
			} while(!access(buf, F_OK));
			if(!(fp[i] = fopen(buf, "w"))) { printf("error!\n"); exit(-1); }
			fwrite(src[i], sizeof(char), strlen(src[i]), fp[i]);
			fclose(fp[i]);
		}
		
		//更新原文件
		if(!(fp[i] = fopen(fname[i], "w"))) { printf("error!\n"); exit(-1); }
		fwrite(emit[i], sizeof(char), strlen(emit[i]), fp[i]);
		fclose(fp[i]);
	}
}
