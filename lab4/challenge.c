#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t challenge_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;

struct trainer{
	int time; // the time what training and chellenging takes
	int challenge_times; // challenge times before success
	char name[5]; // name of the given trainer
};

int waiting_line = 0; // record of number of trainers which are waiting
int done_challenge = 0; // check if all challengers are success

void* challenge(void* param){
	struct trainer *c = (struct trainer*)param; 
	int remain_times = (*c).challenge_times;
	while(remain_times > 0){
		sleep((*c).time);
		pthread_mutex_lock(&wait_mutex);
		if (waiting_line == 0){ // nobody, start challenge
			waiting_line += 1;
			pthread_mutex_unlock(&wait_mutex);
			pthread_mutex_lock(&challenge_mutex);
			//printf("The Gym Leader is waken up by a Trainer.\n");
			printf("The Gym Leader is battling with Trainer %s now.\n", (*c).name);
			sleep((*c).time);
			//printf("The battle with Trainer %s is over.\n", (*c).name);
			pthread_mutex_unlock(&challenge_mutex);
			remain_times -= 1;
			pthread_mutex_lock(&wait_mutex);
			waiting_line -= 1;
			pthread_mutex_unlock(&wait_mutex);
		} else if (waiting_line == 1){ // you have a seat! wait for challenge!
			//printf("%s has a seat, waiting...\n", (*c).name);
			waiting_line += 1;
			pthread_mutex_unlock(&wait_mutex);
			pthread_mutex_lock(&challenge_mutex);	
			printf("The Gym Leader is battling with Trainer %s now.\n", (*c).name);
			sleep((*c).time);
			//printf("The battle with Trainer %s is over.\n", (*c).name);
			pthread_mutex_unlock(&challenge_mutex);
			remain_times -= 1;
			pthread_mutex_lock(&wait_mutex);
			waiting_line -= 1;
			pthread_mutex_unlock(&wait_mutex);
		} else if (waiting_line == 2){ // you dont have seat, back for training!
			pthread_mutex_unlock(&wait_mutex);
			//printf("%s is back to train for %d seconds (no seat)\n", (*c).name, (*c).time);
			continue;
		} else{
			printf("something wrong!!\n");
		}
		if (remain_times > 0){	
			//printf("%s is back to train for %d seconds (after challenge)\n", (*c).name, (*c).time);
			continue;
		}
	}
	//printf("Trainer %s get the badge!\n", (*c).name);
	done_challenge += 1;
	return NULL;	
}


int main(){
	pthread_t ash, misty, brock;
	//printf("1");
	struct trainer ash_data, misty_data, brock_data;
	//printf("2");
	strcpy(ash_data.name, "ash");
	strcpy(misty_data.name, "misty");
	strcpy(brock_data.name, "brock");
	ash_data.time = 2;
	misty_data.time = 3;
	brock_data.time = 5;
	ash_data.challenge_times = 3;
	misty_data.challenge_times = 2;
	brock_data.challenge_times = 1;
	//printf("3");
	pthread_create(&ash, NULL, challenge, (void*)&ash_data);
	pthread_create(&misty, NULL, challenge, (void*)&misty_data);
	pthread_create(&brock, NULL, challenge, (void*)&brock_data);
	//printf("4");
	//pthread_join(ash, NULL);
	//pthread_join(misty, NULL);
	//pthread_join(brock, NULL);
	
	while (done_challenge != 3){
			continue;
	}
	printf("all trainers get the badge!\n");
	return 0;
}
