#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#define SHM_SIZE (250)
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int create_shm()
{
    key_t key = ftok("mypath", 100);
    return shmget(key, 1024, IPC_CREAT|0666);
}
static void destroy_shm(int shmid)
{
    char* str = (char *)shmat(shmid, (void*)0, 0);
    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL); 
}
static void* write(void* shm_id)
{
    printf("Write data: ");
    int shmid = *(int*)shm_id;
    char *str = (char *)shmat(shmid, (void*)0, 0);
    pthread_mutex_lock(&lock);
    gets(str);
    printf("Data written in memory: %s\n", str);
    pthread_mutex_unlock(&lock);

    pthread_cond_signal(&cond1);
    return NULL;
}

static void* read(void* shm_id)
{
    int shmid = *(int*)shm_id;
    pthread_cond_wait(&cond1, &lock);
    char *str = (char *)shmat(shmid, (void*)0, 0);
    printf("Data read from memory: %s\n", str);
    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);
    return NULL;
}

int main(int argc, char*argv[])
{
    pthread_t write_thread, read_thread;
    int shmid = create_shm();
    if(pthread_create(&write_thread, NULL, write, &shmid))
    {
        perror("Write thread create failed");
    }
    if(pthread_create(&read_thread, NULL, read, &shmid))
    {
        perror("Read thread create failed");
    }
    pthread_join(read_thread, NULL);
    destroy_shm(shmid);
    return 0;
}