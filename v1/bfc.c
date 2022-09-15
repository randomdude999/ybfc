#include <stdio.h>
#include <stdlib.h>

#define error(x) fputs("error: " x "\n", stderr), exit(1)

#define ASM_INC "\tinc byte ptr [rdx+rax]"
#define ASM_DEC "\tdec byte ptr [rdx+rax]"
#define ASM_LEFT "\tsub eax,1\n"\
	"\tcmovb eax,ecx"
#define ASM_RIGHT "\tadd eax,1\n"\
	"\tcmp ecx,eax\n"\
	"\tcmovb eax,ebx"
#define ASM_OUT "\tcall output"
#define ASM_INP "\tcall input"

int loopi = 0;
int loopstack[256];
int loopdepth = 0;

void emit_startloop() {
	if(loopdepth == sizeof(loopstack)/sizeof(int))
		error("too many nested loops");
	loopstack[loopdepth++] = ++loopi;
	printf("loop%d:\n", loopi);
	puts("\tcmp bl,[rdx+rax]");
	printf("\tje loop%d_end\n", loopi);
}

void emit_stoploop() {
	if(loopdepth == 0) error("unbalanced brackets");
	int thisloop = loopstack[--loopdepth];
	printf("\tjmp loop%d\n", thisloop);
	printf("loop%d_end:\n", thisloop);
}

int main() {
	puts("\t.intel_syntax noprefix");
	puts("\t.globl main");
	puts("\t.text");
	puts("main:");
	while(1) {
		char buf[1024];
		int numread;
		if((numread = fread(buf, 1, 1024, stdin)) == 0) break;
		for(int i=0; i<numread; i++) {
			if(buf[i] == '+') puts(ASM_INC);
			if(buf[i] == '-') puts(ASM_DEC);
			if(buf[i] == '<') puts(ASM_LEFT);
			if(buf[i] == '>') puts(ASM_RIGHT);
			if(buf[i] == '.') puts(ASM_OUT);
			if(buf[i] == ',') puts(ASM_INP);
			if(buf[i] == '[') emit_startloop();
			if(buf[i] == ']') emit_stoploop();
		}
	}
	puts("\tret");
}
