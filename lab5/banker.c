#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>

pthread_mutex_t mutex;

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

// the available amount of each resource
int available[NUMBER_OF_RESOURCES];

// the maimum demand of each customer
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

// the amount currently allocated to each customer
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {0};

// the remaining need of each customer
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

// the finishing count of the system
// if a customer acquire all needs, finishCount++
int finishCount = 0;

bool finish[NUMBER_OF_CUSTOMERS] = {false};

struct thread_info{
	int customerNum;
	int request[3];
	int release[3];
};


void print_current_state(){
	printf("available\n");
	printf("resource   %d  %d  %d\n\n", available[0], available[1], available[2]);
	printf("           maximum     allocation    need\n");
	int i;
	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++){
		printf("customer %d", i);
		printf("  %d  %d  %d  ", maximum[i][0], maximum[i][1], maximum[i][2]);
		printf("  %d  %d  %d  ", allocation[i][0], allocation[i][1], allocation[i][2]);
		printf("  %d  %d  %d  ", need[i][0], need[i][1], need[i][2]);
		printf("\n");
	}
	printf("\n==================");
	return;
}

void* threadRunner(void* param){
	int i;
	struct thread_info *info = (struct thread_info*)param;
	while (true){	
		for (i = 0; i < NUMBER_OF_RESOURCES; i++){
			//printf("%d", maximum[(*info).customerNum][i]);
			(*info).request[i] = rand() % (maximum[(*info).customerNum][i] + 1);
			(*info).release[i] = rand() % (maximum[(*info).customerNum][i] + 1);
		}
		printf("\n");
		pthread_mutex_lock(&mutex);
		int code = requestResources((*info).customerNum, (*info).request);
		
		print_current_state();
		//pthread_exit(0); // debug
		pthread_mutex_unlock(&mutex);
		if (code >= 2) {pthread_exit(0);}
		sched_yield();
		

		pthread_mutex_lock(&mutex);
		code = releaseResources((*info).customerNum, (*info).release);
		//sleep(2);
		print_current_state();
		pthread_mutex_unlock(&mutex);
		//sleep(2);
		sched_yield();
	}
}


/*
return code
3 succeeds & all customers finish,
2 succeeds & current customer finishes,
1 succeeds,
0 no request,
-1 fails since request exceeds need
-2 fails since request exceeds available,
-3 fails since the state is unsafe
*/
int requestResources(int customerNum, int request[]){
	bool check_available = true;
	int i, j, k;
	
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		// check request exceed need
		if (request[i] > need[customerNum][i]){
			printf("Request ");
			for (j = 0; j < NUMBER_OF_RESOURCES; j++){
				printf("%d  ", request[j]);
			}
			printf("\n");
			printf("Request Code -1: customer %d's request fails since request exceeds need\n\n\n", customerNum);
			return -1;
		} 
		// check request exceed available
		else if (request[i] > available[i]){
			printf("Request ");
			for (j = 0; j < NUMBER_OF_RESOURCES; j++){
				printf("%d  ", request[j]);
			}
			printf("\n");
			printf("Request Code -2: customer %d's request fails since request exceeds available\n\n\n", customerNum);
			return -2;
		} 
	}

	int r = 0;
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		r += request[i];
	}
	if (r == 0){
		printf("Request ");
		for (j = 0; j < NUMBER_OF_RESOURCES; j++){
			printf("%d  ", request[j]);
		}
		printf("\n");
		printf("Request Code 0: customer %d doesn't request\n\n\n", customerNum);
		return 0;
	}
	int _available[NUMBER_OF_RESOURCES];
	memcpy(_available, available, sizeof(available));
	int _allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
	memcpy(_allocation, allocation, sizeof(allocation));
	int _need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
	memcpy(_need, need, sizeof(need));

	// pretend to allocate resources
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		_available[i] -= request[i];
		_need[customerNum][i] -= request[i];
		_allocation[customerNum][i] += request[i];
	}

	// check finishAll, finish, no request with precedence finishAll < finish > no request
	// you should release resources you are allocated here if finishAll of finish occur

	// check safety, if not safe, restore the resources which is allocated when pretending to allocate resources
	bool local_finish[NUMBER_OF_CUSTOMERS] = {false};
	while (true){
		bool next_found = false;
		for (i = 0; i < NUMBER_OF_CUSTOMERS; i++){
			if (local_finish[i]) continue;
			bool can_finish = true;
			for (j = 0; j < NUMBER_OF_RESOURCES; j++){
				if (_need[i][j] > _available[j]){
					can_finish = false;
					break;				
				}
			}
			if (can_finish){
				local_finish[i] = true;
				next_found = true;
				for (j = 0; j < NUMBER_OF_RESOURCES; j++){
					_available[j] += _allocation[i][j];
					_allocation[i][j] = 0;
					_need[i][j] = 0;
				}				
				break;
			}
		}
		if (!next_found) break;
	}
	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++){ // some thread can't finish
		if (!local_finish[i]){
			printf("Request ");
			for (j = 0; j < NUMBER_OF_RESOURCES; j++){
				printf("%d  ", request[j]);
			}
			printf("\n");
			printf("Request Code -3: customer %d's request fails since the state is unsafe\n\n\n", customerNum);
			return -3;			
		}
	}

	// safe and ilegal request, we can allow this request
	// i.e. allocate the resources
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		available[i] -= request[i];
		allocation[customerNum][i] += request[i];
		need[customerNum][i] -= request[i];
	}
	// case 1: succeeds, not finish yet
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		if (need[customerNum][i] != 0){
			printf("Request ");
			for (j = 0; j < NUMBER_OF_RESOURCES; j++){
				printf("%d  ", request[j]);
			}
			printf("\n");
			printf("Request Code 1: customer %d's request succeeds\n\n\n", customerNum);
			return 1;		
			
		}
	}
	// case 2: current customers finish
	finish[customerNum] = true;
	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++){
		for (j = 0; j < NUMBER_OF_RESOURCES; j++){
			if (need[i][j] > 0){
				printf("Request ");
				for (k = 0; k < NUMBER_OF_RESOURCES; k++){
					printf("%d  ", request[k]);
				}
				printf("\n");
				printf("Request Code 2: customer %d's request succeeds & finishes\n\n\n", customerNum);
				for (k = 0; k < NUMBER_OF_RESOURCES; k++){
					available[k] += allocation[customerNum][k];
					allocation[customerNum][k] = 0;
					need[customerNum][k] = 0;
				}
				return 2;	
			}
		}
	}
	// case 3: all finishes
	printf("Request ");
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		printf("%d  ", request[i]);
	}
	printf("\n");
	printf("Request Code 3: customer %d's request succeeds & all customers finish\n\n\n", customerNum);
	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++){
		available[i] += allocation[customerNum][i];
		allocation[customerNum][i] = 0;
		need[customerNum][i] = 0;
	}
	return 3;	
}

/*
return code
1 succeeds
0 no release,
-1 fails since release exceeds allocation
*/
int releaseResources(int customerNum, int release[]){
	int i, j;
	// check release exceed allocation
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		if (release[i] > allocation[customerNum][i]){
			printf("release ");
			for (j = 0; j < NUMBER_OF_RESOURCES; j++){
				printf("%d  ", release[j]);
			}
			printf("\n");
			printf("release Code -1: customer %d's release fails since release exceeds allocation\n\n\n", customerNum);
			return -1;
		}
	}
	// check no release
	int r = 0;
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		r += release[i];
	}	
	if (r == 0){
		printf("release ");
		for (j = 0; j < NUMBER_OF_RESOURCES; j++){
			printf("%d  ", release[j]);
		}
		printf("\n");
		printf("release Code 0: customer %d doesn't release any resource\n\n\n", customerNum);
		return 0;
	}
	// release
	printf("release ");
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		printf("%d  ", release[i]);
	}
	printf("\n");
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		allocation[customerNum][i] -= release[i];
		available[i] += release[i];
		need[customerNum][i] += release[i];
	}
	printf("release ");
	printf("\n");
	printf("release Code 1: customer %d release succeeds\n\n\n", customerNum);
	return 1;
}


/*
int main(){
	return 0;
}
*/

int main(int argc, char **argv){
	if (argc != 1 + NUMBER_OF_RESOURCES){
		printf("wrong numbers of resources\n");
		return 0;
	}
	
	int i, j;
	for (i = 0; i < NUMBER_OF_RESOURCES; i++){
		available[i] = atoi(argv[i+1]);
	}
	for (i = 0; i < NUMBER_OF_CUSTOMERS; i++){
		for (j = 0; j < NUMBER_OF_RESOURCES; j++){
			maximum[i][j] = rand() % (available[j] + 1);
			need[i][j] = maximum[i][j];
		}
	}
	
	pthread_t tid0, tid1, tid2, tid3, tid4;
	struct thread_info info0, info1, info2, info3, info4;
	info0.customerNum = 0;
	info1.customerNum = 1;
	info2.customerNum = 2;
	info3.customerNum = 3;
	info4.customerNum = 4;
	
	pthread_create(&tid0, NULL, threadRunner, (void*)&info0);
	pthread_create(&tid1, NULL, threadRunner, (void*)&info1);
	pthread_create(&tid2, NULL, threadRunner, (void*)&info2);
	pthread_create(&tid3, NULL, threadRunner, (void*)&info3);
	pthread_create(&tid4, NULL, threadRunner, (void*)&info4);
	
	/*
	pthread_t tid[NUMBER_OF_CUSTOMERS];
	for(i = 0; i < NUMBER_OF_CUSTOMERS; i++){
		struct thread_info info;
		info.customerNum = i;
		pthread_create(&tid[i], NULL, threadRunner, (void*)&info);
	}
	*/
	for(i = 0; i < NUMBER_OF_CUSTOMERS; i++){
		pthread_join(tid0, NULL);
		pthread_join(tid1, NULL);
		pthread_join(tid2, NULL);
		pthread_join(tid3, NULL);
		pthread_join(tid4, NULL);
	}
	return 0;
}

