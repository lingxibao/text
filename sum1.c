#include <stdio.h>
#include <stdlib.h>

int main(int argc,char * argv[]){

	int n;

	if (argc > 1){
	n = atoi(argv[1]);
	}else{
	printf("请输入 n:");
	scanf("%d",&n);
		
	}

	if(n < 1){
	printf("n 应为正整数\n");

	return 1;
	
	}
	
	long long sum = 0;
	for (int i = 1; i <= n; i++)
	{
	sum += i;
	}
	printf("1 + 2 + 3 + ... +%d = %lld\n", n, sum);

	return 0;

}
