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
#define BRANCH 9
#define PDROP 10
#define SWAP 11
#define ROT  12
#define PLUS 13
#define MINUS 14
#define MULT 15
#define DIV 16
#define MOD 17
#define RSHIFT 18
#define LSHIFT 19 
#define LESS 20
#define GREAT 21
#define EQL 22
#define EMIT 23
#define NEG 24
#define AND 25
#define OR 26
#define XOR 27
#define NOT 28

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

const int pack = sizeof int / sizeof char;
const int diff = sizeof int - (sizeof char * pack);

int disk[DSIZE] = {3, DSIZE-(RSSIZE+STSIZE+1), DSIZE-1},
    *dict = disk, *rsp = disk+1, *tosp = disk+2,
    link = 0, w, IP;

int scant(char c, char *s) {
	for( int i = 0; ; s++, i++) {
		*s = getchar();
		if (*s == c) {
			*s = '\0';
			return i;
		}
		if ((int) c == 127) {
			switch *s {
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
	enter(link);
	link = *dict-1;
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
		intern(i, i == IMMEDIATE);
	}
}
void finit(){
	//flag for compiling
	intern(DOCOL, 0);
	enter(LIT);
	enter(0);
	enter(EXIT);
	//reference to return stack
	intern(DOCOL, 0);
	enter(LIT);
	enter(1);
	enter(EXIT);
	//reference to dictionary
	intern(DOCOL, 0);
	enter(LIT);
	enter(0);
	enter(EXIT);
	//reference to stack
	intern(DOCOL, 0);
	enter(LIT);
	enter(2);
	enter(EXIT);
	//compile TOS
	intern(DOCOL, 0);
	enter(LIT);
	enter(0);
	enter(PEEK);
	enter(POKE);
	enter(EXIT);
	//computebranch
	intern(DOCOL, 0);
	int compbran = *dict - 1;
	enter(LIT);
	enter(1);  
	enter(MINUS);
	enter(MULT);
	enter(LIT);
	enter(1);
	enter(MINUS);
	enter(NEG);
	enter(EXIT);

}


void execute(int x) {
	switch x {
	case DOCOL:
		w = ++IP;
		RPUSH ++IP;
		IP = disk[w];
		break;
	case IMMEDIATE:
		disk[link+disk[link+1]+2] = 1;
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
		w = link;
		while (strcmp((char *)(disk+TOS), (char *)(disk+w+1))) {
			w = disk[w];
		}
		if !w {
			PUSH -1;
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
		PUSH disk[++IP];
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
	case MOD:
		TWOLEVEL(NTOS % TOS);
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
	case NEG:
		TOS = ~TOS + 1;
		NEXT;
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
	case NOT:
		TOS = ~TOS;
		NEXT;
		break;
	}
}

void sanitycheck() {
	for (int i = 0; i < 3; i++) {
		if !(0 <= disk[i] && disk[i] < DSIZE){
			printf("ERROR: reference out of bounds: disk[%d]\n", i);
			exit(1);
		}
	}
	if !(*dict < DSIZE-RSSIZE-STSIZE) {
		printf("ERROR: out of dictionary space!\n");
		exit(1);
	}
	if !(DSIZE-RSSIZE-STSIZE <= *rsp) {
		printf("ERROR: improper return stack size!\n");
		exit(1);
	}
	if !(DSIZE-RSSIZE-STSIZE < *tosp) {
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
	init()
