#include <stdio.h>

int main(){
	int pack = sizeof (int) / sizeof (char);
	int diff = sizeof (int) - (sizeof (char) * pack);
	printf("int pack = %d;\nint diff = %d;\n", pack, diff);
	return 0;
}
