#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DOCOL 0
#define IMMEDIATE 1
#define KEY 2
#define WORD 3 
#define FIND 4 
#define EXIT 5
#define PEEK 6
#define POKE 7
#define LIT  8
#define PUSNXT 9
#define BRANCH 10
#define PDROP 11
#define TOR 12
#define FROMR 13
#define DUP 14
#define SWAP 15
#define ROT  16
#define PLUS 17
#define MINUS 18
#define MULT 19
#define DIV 20
#define RSHIFT 21
#define LSHIFT 22 
#define LESS 23
#define GREAT 24
#define EQL 25
#define EMIT 26
#define ATOI 27
#define AND 28
#define OR 29
#define XOR 30
#define NOT 31

#define DSIZE 500
#define RSSIZE 50
#define STSIZE 50

#define NEXT IP=disk[(*rsp)++]
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
#define IF 

int pack;
int diff;

int disk[DSIZE] = {3, DSIZE-(RSSIZE+STSIZE+1), DSIZE-1},
    *dict = disk, *rsp = disk+1, *tosp = disk+2,
    *link = disk+3, w, IP, primaddr[NOT+1];

char itoabuf[10] = {'\0'};
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

void enter(int x){
	disk[(*dict)++] = x;
}

void intern(int x, int imm) {
	enter(*link);
	*link = *dict-1;
	w = *dict;
	enter(0);
	int slen = scant((char) 127, (char *)(disk+(*dict)));
	int ilen = slen/pack + (slen%pack) ? 1 : 0;
	disk[w] = ilen;
	*dict += ilen;
	enter(imm);
	enter(x);
}

void pinit() {
	for(int i = DOCOL; i <= NOT; i++) {
		intern(i, i == IMMEDIATE ? -1 : 0);
		primaddr[i] = *dict - 1;
	}
}
void finit(){
	//flag for compiling
	COLON(compileq);
	COMPPRIM(LIT);
	int compflag = *dict;
	enter(0);
	COMPPRIM(EXIT);
	//enter compile mode
	COLON(compilemode); 
	COMPPRIM(LIT);
	enter(-1);
	COMPPRIM(LIT);
	enter(compflag);
	COMPPRIM(POKE);
	COMPPRIM(EXIT);
	//exit compile mode
	COLON(interpmode);
	COMPPRIM(LIT);
	enter(0);
	COMPPRIM(LIT);
	enter(compflag);
	COMPPRIM(POKE);
	COMPPRIM(EXIT);
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
	COMPPRIM(LIT);
	enter(1);  
	COMPPRIM(MINUS);
	COMPPRIM(AND);
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(PLUS);
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
	COMPPRIM(LIT);	//add the branch value to the return value and store it back on the return stack, overwriting the old value.
	enter(1);
	COMPPRIM(PEEK);
	COMPPRIM(PEEK);
	COMPPRIM(PLUS);
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(PEEK);
	COMPPRIM(POKE);
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
	COMPPRIM(TOR);
	COMPPRIM(EXIT);
	//interpret
	intern(DOCOL, -1);
	int intloop = *dict-1;
	enter(peekxt);
	COMPPRIM(DUP); //dup not primitive...
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(EQL);
	COMPPRIM(NOT);
	enter(notbranch);
	int intfounbranch = *dict;
	enter(0); 
	COMPPRIM(NOT);
	enter(notbranch);
	int intimmpatch = *dict;
	enter(0);
	enter(excut);
	COMPPRIM(BRANCH);
	int intpushint = *dict;

	int intrecur = *dict;
	dict[intfounbranch] = intrecur - intfounbranch;
	dict[intimmpatch] = intrecur - intimmpatch;
	COMPPRIM(FROMR);
	COMPPRIM(DROP);
	enter(intloop);
	//compile
	COLON(comploop);
	enter(peekxt);
	COMPPRIM(DUP);
	COMPPRIM(LIT);
	enter(1);
	COMPPRIM(EQL);
	COMPPRIM(NOT);
	enter(notbranch);
	int compfounpatch = *dict;
	enter(0);
	enter(notbranch);
	int compnotimmbran = *dict;
	enter(0);
	enter(excut);
	COMPPRIM(BRANCH);
	int compimmelse = *dict;
	enter(0);
	dict[compnotimmbran] = compimmelse - compnotimmbran + 1;
	enter(comptos);
	int comprecur = *dict;
	dict[compfounpatch] = comprecur - compfounpatch;
	dict[compimmelse] = comprecur - compimmelse;
	COMPPRIM(FROMR);
	COMPPRIM(DROP);
	enter(comploop);

	IP=intloop;
}

char* itoa(int i){
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
	do{ //Move back, inserting digits as u go
		*--p =(char)((i%10)+48);
		i = i/10;
	} while(i);
	return itoabuf;
}


void execute(int x) {
	switch (x) {
	case DOCOL:
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
		w = *dict;
		enter(0);
		int slen = scant((char) disk[(*tosp)++], (char *)(disk + (*dict)));
		int ilen = slen/pack + (slen%pack) ? 1 : 0;
		disk[w] = ilen;
		PUSH *dict;
		*dict += ilen;
		NEXT;
		break;
	case FIND:
		w = *link;
		while (strcmp((char *)(disk+TOS), (char *)(disk+w+1))) {
			w = disk[w];
		}
		if (!w) {
			PUSH 1;
		} else {
			TOS = w + (dict[w+1]);
			PUSH dict[TOS+1];
		}
		NEXT;
		break;
	case EXIT:
		RDROP;
		IP = TORS++;
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
		PUSH disk[NTORS++];
		NEXT;
		break;
	case PUSNXT:
		PUSH disk[NTORS++];
		NEXT;
		break;
	case BRANCH:
		TORS = TORS+disk[++IP];
		NEXT;
		break;
	case PDROP:
		DROP;
		NEXT;
		break;
	case TOR:
		RPUSH TOS;
		DROP;
		NEXT;
		break;
	case FROMR:
		PUSH TORS;
		RDROP;
		NEXT;
		break;
	case DUP:
		PUSH TOS;
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
		TWOLEVEL(NTOS == TOS ? -1 : 0);
		break;
	case EMIT:
		putchar(TOS);
		DROP;
		NEXT;
		break;
	case ATOI:
		TOS = atoi((char *)(disk+TOS));
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
	}
}

void sanitycheck() {
	for (int i = 0; i < 3; i++) {
		if (!(0 <= disk[i] && disk[i] < DSIZE)){
			printf("ERROR: reference out of bounds: disk[%d]\n", i);
			exit(1);
		}
	}
	if (!(*dict < DSIZE-RSSIZE-STSIZE)) {
		printf("ERROR: out of dictionary space!\n");
		exit(1);
	}
	if (!(DSIZE-RSSIZE-STSIZE <= *rsp)) {
		printf("ERROR: improper return stack size!\n");
		exit(1);
	}
	if (!(DSIZE-RSSIZE-STSIZE < *tosp)) {
		printf("ERROR: improper stack size!\n");
		exit(1);
	}
	if (*rsp > *tosp) {
		printf("ERROR: stack intersection!\n");
		exit(1);
	}
}

void cycle() {
	sanitycheck();
	execute(disk[IP]);
}

void main() {
	pack = sizeof (int) / sizeof (char);
	diff = sizeof (int) - (sizeof (char) * pack);
	pinit();
	finit();
	while(1) {
		cycle();
	}
}
