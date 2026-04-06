#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int buffer_has_data = 0; 


void* consumer(void* arg) {
    printf("Consumer: Toi dang cho du lieu...\n");
    
    pthread_mutex_lock(&lock);
    
    while (buffer_has_data == 0) {
        printf("Consumer: Khay trong, di ngu day!\n");
        pthread_cond_wait(&cond, &lock); 
    }
    
    printf("Consumer: Da duoc danh thuc! Thay du lieu roi, dang xu ly...\n");
    buffer_has_data = 0; 
    
    pthread_mutex_unlock(&lock);
    return NULL;
}


void* producer(void* arg) {
    sleep(2); 
    pthread_mutex_lock(&lock);
    
    printf("Producer: Da tao xong du lieu!\n");
    buffer_has_data = 1;

    pthread_cond_signal(&cond); 
    printf("Producer: Da gui tin hieu danh thuc Consumer.\n");
    
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t con_thread, pro_thread;

    pthread_create(&con_thread, NULL, consumer, NULL);
    sleep(1); 
    
    pthread_create(&pro_thread, NULL, producer, NULL);

    pthread_join(con_thread, NULL);
    pthread_join(pro_thread, NULL);

    return 0;
}