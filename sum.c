#include <stdio.h>

int main(){
    int a = 0;
    int b = 0;
  
    printf("请输入第一个数");
    scanf("%d", &a);

    printf("请输入第二个数");
    scanf("%d", &b);

    int c = a + b;
    printf("相加后结果%d", c);

    getchar();   // 吃掉输入 55 后留在缓冲区的回车符
    getchar();   // 等待用户按回车再退出
    return 0;

}