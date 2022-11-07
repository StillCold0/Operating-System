
#include <pthread.h>        // POSIX threads oluşturur.
#include <time.h>            // Rastgele bir süre bekletir.
#include <unistd.h>            //Thread, belirtilen saniye sayısı boyunca uykuyu çağırır.
#include <semaphore.h>        //Semafor oluşturmak için
#include <stdlib.h>
#include <stdio.h>            //Input Output

pthread_t *Ogrenciler;        // Öğrenci olarak çalışan N iş parçacığı.
pthread_t AG;                // AG için Ayrı Konu.

int sandalyeSayaci = 0;
int simdikiIndex = 0;

//Semaforlar ve Mutex Kilidi.
sem_t AG_uyuyor;
sem_t ogrenciSem;
sem_t sandalyeSem[3];
pthread_mutex_t sandalyeErisimi;

//Bildirilen İşlevler
void *AG_aktivitesi();
void *ogrenci_aktivitesi(void *threadID);

int main(int argc, char* argv[])
{
    int ogrenci_sayisi;        //öğrenci dizileri oluşturmak için kullanıcıdan alınan bir değişken. Varsayılan 5 öğrenci dizisidir.
    int id;
    srand(time(NULL));

    //Mutex Lock ve Semaforları Başlatma.
    sem_init(&AG_uyuyor, 0, 0);
    sem_init(&ogrenciSem, 0, 0);
    for(id = 0; id < 3; ++id)            //3 semafordan oluşan sandalye dizisi.
        sem_init(&sandalyeSem[id], 0, 0);

    pthread_mutex_init(&sandalyeErisimi, NULL);
    
    if(argc<2)
    {
        printf("Öğrenci Sayısı Belirtilmemiş. Varsayılan (3) öğrenciyi kullanma.\n");
        ogrenci_sayisi = 3;
    }
    else
    {
        printf("Belirtilen Öğrenci Sayısı. %d oluşturuluyor.\n", ogrenci_sayisi);
        ogrenci_sayisi = atoi(argv[1]);
    }
    
    //Öğrenciler için bellek ayırma
    Ogrenciler = (pthread_t*) malloc(sizeof(pthread_t)*ogrenci_sayisi);

    //AG dizisi ve N Öğrenci dizisi oluşturma.
    pthread_create(&AG, NULL, AG_aktivitesi, NULL);
    for(id = 0; id < ogrenci_sayisi; id++)
        pthread_create(&Ogrenciler[id], NULL, ogrenci_aktivitesi,(void*) (long)id);

    //AG dizisi ve N Öğrenci dizisi bekleniyor.
    pthread_join(AG, NULL);
    for(id = 0; id < ogrenci_sayisi; id++)
        pthread_join(Ogrenciler[id], NULL);

    //Boş ayrılmış bellek
    free(Ogrenciler);
    return 0;
}

void *AG_aktivitesi()
{
    while(1)
    {
        sem_wait(&AG_uyuyor);        //AG uyuyor
        //printf("AG bir öğrenci tarafından uyandırıldı.\n");

        while(1)
        {
            // kilit
            pthread_mutex_lock(&sandalyeErisimi);
            if(sandalyeSayaci == 0)
            {
                //sandalyeler boşsa, döngüyü kırıyoruz.
                pthread_mutex_unlock(&sandalyeErisimi);
                break;
            }
            //AG sıradaki öğrenciyi sandalyeye oturtur.
            sem_post(&sandalyeSem[simdikiIndex]);
            sandalyeSayaci--;
            printf("Öğrenci sandalyeden kalktı. Kalan Sandalyeler %d\n", 3 - sandalyeSayaci);
            simdikiIndex = (simdikiIndex + 1) % 3;
            pthread_mutex_unlock(&sandalyeErisimi);
            // kilit kalktı

            printf("\t AG öğrenciye yardım ediyor.\n");
            sleep(5);
            sem_post(&ogrenciSem);
            usleep(1000);
        }
    }
}

void *ogrenci_aktivitesi(void *threadID)
{
    int programZamani;

    while(1)
    {
        printf("Öğrenci %ld programlama ödevi yapıyor.\n", (long)threadID);
        programZamani = rand() % 10 + 1;
        sleep(programZamani);       //Rastgele süre için uyku.

        printf("%ld adlı öğrencinin AG'nin yardımına ihtiyacı var\n", (long)threadID);
        
        pthread_mutex_lock(&sandalyeErisimi);
        int sayac = sandalyeSayaci;
        pthread_mutex_unlock(&sandalyeErisimi);

        if(sayac < 3)       //Öğrenci bir sandalyeye oturmaya çalıştı.
        {
            if(sayac == 0)        //Öğrenci ilk boş sandalyeye oturursa, ag'yı uyandırın.
                sem_post(&AG_uyuyor);
            else
                printf("Öğrenci %ld bir sandalyeye oturdu ve AG'nin işinin bitmesini bekledi. \n", (long)threadID);

            // lock
            pthread_mutex_lock(&sandalyeErisimi);
            int index = (simdikiIndex + sandalyeSayaci) % 3;
            sandalyeSayaci++;
            printf("Öğrenci sandalyeye oturdu. Kalan Sandalyeler: %d\n", 3 - sandalyeSayaci);
            pthread_mutex_unlock(&sandalyeErisimi);
            // unlock

            sem_wait(&sandalyeSem[index]);        //Öğrenci sandalyesini terk eder.
            printf("\t Öğrenci %ld, AG'dan yardım alıyor. \n", (long)threadID);
            sem_wait(&ogrenciSem);        //Öğrenci bir sonrakine gitmek için bekler.
            printf("Öğrenci %ld AG odasından ayrıldı.\n",(long)threadID);
        }
        else
            printf("Öğrenci %ld başka bir zamanda dönecek. \n", (long)threadID);
            //Öğrenci oturacak sandalye bulamadıysa.
    }
}
