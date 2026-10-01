#include <stdio.h>


int main()
{
    int arr[5];
    for (int i = 0; i <5; i++){
        scanf("%d", &arr[i]);
    }

    int z = 0;
    for(int i = 0; i < 5; i++)
    {
        z = arr[i] + z;
    }
    double p = 0;
    p = z / 5;

    printf("z = %d, p = %lf\n" , z, p);

    getchar();
    getchar();
    return 0;
}

