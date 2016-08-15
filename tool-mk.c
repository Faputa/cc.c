//将文件中的所有"error"替换成"error"+数字

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 10000

int main(int argc, char *argv[]) {
	char *src, *emit, *fname = NULL;
	int reset = 0;
	FILE *fp;
	if(argc < 2) { printf("error!\n"); exit(-1); }
	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-r")) {
			reset = 1;
		} else {
			fname = argv[i];
		}
	}
	if(!fname) { printf("error!\n"); exit(-1); }
	if(!(fp = fopen(fname, "r"))) { printf("error!\n"); exit(-1); }
	
	{ //read
		fseek(fp, 0, SEEK_END);
		int fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char *p = src = (char*)malloc(fsize * sizeof(char));
		char *e = emit = (char*)malloc((fsize + MAXSIZE) * sizeof(char));
		emit[0] = '\0';
		
		{ int i = fread(p, sizeof(char), fsize, fp); p[i] = '\0'; }
		fclose(fp);
		
		char buf[100], *str = "error";
		int count = 0, len = strlen(str);
		while(*p) {
			if(!strncmp(p, str, len)) {
				p += len;
				while(*p >= '0' && *p <= '9') p++;
				if(reset) {
					sprintf(buf, "%s", str);
				} else {
					sprintf(buf, "%s%d", str, ++count);
				}
				strcat(emit, buf);
				e += strlen(buf);
				//sprintf(emit, "%s%s%d", emit, str, ++count); //效率低，废弃
				//e = emit + strlen(emit);
			} else {
				*e++ = *p++;
				*e = '\0';
			}
		}
	}
	
	{ //write
		//打印
		//printf("%s", emit);
		
		if(!reset) {//备份原文件
			int len;
			char buf[100], *fe;
			if(fe = strrchr(fname, '.')) {
				len = fe - fname;
				strncpy(buf, fname, len);
			} else {
				len = strlen(fname);
				strncpy(buf, fname, len);
				fe = "";
			}
			int count = 0;
			do {
				buf[len] = '\0';
				sprintf(buf, "%s%s%d%s%s", buf, "(", ++count, ")", fe);
			} while(!access(buf, F_OK));
			if(!(fp = fopen(buf, "w"))) { printf("error!\n"); exit(-1); }
			fwrite(src, sizeof(char), strlen(src), fp);
			fclose(fp);
		}
		
		//更新原文件
		if(!(fp = fopen(fname, "w"))) { printf("error!\n"); exit(-1); }
		fwrite(emit, sizeof(char), strlen(emit), fp);
		fclose(fp);
	}
}
