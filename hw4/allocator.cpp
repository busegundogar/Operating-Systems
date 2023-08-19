#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>

using namespace std;


struct myNode{
    int ID;
    int size;
    int index;
    myNode* next;

    myNode(int initId, int initSize, int initIndex, myNode* initNext){
        ID = initId;
        size = initSize;
        index = initIndex;
        next = initNext;
    }
};

class HeapManager{
    private:
        myNode* head;
        pthread_mutex_t mutex;
    public:
        HeapManager();
        //de
        int initHeap(int size);
        int myMalloc(int ID, int size);
        int myFree(int ID, int index);
        void print();
};

HeapManager::HeapManager(){
    head = NULL;
    mutex = PTHREAD_MUTEX_INITIALIZER;
}

int HeapManager::initHeap(int size){
    pthread_mutex_lock(&mutex);
    head = new myNode(-1,size,0,NULL);
    print();
    pthread_mutex_unlock(&mutex);
}

int HeapManager::myMalloc(int ID, int size){
    pthread_mutex_lock(&mutex);

    myNode *search = head;
    myNode *prev = NULL;

    while(search != NULL){
        if(search->ID != -1 || search->size < size){
            prev = search;
            search = search->next;
        }
        else{
            break;
        }
    }

    //Check if there is a suitable node found
    if(search == NULL){
        cout<< "Allocating error for size: "<< size <<" and thread ID: " << ID << "\n";
        print();
        pthread_mutex_unlock(&mutex);

        return -1;
    }else{ //If found, continue
        if(size == search->size){
            //No need for dividing the nodes if the sizes are equal
            search->ID = ID;
            cout << "Allocated for thread " << ID << endl;
            print();
            pthread_mutex_unlock(&mutex);

            return search->index;
        }

        //If the searchent node's size is greater than the requested,we will divide into 2
        myNode* newNode = new myNode(ID,size,search->index,search);
        
        if(prev == NULL){
            head = newNode;
        }else{
            prev->next = newNode;
        }
        search->index += size;
        search->size -= size;
        cout << "Allocated for thread " << ID << endl;
        print();
        pthread_mutex_unlock(&mutex);

        return newNode->index;
    }
}

int HeapManager::myFree(int ID, int index) {
    pthread_mutex_lock(&mutex);

    myNode* search = head;
    myNode* prev = NULL;

    while (search != NULL) {
        if (search->ID == ID && search->index == index) {
            search->ID = -1;

            if (prev != NULL && prev->ID == -1) {
                prev->size += search->size;
                prev->next = search->next;
                delete search;
                search = prev;
            }

            if (search->next != NULL && search->next->ID == -1) {
                search->size += search->next->size;
                myNode* nextNode = search->next;
                search->next = nextNode->next;
                delete nextNode;
            }
            cout << "Freed for thread " << ID << endl;
            print();

            pthread_mutex_unlock(&mutex);
            return 1;
        }

        prev = search;
        search = search->next;
    }


    cout << "Cannot free for thread ID: " << ID <<  " with index: " << index << endl;
    print();
    pthread_mutex_unlock(&mutex);

    return -1;
}

void HeapManager::print(){
    myNode* printPtr = head;

    while(printPtr != NULL){
        cout << "[" << printPtr->ID << "][" << printPtr->size << "][" << printPtr->index << "]";

        if (printPtr->next != NULL) {
            cout << "---";
        }
        printPtr = printPtr->next;
    }
    cout << "\n";
}

