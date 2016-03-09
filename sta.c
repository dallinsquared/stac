#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sizedefs.h"
#include "primdefs.h"

#define DSIZE 5000
#define RSSIZE 100
#define STSIZE 100

#define NEXT IP=disk[disk[*rsp]++]
#define RPUSH disk[++(*rsp)]=
#define PUSH disk[--(*tosp)]=
#define TOS disk[*tosp]
#define NTOS disk[(*tosp)+1]
#define NTORS disk[(*rsp)-1]
#define TORS disk[*rsp]
#define DROP ++(*tosp)
#define RDROP --(*rsp)

#define TWOLEVEL(EFFECT) NTOS=EFFECT;DROP;NEXT
#define COMPPRIM(PRIM) enter(primaddr[PRIM])
#define COLON(NAME) intern(DOCOL, 0);int NAME=*dict-1
#define IF(BNAME) enter(notbranch);int BNAME=*dict;enter(0)
#define ELSE(BNAME) COMPPRIM(BRANCH);\
       	disk[BNAME]=(*dict)-BNAME;\
	BNAME=*dict;enter(0)
#define THEN(BNAME) disk[BNAME]=*dict-BNAME

int disk[DSIZE] = {4, DSIZE-(RSSIZE+STSIZE+1), DSIZE-1, 0},
    *dict = disk, *rsp = disk+1, *tosp = disk+2,
    *link = disk+3, w, IP, primaddr[NOT+1], cs;

char itoabuf[10] = {'\0'};

int mputs(char *s){
	int i = fputs(s, stdout);
	fflush(stdout);
	return i;
}
int mputchar(int c){
	int i = putchar(c);
	fflush(stdout);
	return i;
}
int scant(char c, char *s) {  
	int i = 1;
	if((int)c == 127){
		while(i){
			*s = getchar();
			switch (*s) {
			case ' ':
			case '\t':
			case '\n':
				break;
			default:
				s++;
				i = 0;
				break;
			}
		}
	}
	for( i = 0; ; s++, i++) {
		*s = getchar();
		if (*s == c) {
			*s = '\0';
			return i;
		}
		if ((int) c == 127) {
			switch (*s) {
			case ' ':
			case '\t':
			case '\n':
			case '\0':
				*s = '\0';
				return i+1;
			}
		}
		if (*s == EOF){
			exit(0);
		}
	}
}

int streql(int *s1, int *s2) {
	char *ss1 = (char *)(s1+1);
	char *ss2 = (char *)(s2+1);
	int comp = *s1 == *s2;
	if (*s1 != *s2)
		return 0;
	for (int i = 0; i < *s1; i++){
		if (ss1[i] != ss2[i])
			return 0;
	}
	return 1;
}

int putnumstr(int *s1) {
	int len = *s1;
	char *ss1 = (char *)(s1+1);
	for(int i = 0; i < len; i++, ss1++){
		putchar((int)(*ss1));
	}
	return 0;
}

char *itoa(int i){
	char *p = itoabuf;
	if(i<0){
		*p++ = '-';
		i *= -1;
	}
	int shifter = i;
	do{ //Move to where representation ends
		++p;
		shifter = shifter/10;
	} while(shifter);
	*p = '\0';
	do{ //Move back, inserting digits as you go
		*--p =(char)((i%10)+48);
		i = i/10;
	} while(i);
	return itoabuf;
}

void dumpstack(int i, int *sp){
	mputs("STACK[");
	for(int j = 0; j < i; j++){
		mputs(" ");
		mputs(itoa(disk[*sp+j]));
	}
	puts(" ]");
}
void enter(int x){
	disk[(*dict)++] = x;
}

void intern(int x, int imm) {
	enter(*link);
	*link = *dict-1;
	w = *dict;
	enter(0);
	int slen = scant((char) 127, (char *)(disk+(*dict)));
	int ilen = slen/PACK + (slen%PACK) ? 1 : 0;
	disk[w] = slen;
	*dict += slen/PACK + 1;
	enter(imm);
	enter(x);
	mputs("interning ");
	putnumstr(disk+w);
	mputs(" @ ");
	puts(itoa(*dict-1));
}

void execute(int x) {
	char *s;
	switch (x) {
	case DOCOL:
		w = ++IP;
		RPUSH ++IP;
		IP = disk[w];
		break;
	case KEY:
		PUSH getchar();
		NEXT;
		break;
	case WORD:
		w = *dict;
		enter(0);
		int slen = scant((char) TOS, (char *)(disk + (*dict)));
		DROP;
		disk[w] = slen;
		PUSH w;
		*dict += slen/PACK + 1;
		NEXT;
		break;
	case FIND:
		w = *link;
		while (!(streql(disk+TOS, (disk+w+1))) && w) {
			w = disk[w];
		}
		if (!w) {
			PUSH 1;
		} else {
			TOS = w + 2 + ((disk[w+1])/PACK + 2);
			w = disk[TOS-1];
			PUSH w;
		}
		NEXT;
		break;
	case EXIT:
		RDROP;
		NEXT;
		break;
	case PEEK:
		TOS = disk[TOS];
		NEXT;
		break;
	case POKE:
		disk[TOS] = NTOS;
		DROP;
		DROP;
		NEXT;
		break;
	case LIT:
		PUSH disk[TORS++];
		NEXT;
		break;
	case PUSNXT:
		PUSH disk[NTORS++];
		NEXT;
		break;
	case BRANCH:
		TORS += disk[TORS];
		NEXT;
		break;
	case PDROP:
		DROP;
		NEXT;
		break;
	case TOR:
		w = TORS;
		TORS = TOS;
		RPUSH w;
		DROP;
		NEXT;
		break;
	case FROMR:
		PUSH NTORS;
		w = TORS;
		RDROP;
		TORS = w;
		NEXT;
		break;
	case DUP:
		w = TOS;
		PUSH w;
		NEXT;
		break;
	case SWAP:
		w = TOS;
		TOS = NTOS;
		NTOS = w;
		NEXT;
		break;
	case ROT:
		w = TOS;
		TOS = disk[(*tosp)+2];
		disk[(*tosp)+2] = NTOS;
		NTOS = w;
		NEXT;
		break;
	case PLUS:
		TWOLEVEL(TOS + NTOS);
		break;
	case MINUS:
		TWOLEVEL(NTOS - TOS);
		break;
	case MULT:
		TWOLEVEL(TOS * NTOS);
		break;
	case DIV:
		TWOLEVEL(NTOS / TOS);
		break;
	case RSHIFT:
		NTOS >>= TOS;
		DROP;
		NEXT;
		break;
	case LSHIFT:
		NTOS <<= TOS;
		DROP;
		NEXT;
		break;
	case LESS:
		TWOLEVEL(NTOS < TOS ? -1 : 0);
		break;
	case GREAT:
		TWOLEVEL(NTOS > TOS ? -1 : 0);
		break;
	case EQL:
		NTOS = NTOS == TOS ? -1 : 0;
		DROP;
		NEXT;
		break;
	case EMIT:
		mputchar(TOS);
		DROP;
		NEXT;
		break;
	case PUTS:
		putnumstr(disk+TOS);
		DROP;
		NEXT;
		break;
	case ATOI:
		TOS = (int) strtol((char *)(disk+TOS+1),&s, 10);
		if(s == (char *)(disk+TOS)){ //this might fail, if so, we can cast the disk pointer to a char *
			DROP;
			puts("NO NUMBER FOUND");
		}
		NEXT;
		break;
	case PNUM:
		puts(itoa(TOS));
		DROP;
		NEXT;
		break;
	case AND:
		NTOS &= TOS;
		DROP;
		NEXT;
		break;
	case OR:
		NTOS |= TOS;
		DROP;
		NEXT;
		break;
	case XOR:
		NTOS ^= TOS;
		DROP;
		NEXT;
		break;
	case NOT:
		TOS = ~TOS;
		NEXT;
		break;
	default: //this should be unreachable
		mputs("execute fallthrough: ");
		puts(itoa(x));
		dumpstack(3,tosp);
		dumpstack(5,rsp);
		dumpstack(15, disk+(*link));
		mputs("IP = ");
		puts(itoa(IP));
		*rsp = DSIZE-(RSSIZE+STSIZE+1);
		IP = cs;
	}
}

void pinit() {
	for(int i = DOCOL; i <= NOT; i++) {
		switch(i){
		case DOCOL:
		case LIT:
			enter(i);
			break;
		default:
			intern(i, 0);
		}
		primaddr[i] = *dict - 1;
	}
}

void finit(){
	//push the address of the next open cell to the stack
	COLON(here);
	COMPPRIM(LIT);
	enter(0);
	COMPPRIM(PEEK);
	COMPPRIM(EXIT);
	//compile TOS
	COLON(comptos);
	enter(here);
	COMPPRIM(POKE);
	enter(here);
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(PLUS);
	COMPPRIM(LIT);
	enter(0);
	COMPPRIM(POKE);
	COMPPRIM(EXIT);
	//compiletime lit
	intern(DOCOL, 0);
	int lit = *dict-1;
	COMPPRIM(LIT);
	COMPPRIM(LIT);
	enter(comptos);
	COMPPRIM(EXIT);
	//turn a nonzero value to -1, and keep zero values
	COLON(logify);
	COMPPRIM(LIT);
	enter(0);
	COMPPRIM(EQL);
	COMPPRIM(NOT);
	COMPPRIM(EXIT);
	//branch if false
	COLON(notbranch);
	COMPPRIM(LIT);  //I could have used logify and then not, but there's an extra 'not' in logify, so for efficiency I just implemented it inline.
	enter(0);
	COMPPRIM(EQL);
	COMPPRIM(PUSNXT);
	COMPPRIM(AND); //compute branch value depending on the boolean found on the stack.
	COMPPRIM(FROMR);
	COMPPRIM(PLUS);
	COMPPRIM(TOR);
	//add the branch value to the return value and store it back on the return stack, overwriting the old value.
	COMPPRIM(EXIT);
	//peek xt
	COLON(peekxt);
	COMPPRIM(LIT);
	enter(127);
	COMPPRIM(WORD);
	COMPPRIM(DUP);
	enter(comptos);
	COMPPRIM(FIND);
	enter(here);
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(MINUS);
	COMPPRIM(PEEK);
	COMPPRIM(LIT);
	enter(0);
	COMPPRIM(POKE);
	COMPPRIM(EXIT);
	//execute xt
	COLON(excut);
	int extspace = *dict + 3;
	COMPPRIM(LIT);
	enter(extspace);
	COMPPRIM(POKE);
	enter(0);
	COMPPRIM(EXIT);
	//interpret
	intern(DOCOL, -1);
	int intloop = *dict-1;
	enter(peekxt);
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(EQL);
	COMPPRIM(NOT);
	IF(intfound);
	enter(excut);
	ELSE(intfound);
	COMPPRIM(ATOI);
	THEN(intfound);
	COMPPRIM(LIT);
	enter((int)'\n');
	COMPPRIM(LIT);
	enter((int)'k');
	COMPPRIM(LIT);
	enter((int)'o');
	COMPPRIM(EMIT);
	COMPPRIM(EMIT);
	COMPPRIM(EMIT);
	COMPPRIM(FROMR);
	COMPPRIM(PDROP);
	enter(intloop);
	//compile
	COLON(comploop);
	enter(peekxt);
	COMPPRIM(DUP);
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(EQL);
	COMPPRIM(NOT);
	IF(compfound);
	IF(compimm);
	enter(excut);
	ELSE(compimm);
	enter(comptos);
	ELSE(compfound);
	COMPPRIM(PDROP);
	COMPPRIM(LIT);
	COMPPRIM(LIT);
	enter(comptos);
	COMPPRIM(ATOI);
	enter(comptos);
	THEN(compfound); THEN(compimm);
	COMPPRIM(FROMR);
	COMPPRIM(PDROP);
	enter(comploop);
	//colon compiler
	COLON(colon);
	enter(here);
	COMPPRIM(LIT);
	enter(3);
	COMPPRIM(DUP);
	COMPPRIM(PEEK);
	enter(comptos);
	COMPPRIM(POKE);
	COMPPRIM(LIT);
	enter(127);
	COMPPRIM(WORD);
	COMPPRIM(LIT);
	enter(0);
	COMPPRIM(DUP);
	enter(comptos);
	enter(comptos);
	COMPPRIM(EXIT);
	//cold start to setup interpreter
	mputs("cs @ ");
	puts(itoa(*dict));
	int coldstart = *dict;
	enter(DOCOL);
	enter(intloop);

	IP=coldstart;
	cs=coldstart;
}

int main() {
	pinit();
	finit();
	while(1) {
		execute(disk[IP]);
	}
}
