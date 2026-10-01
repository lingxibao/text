#include <stdio.h>

int main(){
    int arr[5];
    int sum = 0;
    for(int i = 0; i < 5; i++){
        scanf("%d", &arr[i]);

    }
    for (int i = 0; i < 5; i++){
        sum = sum + arr[i];
    }
    double avg;
    avg = 1.0 * sum / 5;

    printf("sum = %d avg = %lf", sum, avg);
    return 0;
}