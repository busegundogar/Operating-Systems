#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_THREADS 512

//participant threads
pthread_t student_threads[MAX_THREADS];
pthread_t assistant_threads[MAX_THREADS];

//mutex to protect atomicity
pthread_mutex_t atomicity = PTHREAD_MUTEX_INITIALIZER;

//semaphore for checking entering and leaving condition
sem_t availability;

//2 semaphore for waiting/waking participants for demo sessions
sem_t student_check;
sem_t assistant_check;

//2 counters for waiting participants
int stu_in_class = 0;
int assist_in_class = 0;

//barrier for demo sessions
pthread_barrier_t barrier;

void *student(void *arg) {
    printf("Thread ID:%ld, Role:Student, I want to enter the classroom.\n", pthread_self());
    
    sem_wait(&availability);
    printf("Thread ID:%ld, Role:Student, I entered the classroom.\n", pthread_self());

    pthread_mutex_lock(&atomicity);
    if(assist_in_class >= 1 && stu_in_class >= 1){ //If we have 1 stu. and 1 assist., we should wake them up and start demo

        //We should also decrease them from counter since they are not waiting anymore
        stu_in_class--;
        assist_in_class--;
        pthread_mutex_unlock(&atomicity); 

        sem_post(&student_check);
        sem_post(&assistant_check);
    }else{
        stu_in_class++;
        pthread_mutex_unlock(&atomicity); 
        sem_wait(&student_check);
    }

    printf("Thread ID:%ld, Role:Student, I am now participating.\n",pthread_self());
    pthread_barrier_wait(&barrier); //Waiting in barrier if demo is not yet finished

    //If demo is done, student can leave
    printf("Thread ID:%ld, Role:Student, I left the classroom.\n", pthread_self());
    sem_post(&availability);
}

void *assistant(void *arg) {
    
    printf("Thread ID:%ld, Role:Assistant, I entered the classroom.\n", pthread_self());

    //Assistants directly enter to the classroom, so we can increase the num directly
    sem_post(&availability);
    sem_post(&availability);
    sem_post(&availability);

    //Same demo check with the student function is also written to here
    pthread_mutex_lock(&atomicity);
    if(stu_in_class >= 2 && assist_in_class>=0){ //&& stu_in_class >= 1){ //If we have 2 stu. and 1 assist., we should wake them up and start demo

        //We should also decrease them from counter since they are not waiting anymore
        stu_in_class--;
        stu_in_class--;
        sem_post(&student_check);
        sem_post(&student_check);
        pthread_mutex_unlock(&atomicity);
    }else{
        assist_in_class++;
        pthread_mutex_unlock(&atomicity);
        sem_wait(&assistant_check);
    }

    printf("Thread ID:%ld, Role:Assistant, I am now participating.\n",pthread_self());
    pthread_barrier_wait(&barrier); //Waiting in barrier if demo is not yet finished

    //If we get to here, it means demo is over, so assistant can leave
    printf("Thread ID:%ld, Role:Assistant, demo is over.\n",pthread_self());

    sem_wait(&availability);
    sem_wait(&availability);
    sem_wait(&availability);

    printf("Thread ID:%ld, Role:Assistant, I left the classroom.\n", pthread_self());
}

int main(int argc, char *argv[]) {

    printf("My program complies with all the conditions.\n");

    int num_assistants = atoi(argv[1]);
    int num_students = atoi(argv[2]);

    if(num_assistants <= 0 || num_students <= 0){
        printf("Inputs must be positive!\n");
        return 1;
    }else if(num_students != num_assistants*2){
        printf("Number of students must be 2 times the number of assistants!\n");
        return 1;
    }

    int i,j,k,m;
    pthread_barrier_init(&barrier, NULL, 3);
    sem_init(&availability, 0, 0);
    sem_init(&student_check, 0, 0);
    sem_init(&assistant_check, 0, 0);


    for ( i = 0; i < num_students; i++) {
        pthread_create(&student_threads[i], NULL, student, NULL);
    }

    for ( j = 0; j < num_assistants; j++) {
        pthread_create(&assistant_threads[j], NULL, assistant, NULL);
    }

    for ( k = 0; k < num_students; k++) {
        pthread_join(student_threads[k], NULL);
    }

    for ( m = 0; m < num_assistants; m++) {
        pthread_join(assistant_threads[m], NULL);
    }

    printf("The main terminates.\n");
    return 0;
}
