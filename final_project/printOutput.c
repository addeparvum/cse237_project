#include <stdio.h>

int main(){
	int condition = 1;
	while(condition){
		int k;
		scanf("%d",&k);
		switch(k){
			case 0:
				printf("I am 1\n");
				break;
			case 1:
				printf("I am 0\n");
				break;
			default:
				condition = 0;
				break;
		}
	}
	return 0;
}

