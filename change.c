#include <stdio.h>
 
int main(){
 
    int price = 0;
    printf("请输入金额（元）");
    scanf("%d",&price);
    
    int change = 100 - price;
    printf("找您%d元\n", change );

    getchar();   // 吃掉输入 55 后留在缓冲区的回车符
    getchar();   // 等待用户按回车再退出

    return 0;
 
}