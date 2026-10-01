#include<stdio.h>

int main(){
    int a = 0;
    int b = 0;
    int c = 0;
    scanf("%d", &a);
    scanf("%d", &b);
    c = a * b;
    printf("%d",c);
    getchar();   // 吃掉输入 55 后留在缓冲区的回车符
    getchar();   // 等待用户按回车再退出
    return 0;

}