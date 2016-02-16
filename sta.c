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
#define THEN(BNAME) disk[BNAME]=*dict-BNAME-1

int disk[DSIZE] = {4, DSIZE-(RSSIZE+STSIZE+1), DSIZE-1, 0},
    *dict = disk, *rsp = disk+1, *tosp = disk+2,
    *link = disk+3, w, IP, primaddr[NOT+1];

char itoabuf[10] = {'\0'};

int mputs(char *s){
	return fputs(s, stdout);
}
int scant(char c, char *s) {  
	for( int i = 0; ; s++, i++) {
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
				return i;
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
	putnumstr(disk+w);
	mputs(": interning ");
	mputs(itoa(x));
	putchar(32);
	*dict += slen/PACK + 1;
	enter(imm);
	mputs("@ ");
	puts(itoa(*dict));
	enter(x);
}

void execute(int x) {
	char *s;
	mputs("executing ");
	puts(itoa(x));
	switch (x) {
	case DOCOL:
		mputs("DOCOL IP=");
		puts(itoa(IP));
		w = ++IP;
		RPUSH ++IP;
		IP = disk[w];
		break;
	case IMMEDIATE:
		disk[*link+disk[*link+1]+2] = -1;
		NEXT;
		break;
	case KEY:
		PUSH getchar();
		NEXT;
		break;
	case WORD:
		puts("WORD\n");
		w = *dict;
		enter(0);
		int slen = scant((char) TOS, (char *)(disk + (*dict)));
		DROP;
		disk[w] = slen;
		PUSH w;
		*dict = w;
		NEXT;
		break;
	case FIND:
		mputs("FIND ");
		w = *link;
		mputs("w = ");
		puts(itoa(w));
		while (!(streql(disk+TOS, (disk+w+1))) && w) {
			w = disk[w];
			mputs("w = ");
			puts(itoa(w));
		}
		if (!w) {
			PUSH 1;
		} else {
			for (int i = 0; i < 5; i++) {
				mputs(itoa(w+i));
				mputs(": ");
				puts(itoa(disk[w+i]));
			}
			putnumstr(disk+w+1);
			TOS = w + 2 + ((dict[w+1])/PACK + 2);
			w = dict[TOS-1];
			PUSH w;
		}
		puts(itoa(TOS));
		NEXT;
		break;
	case EXIT:
		mputs("EXIT\n");
		RDROP;
		NEXT;
		break;
	case PEEK:
		mputs("PEEKING ");
		mputs(itoa(TOS));
		TOS = disk[TOS];
		mputs("->");
		puts(itoa(TOS));
		NEXT;
		break;
	case POKE:
		mputs("POKING ");
		mputs(itoa(NTOS));
		mputs("->");
		puts(itoa(TOS));
		disk[TOS] = NTOS;
		DROP;
		DROP;
		NEXT;
		break;
	case LIT:
		mputs("LIT TORS = ");
		puts(itoa(TORS));
		PUSH disk[TORS++];
		mputs("TOS = ");
		puts(itoa(TOS));
		NEXT;
		break;
	case PUSNXT:
		mputs("previous TORS = ");
		puts(itoa(TORS));
		PUSH disk[NTORS++];
		mputs("PUSHNEXT TOS = ");
		puts(itoa(TOS));
		mputs("TORS = ");
		puts(itoa(TORS));
		NEXT;
		break;
	case BRANCH:
		puts("BRANCH");
		w = disk[TORS++];
		mputs("BVALUE=");
		puts(itoa(w));
		TORS = TORS+w;
		NEXT;
		break;
	case PDROP:
		puts("DROP");
		DROP;
		NEXT;
		break;
	case TOR:
		puts(">R");
		w = TORS;
		TORS = TOS;
		RPUSH w;
		DROP;
		NEXT;
		break;
	case FROMR:
		puts("<R");
		PUSH NTORS;
		w = TORS;
		RDROP;
		TORS = w;
		NEXT;
		break;
	case DUP:
		puts("DUP");
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
		mputs(itoa(NTOS));
		mputs(" PLUS ");
		puts(itoa(TOS));
		TWOLEVEL(TOS + NTOS);
		break;
	case MINUS:
		mputs(itoa(NTOS));
		mputs(" MINUS ");
		puts(itoa(TOS));
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
		mputs(itoa(NTOS));
		mputs(" EQL? ");
		puts(itoa(TOS));
		NTOS = NTOS == TOS ? -1 : 0;
		DROP;
		NEXT;
		break;
	case EMIT:
		putchar(TOS);
		DROP;
		NEXT;
		break;
	case ATOI:
		puts("ATOI");
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
		mputs(itoa(NTOS));
		mputs(" AND ");
		puts(itoa(TOS));
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
		puts("NOT");
		TOS = ~TOS;
		NEXT;
		break;
	default: //this should be unreachable
		puts("execute fallthrough!\n");
		exit(1);
	}
}

void pinit() {
	for(int i = DOCOL; i <= NOT; i++) {
		intern(i, i == IMMEDIATE ? -1 : 0);
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
	COMPPRIM(LIT);
	enter(0);
	COMPPRIM(PEEK);
	COMPPRIM(POKE);
	COMPPRIM(EXIT);
	//computebranch
	COLON(compbran);
	COMPPRIM(AND);
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
	enter(compbran); //compute branch value depending on the boolean found on the stack.
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
	COMPPRIM(FIND);
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
	COMPPRIM(DUP); 
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(EQL);
	COMPPRIM(NOT);
	IF(intfound);
	COMPPRIM(NOT);
	IF(intnotimm);
	enter(excut);
	ELSE(intfound);
	COMPPRIM(PDROP);
	COMPPRIM(ATOI);
	THEN(intnotimm); THEN(intfound);
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
	COMPPRIM(WORD);
	COMPPRIM(DUP);
	COMPPRIM(PEEK);
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(PLUS);
	COMPPRIM(PLUS);
	COMPPRIM(LIT);
	enter(0);
	COMPPRIM(POKE);
	COMPPRIM(LIT);
	enter(0);
	enter(comptos);
	enter(comploop);
	//cold start to setup interpreter
	int coldstart = *dict;
	enter(DOCOL);
	enter(intloop);

	IP=coldstart;
}

void sanitycheck() {
	for (int i = 0; i < 3; i++) {
		if (!(0 <= disk[i] && disk[i] < DSIZE)){
			printf("ERROR: reference out of bounds: disk[%d]\n", i);
			exit(1);
		}
	}
	if (!(*dict < DSIZE-RSSIZE-STSIZE)) {
		puts("ERROR: out of dictionary space!\n");
		exit(1);
	}
	if (!(DSIZE-RSSIZE-STSIZE <= *rsp)) {
		puts("ERROR: improper return stack size!\n");
		exit(1);
	}
	if (!(DSIZE-RSSIZE-STSIZE < *tosp)) {
		puts("ERROR: improper stack size!\n");
		exit(1);
	}
	if (*rsp > *tosp) {
		puts("ERROR: stack intersection!\n");
		exit(1);
	}
}

void cycle() {
	//sanitycheck();
	execute(disk[IP]);
	mputs("STACK = ");
	mputs(itoa(TOS));
	mputs(", ");
	mputs(itoa(NTOS));
	mputs(", ");
	puts(itoa(disk[(*tosp) + 2]));
	puts("");
}

void main() {
	pinit();
	finit();
	while(1) {
		cycle();
	}
}
