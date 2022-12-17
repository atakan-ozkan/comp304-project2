#include "queue.c"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#define TOTAL_ORDER 15
#ifdef __APPLE__
    #include <dispatch/dispatch.h>
    typedef dispatch_semaphore_t psem_t;
#else
    #include <semaphore.h> // sem_*
    typedef sem_t psem_t;
#endif
void psem_init(psem_t *sem, u_int32_t value);
void psem_wait(psem_t sem);
void psem_post(psem_t sem);

int simulationTime = 120;    // simulation time
int seed = 48324;               // seed for randomness
int emergencyFrequency = 30; // frequency of emergency gift requests from New Zealand
int timePassed = 0;
int currentOrderNumbers= 0;
int waitingForDelivery = 0;
int waitingForPackaging = 0;
int waitingForPainting = 0;
int waitingForAssembly = 0;
int waitingForQA =0;
int totalDeliveredOrder = 0;
int totalCreatedOrders = 0;
Queue *myQ;
psem_t semaphore;

void* ElfA(void *arg); // the one that can paint
void* ElfB(void *arg); // the one that can assemble
void* Santa(void *arg);
void* ControlThread(void *arg); // handles printing and queues (up to you)
void* CreateOrder();

// pthread sleeper function
int pthread_sleep (int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if(pthread_mutex_init(&mutex,NULL))
    {
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL))
    {
        return -1;
    }
    struct timeval tp;
    //When to expire is an absolute time, so get the current time and add it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;
    
    pthread_mutex_lock(&mutex);
    int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);
    
    //Upon successful completion, a value of zero shall be returned
    return res;
}


int main(int argc,char **argv){
    // -t (int) => simulation time in seconds
    // -s (int) => change the random seed
    for(int i=1; i<argc; i++){
        if(!strcmp(argv[i], "-t")) {simulationTime = atoi(argv[++i]);}
        else if(!strcmp(argv[i], "-s"))  {seed = atoi(argv[++i]);}
    }
    
    srand(seed); // feed the seed
    pthread_t th[TOTAL_ORDER];
    
    pthread_mutex_t mutex;
    
    myQ = ConstructQueue(TOTAL_ORDER);
    
    //pthread_mutex_init(&mutex,NULL);
    int index=0;
    int check =0;
    int wait= 0;
    
    psem_init(&semaphore,1);
    ControlThread(ElfA);
    ControlThread(ElfB);
    ControlThread(Santa);
    
    while(TRUE){
        printf("Total delivired ordes : %d // Total created orders: %d\n",totalDeliveredOrder,totalCreatedOrders);
        if(index< TOTAL_ORDER){
            CreateOrder(timePassed);
            index++;
            
        }
        timePassed++;
        pthread_sleep(1);
        if(totalCreatedOrders > 0 && totalCreatedOrders == totalDeliveredOrder){
            printf("At seeconds: %d  ::  ALL ORDERS ARE DELIVERED! \n",timePassed);
            break;
        }
    }
    //pthread_mutex_destroy(&mutex);

    // your code goes here
    // you can simulate gift request creation in here,
    // but make sure to launch the threads first

    return 0;
}
void* CreateOrder(int seconds){
    int num = rand() % 100;
    if (num < 90) {
        Task t;
        if (num > 50) {
            t.type = 1; // PACKAGING -> DELIVERY
            t.painting = -1;
            t.assembly = -1;
            t.qa =  -1;
            t.stage = 1;
            t.ID = currentOrderNumbers;
            waitingForPackaging++;
        }
        else if (num > 30) {
            t.type = 2; // PAINTING -> PACKAGING -> DELIVERY
            t.painting = 0;
            t.assembly = -1;
            t.qa =  -1;
            t.stage = 0;
            t.ID = currentOrderNumbers;
            waitingForPainting++;
        }
        else if(num > 10) {
            t.type = 3; // ASSEMBLY -> PACKAGING -> DELIVERY
            t.painting = -1;
            t.assembly = 0;
            t.qa =  -1;
            t.stage = 0;
            t.ID = currentOrderNumbers;
            waitingForAssembly++;
        }
        else if(num > 5) {
            t.type = 4; // PAINTING && QA -> PACKAGING -> DELIVERY
            t.painting = 0;
            t.assembly = -1;
            t.qa =  0;
            t.stage = 0;
            t.ID = currentOrderNumbers;
            waitingForPainting++;
            waitingForQA++;
        }
        else{
            t.type = 5; // ASSEMBLY && QA -> PACKAGING -> DELIVERY
            t.painting = -1;
            t.assembly = 0;
            t.qa =  0;
            t.stage = 0;
            t.ID = currentOrderNumbers;
            waitingForAssembly++;
            waitingForQA++;
        }
        t.delivery =0 ;
        t.packaging = 0;
        totalCreatedOrders++;

        printf("At Seconds : %d  ::  Order type %d is created! with ID %d\n",seconds,t.type,t.ID);
        Enqueue(myQ, t);
        currentOrderNumbers++;
    }
    else{
        printf("At Seconds : %d  ::  Order is not created!\n",seconds);
    }

    return 0;
}

void* ElfA(void *arg){ // PAINT AND PACKAGE
    while(TRUE) {
        struct Node_t *node;
        node= myQ->head;
        Task *t = &node->data;
        double passed;
        clock_t start, end;
        while(waitingForPainting > 0 || waitingForPackaging > 0){
            if(t==NULL ||node == NULL){
                break;
            }
            //printf("id of : %d , stage : %d , packaging : %d , type : %d \n",t->ID,t->stage ,t->packaging,t->type);
            if(t->stage == 1 && waitingForPackaging > 0){
                psem_wait(semaphore);
                if(t->packaging == 0){
                    //start = clock();
                    pthread_sleep(1);
                    //end = clock();
                    //passed = ((double)end - start)/CLOCKS_PER_SEC;
                    printf("At Seconds : %d  ::  Elf A packaged order type %d with id %d\n",timePassed, t->type,t->ID);
                    t->packaging = 1;
                    t->stage = 2;
                    waitingForPackaging--;
                    waitingForDelivery++;
                }
                psem_post(semaphore);
                free(arg);
            }
            else if(waitingForPackaging == 0 && t->stage == 0 && t->painting == 0 && t->type ==2){
                //start = clock();
                pthread_sleep(2);
                //end = clock();
                //passed = ((double)end - start)/CLOCKS_PER_SEC;
                printf("At Seconds : %d  ::  Elf A painted order type %d with id %d\n", timePassed, t->type,t->ID);
                t->painting = 1;
                t->stage = 1;
                waitingForPainting--;
                waitingForPackaging++;
                break;
            }
            
            node = node->prev;
            t = &node->data;
        }
    }
    
    
        
    return 0;
}

void* ElfB(void *arg){ // ASSEMBLY AND PACKAGE
    while(TRUE) {
        struct Node_t *node;
        node= myQ->head;
        Task *t = &node->data;
        int passed = timePassed;
        while(waitingForAssembly > 0 || waitingForPackaging > 0){
            if(t==NULL ||node == NULL){
                break;
            }
            //printf("id of : %d , stage : %d , packaging : %d , type : %d \n",t->ID,t->stage ,t->packaging,t->type);
            if(t->stage == 1 && waitingForPackaging > 0){
                psem_wait(semaphore);
                if(t->packaging == 0){
                    pthread_sleep(1);
                    printf("At Seconds : %d  ::  Elf B packaged order type %d with id %d\n", timePassed,t->type,t->ID);
                    t->packaging = 1;
                    t->stage = 2;
                    waitingForPackaging--;
                    waitingForDelivery++;
                }
                psem_post(semaphore);
                free(arg);
            }
            else if(waitingForPackaging == 0 && t->stage == 0 && t->assembly == 0 && t->type ==3){
                pthread_sleep(3);
                printf("At Seconds : %d  ::  Elf B assembled order type %d with id %d\n", timePassed,t->type,t->ID);
                t->assembly = 1;
                t->stage = 1;
                waitingForAssembly--;
                waitingForPackaging++;
                break;
            }
            
            node = node->prev;
            t = &node->data;
        }
    }
    
    
        
    return 0;
}

// manages Santa's tasks
void* Santa(void *arg){ // QA AND DELIVERY
    while(TRUE) {
        struct Node_t *node;
        node= myQ->head;
        Task *t = &node->data;
        while(waitingForQA > 0 || waitingForDelivery > 0){
            if(t==NULL ||node == NULL){
                break;
            }
            //printf("id of : %d , stage : %d , packaging : %d , type : %d \n",t->ID,t->stage ,t->packaging,t->type);
            if(t->stage == 2 && t->delivery == 0){
                pthread_sleep(1);
                printf("At Seconds : %d  ::  Santa delivered order type %d with id %d\n", timePassed,t->type,t->ID);
                t->delivery = 1;
                t->stage = 3;
                waitingForDelivery--;
                totalDeliveredOrder++;
                break;
            }
            else if(waitingForDelivery == 0 && t->stage == 0 &&
                    t->qa == 0 && (t->type ==4 || t->type== 5)){
                pthread_sleep(1);
                printf("At Seconds : %d  ::  Santa did QA order type %d with id %d\n", timePassed,t->type,t->ID);
                t->qa = 1;
                t->stage = 1;
                waitingForQA--;
                waitingForPackaging++;
                break;
            }
            
            node = node->prev;
            t = &node->data;
        }
    }
    return 0;
    
}

// the function that controls queues and output
void* ControlThread(void *arg){
    pthread_t thread;
    //pthread_mutex_t mutex;
    
    //pthread_mutex_init(&mutex,NULL);
    
    pthread_create(&thread,NULL,arg,NULL);
    
    //pthread_join(thread,NULL);
    
    //pthread_mutex_destroy(&mutex);
    
    return 0;
}


#ifdef __APPLE__
void psem_init(psem_t *sem, u_int32_t value) {
    *sem = dispatch_semaphore_create(value);
}
void psem_wait(psem_t sem) {
    dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
}
void psem_post(psem_t sem) {
    dispatch_semaphore_signal(sem);
}
#else
void psem_init(psem_t *sem, u_int32_t value) {
    sem_init(sem, 0, value);
}
void psem_wait(psem_t sem) {
    sem_wait(&sem);
}
void psem_post(psem_t sem) {
    sem_post(&sem);
}
#endif
