#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int sudoku_grid[9][9];
int check_subsections[11] = {0}; // 0-8: field, 9: row, 10: col

typedef struct{
    int row;
    int column;
} parameters;

void *check_row(void *param){
    int i, j, num;
    int check_array[9] = {0};
    for (i = 0; i < 9; i++){
        for (j = 0; j < 9; j++){
            num = sudoku_grid[i][j];
            if (check_array[num-1] == 1){
                pthread_exit(NULL);
            }
            check_array[num-1] = 1;
        }
        memset(check_array, 0, sizeof(check_array));
    }
    check_subsections[9] = 1;
}

void *check_col(void *param){
    int i, j, num;
    int check_array[9] = {0};
    for (i = 0; i < 9; i++){
        for (j = 0; j < 9; j++){
            num = sudoku_grid[j][i];
            if (check_array[num-1] == 1){
                pthread_exit(NULL);
            }
            check_array[num-1] = 1;
        }
        memset(check_array, 0, sizeof(check_array));
    }
    check_subsections[10] = 1;
}

void *check_field(void *param){
    int i, j, num;
    parameters *params = (parameters*) param;
    int row = params -> row;
    int col = params -> column;
    int check_array[9] = {0};
    for (i = row; i < row+3; i++){
        for (j = col; j < col+3; j++){
            num = sudoku_grid[i][j];
            if (check_array[num-1] == 1){
                pthread_exit(NULL);
            }
            check_array[num-1] = 1;
        }
    }
    check_subsections[row/3 + col] = 1; 
}

int main(int argc, char *argv[]){
    // read file into grid
    FILE *f = fopen(argv[1], "r");
    int num;
    int i = 0;
    int j = 0;
    int c = fgetc(f);
    int k = 1; // check length of line
    while (c != EOF){
        num = c - 48; // ascii 40->0, 41->1, ...
        if (num == -35){
            c = fgetc(f);
            continue;
        }
        //printf("%d %d;", k, num);
        if ((c > 57) || (c < 49)){ // end of line
            if (k != 10){ // length > 9
                printf("false\n");
                return 0;
            }
            c = fgetc(f);
            k = 1;
            continue;
        }
        else if ((num <= 9) && (num >= 1)){
            sudoku_grid[i/9][j%9] = num;
            i += 1;
            j += 1;
            c = fgetc(f);
        }
        else{
            printf("false");
            return 0;
        }
        k += 1;
    }
    
    int num_threads = 11;
    pthread_t p_threads[num_threads];
    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            parameters *param = (parameters *)malloc(sizeof(parameters));
            param -> row = i*3;
            param -> column = j*3;
            pthread_create(&p_threads[i*3+j], NULL, check_field, param);
        }
    }
    for (i = 0; i < 1; i++){
        parameters *param = (parameters *)malloc(sizeof(parameters));
        param -> row = 0;
        param -> column = 0;
        pthread_create(&p_threads[9], NULL, check_row, param);
    }
    for (i = 0; i < 1; i++){
        parameters *param = (parameters *) malloc(sizeof(parameters));
        param -> row = 0;
        param -> column = 0;
        pthread_create(&p_threads[10], NULL, check_col, param);
    }

    for (i = 0; i < num_threads; i++){
        pthread_join(p_threads[i], NULL);
    }

    //for (i = 0; i < 11; i++){
    //    printf("check_subsections %d = %d\n", i, check_subsections[i]);
    //}
    for (i = 0; i < 11; i++){
        if (check_subsections[i] == 0){
            printf("false\n");
            return 0;
        }
    }
    printf("true\n");
    return 0;
}

