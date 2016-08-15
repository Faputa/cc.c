//将文件中的所有"error"替换成"error"+数字

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#define MAXSIZE 10000*100

int main(int argc, char *argv[]) {
	char *emit, *fname = argv[1];
	{ //read
		FILE *fp = fopen(fname, "r");
		//p = (char*)malloc(MAXSIZE * sizeof(char));
		fseek(fp, 0, SEEK_END);
		int fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char *p = (char*)malloc(fsize * sizeof(char));
		char *e = emit = (char*)malloc(MAXSIZE * sizeof(char));
		
		//{ int i = fread(p, sizeof(char), MAXSIZE, fp); p[i] = '\0'; }//printf("%s",p);
		{ int i = fread(p, sizeof(char), fsize, fp); p[i] = '\0'; }//printf("%s",p);
		fclose(fp);
		
		char buf[100], *str = "error";
		int count = 0, len = strlen(str);
		while(*p) {
			if(!strncmp(p, str, len)) {
				p += len;
				while(*p >= '0' && *p <= '9') p++;
				sprintf(buf, "%s%d", str, ++count);//printf("%s\n",buf);
				strcat(emit, buf);
				e += strlen(buf);
				//sprintf(emit, "%s%s%d", emit, str, ++count); //效率低
				//e = emit + strlen(emit);
			} else {
				*e++ = *p++;
				*e = '\0';
			}
		}
	}
	{ //write
		/*打印*/
		//printf("%s", emit);
		
		/*覆盖原文件*/
		//fp = fopen(fname, "w");
		//fwrite(emit, sizeof(char), strlen(emit), fp);
		
		/*不覆盖原文件*/
		int i;
		char buf[100], *fe;
		if(fe = strrchr(fname, '.')) {
			strncpy(buf, fname, fe - fname);
			i = fe - fname;
		} else {
			strncpy(buf, fname, strlen(fname));
			i = strlen(fname);
			fe = "";
		}
		int count = 0;
		do {
			buf[i] = '\0';
			sprintf(buf, "%s%s%d%s%s", buf, "(", ++count, ")", fe);//printf("%s\n",buf);
		} while(!access(buf, F_OK));
		FILE *fp = fopen(buf, "w");
		fwrite(emit, sizeof(char), strlen(emit), fp);
		fclose(fp);
	}
}
