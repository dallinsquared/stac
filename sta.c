#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DOCOL 0
#define KEY 1
#define WORD 2 
#define FIND 3 
#define EXIT 4
#define PEEK 5
#define POKE 6
#define LIT  7
#define PDROP 8
#define SWAP 9
#define ROT  10
#define PLUS 11
#define MINUS 12
#define MULT 13
#define DIV 14
#define MOD 15
#define RSHIFT 16
#define LSHIFT 17 
#define LESS 18
#define GREAT 19
#define EQL 20
#define EMIT 21
#define AND 22
#define OR 23
#define XOR 24

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
#define DROP --(*tosp)
#define RDROP --(*rsp)
#define TWOLEVEL(EFFECT) NTOS=EFFECT;DROP;NEXT

const int pack = sizeof int / sizeof char;
const int diff = sizeof int - (sizeof char * pack);

int disk[DSIZE] = {3, DSIZE-(RSSIZE+STSIZE+1), DSIZE-1},
    *dict = disk, *rsp = disk+1, *tosp = disk+2,
    link = 0, w, IP;

int scant(char c, char *s) {
	for( int i = 0; ; s++, i++) {
		*s = getchar;
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
	}
}

void enter(int x){
	disk[(*dict)++] = x;
}

void intern(int x) {
	enter(link);
	link = *dict-1;
	w = *dict;
	enter(0);
	int slen = scant((char) 127, (char *)(disk+(*dict)));
	int ilen = slen/pack + (slen%pack) ? 1 : 0;
	disk[w] = ilen;
	*dict += ilen;
	enter(x);
}

void init() {
	for(int i = DOCOL; i <= XOR; i++) {
		intern(i);
	}
}

void execute(int x) {
	switch x {
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
			PUSH 0;
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
		TWOLEVEL(NTOS >> TOS);
		break;
	case LSHIFT:
		TWOLEVEL(NTOS << TOS);
		break;
	case LESS:
		TWOLEVEL(NTOS < TOS);
		break;
	case GREAT:
		TWOLEVEL(NTOS > TOS);
		break;
	case EQL:
		TWOLEVEL(NTOS == TOS);
		break;
	case EMIT:
		putchar(TOS);
		DROP;
		NEXT;
		break;

