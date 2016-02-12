#include <stdio.h>
#define PRIMS "primitives"
char s[15] = {'\0'}, si[4] = {'0', '\0'}, *sa[4] = {"#define ", s, si, NULL};
int mputs(char **z){
	for(int i = 0; z[i]; i++) {
		fputs(z[i], stdout);
	}
	return 0;
}
int mgets(char *z, int i){
	for(int j = 0; j < (i-1) ; z++,j++){
		*z = getchar();
		if ((*z == '\n') || (*z = EOF)){
			*z = '\0';
			return j;
		}
	}
	z[i-1] = '\0';
	return i-1;
}

char *itoa(int i){
        char *p = si;
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
        return si;
}

int main(){
	int e = 0;
	int i = 0;
	while(1){
		itoa(i);
		if (mgets(s, 15) < 0){
			e = -1;
		}
		mputs(sa);
		if( e < 0 ){
			return 0;
		}
	}
}
