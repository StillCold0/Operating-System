#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>



int main(int argc, const char * argv[]) {
    
    printf("***proses oluşturma***\n");
    
    int n;
    pid_t PId;
    
    do{
        printf("0'dan büyük bir sayı giriniz:");
        scanf("%d", &n);
    }while(n<=0);
    
    
    PId = fork();
    
    printf("fork değeri:%d %s",PId,"\n");
    
    
    if(PId < 0){
        printf("fork hatası");
    }
    else if(PId==0){
        printf("child:%d",n);
            while(n > 1){
                if(n % 2 == 0){
                    n = n/2;
                    printf("\ndeğer:%d",n);
                }
                else if(n % 2 == 1){
                    n = n*3+1;
                    printf("\ndeğer:%d",n);
                }
            }
        
        if(n == 1){
            printf("\nprogram sonlandırılıyor...\n");
            printf("program çıktısı:%d %s",n,"\n");
        }
    }
    else{
        wait(NULL);
        printf("\nchild çıkışı\n\n");
    }
    
    return 0;
}
