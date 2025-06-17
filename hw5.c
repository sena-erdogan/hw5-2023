#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <sys/ipc.h>

#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_TRUNC)
#define WRITE_PERMS (S_IRUSR | S_IWUSR | S_IWGRP)

#define PI 3.142857

int N, n, m; /* N = n^2 */

int** mat1;
int** mat2;
int** res;
double** dftmat;

pthread_mutex_t lock;
int count = 0;
int start = 0;

void* thread(){

	clock_t t;
	t = clock();
	
	pthread_mutex_lock(&lock);
	int i, j, k;
	int final=0;
	time_t ct = time(NULL);
	struct tm *tm;
	
	count++;
	
	j=0;
	
	while(res[0][j] != -100){
		j++;
	}
	
	final = j + (int)(N / m);
	
	for(i=0; i<N; i++){
		for(j=start; j<final && j<N; j++){
			for(k=0; k<N; k++)	res[i][j] += mat1[i][k] * mat2[k][j];
		}
	}
	
	start = final;
	
	tm = localtime(&ct);
	t = clock()-t;
	
	printf("%s - Thread %d has reached the rendezvous point in %lf seconds\n", asctime(tm), count, ((double)t)/CLOCKS_PER_SEC);
	
	pthread_mutex_unlock(&lock);
	
	return 0;
}

void* dft(){

	clock_t t;
	t = clock();
	
	pthread_mutex_lock(&lock);
	int i, j, k, l;
	double temp, templ, tempr;
	
	int final=0;
	time_t ct = time(NULL);
	struct tm *tm;
	
	count++;
	tm = localtime(&ct);
	printf("%s - Thread %d is advancing to the second part\n", asctime(tm), count);
	
	j=0;
	
	while(dftmat[0][j] != -100.0){
		j++;
	}
	
	final = j + (int)(N / m);
	
	for(i=0; i<N; i++){
		for(j=start; j<final && j<N; j++){
			for(k=0; k<N; k++){
				for(l=0; l<N; l++){
					temp = (res[k][l]);
					templ = (double)(((double)i/N)*k);
					tempr = (double)(((double)j/N)*l);
					templ += tempr;
					templ *= -2.00000;
					templ *= PI;
					temp *= (exp(templ));
					dftmat[i][j] += temp;
				}
			}
		}
	}
	
	start = final;
	
	tm = localtime(&ct);
	t = clock()-t;
	
	printf("%s - Thread %d has finished the second part in %lf seconds\n", asctime(tm), count, ((double)t)/CLOCKS_PER_SEC);
	
	pthread_mutex_unlock(&lock);
	
	return 0;
}

int main(int argc, char *argv[]){

	clock_t t;
	t = clock();
	
	int i,j;
	int bytesread = 0;
	int fi1, fi2, fo;
	char buf[1];
	
	time_t ct = time(NULL);
	struct tm *tm;
	
	if(argc != 11){
		perror("\nUsage: Thread Synchronization\nformat: ./hw5 -i filePath1 -j filePath2 -o output -n 4 -m 2\n");
		return 1;
	}
	
	for(i=1; i<=9; i+=2){
		if(strcmp(argv[i], "-i") == 0){
			if((fi1 = open(argv[i+1], READ_FLAGS)) == -1){
				perror("Failed to open input file 1\n");
				return 1;
			}
		}else if(strcmp(argv[i], "-j") == 0){
			if((fi2 = open(argv[i+1], READ_FLAGS)) == -1){
				perror("Failed to open input file 2\n");
				return 1;
			}
		}else if(strcmp(argv[i], "-o") == 0){
			if((fo = open(argv[i+1], WRITE_FLAGS, WRITE_PERMS)) == -1){
				perror("Failed to open output file\n");
				return 1;
			}
		}else if(strcmp(argv[i], "-n") == 0){
			n = atoi(argv[i+1]);
			if(n<=2){
				perror("n should be larger than 2");
				exit(1);
			}
		}else if(strcmp(argv[i], "-m") == 0){
			m = atoi(argv[i+1]);
			if(m%2 != 0){
				perror("m should satisfy the condition: m >= 2k, k>=1");
				exit(1);
			}
		}else{
			perror("\nInvalid command\nformat: ./hw5 -i filePath1 -j filePath2 -o output -n 4 -m 2\n");
			return 1;
		}
	}
	
	N = pow(2,n);
	
	if(m>N){
		printf("m is greater than n^2, m is equaled to n^2\n");
		printf("--Program keeps running--\n");
		m=N;
	}	

	mat1 = (int**)malloc(sizeof(int)*N);
	mat2 = (int**)malloc(sizeof(int)*N);
	res = (int**)malloc(sizeof(int)*N);
	
	for(i=0; i<N; i++){
		mat1[i] = (int*)malloc(sizeof(int)*N);
		mat2[i] = (int*)malloc(sizeof(int)*N);
		res[i] = (int*)malloc(sizeof(int)*N);
	}
	
	i=0;
	j=0;
	
	for( ; ; ){
		while(((bytesread = read(fi1, buf, 1)) == -1) && (errno == EINTR));
		
		if(buf[0] == EOF){
			perror("Input file 1 does not have sufficient content. (Last character is End Of File)");
			exit(1);
		}
		
		if(bytesread <= 0){
			if(i < N){
				perror("Input file 1 does not have sufficient content");
				exit(1);
			}
			
			break;
		}
		
		if(j == N){
			j=0;
			i++;
		}
		
		if(i == N){
			break;
		}
		
		mat1[i][j] = buf[0];
		
		j++;
		
	}
	
	if(close(fi1) == -1){
		perror("Failed to close input file 2\n");
		return 1;
	}
	
	i=0;
	j=0;
	bytesread=0;
	
	for( ; ; ){
		while(((bytesread = read(fi2, buf, 1)) == -1) && (errno == EINTR));
		
		if(buf[0] == EOF){
			perror("Input file 2 does not have sufficient content. (Last character is End Of File)");
			exit(1);
		}
		
		if(bytesread <= 0){
			if(i < N){
				perror("Input file 2 does not have sufficient content");
				exit(1);
			}
			
			break;
		}
		
		if(j == N){
			j=0;
			i++;
		}
		
		if(i == N){
			break;
		}
		
		mat2[i][j] = buf[0];
		
		j++;
		
	}
	
	if(close(fi2) == -1){
		perror("Failed to close input file 1\n");
		return 1;
	}
	
	tm = localtime(&ct);
	
	printf("%s - Two matrices of size %dx%d have been read. The number of threads is %d\n", asctime(tm), N, N, m);
	
	for(i=0; i<N; i++){
		for(j=0; j<N; j++){
			res[i][j] = -100;
		}
	}
	
	pthread_t* th;
	
	th = (pthread_t*)malloc(sizeof(pthread_t)*m);
	
	for (i = 0; i < m; i++){
		if (pthread_create(&th[i], NULL, &thread, NULL) != 0) {
			perror("Failed to create thread");
		}
	}
	
	for (i = 0; i < m; i++){
		if (pthread_join(th[i], NULL) != 0)	perror("Failed to join thread");
	}

	free(th);
	
	dftmat = (double**)malloc(sizeof(double)*N);
	
	for(i=0; i<N; i++){
		dftmat[i] = (double*)malloc(sizeof(double)*N);
	}
	
	for(i=0; i<N; i++){
		for(j=0; j<N; j++){
			dftmat[i][j] = -100.0;
		}
	}
	
	count = 0;
	start = 0;
	
	pthread_t* thn;
	
	thn = (pthread_t*)malloc(sizeof(pthread_t)*m);
	
	for (i = 0; i < m; i++){
		if (pthread_create(&thn[i], NULL, &dft, NULL) != 0) {
			perror("Failed to create thread");
		}
	}
	
	for (i = 0; i < m; i++){
		if (pthread_join(thn[i], NULL) != 0)	perror("Failed to join thread");
	}
	
	int temp, digit;
	char* buffer;
	
	for(i=0; i<N; i++){
		for(j=0; j<N; j++){
		
			buffer = (char*)malloc(20);
			
			digit=7;
			temp = dftmat[i][j];
			
			while(temp != 0){
				digit++;
				temp /= 10;
			}
			
			gcvt(dftmat[i][j], digit, buffer);
			
			if(write(fo, buffer, digit) != digit)	perror("Can't write to output file");
			if(write(fo, " + j(", 5) != 5)	perror("Can't write to output file");
			if(write(fo, buffer, digit) != digit)	perror("Can't write to output file");
			if(write(fo, "), ", 3) != 3)	perror("Can't write to output file");
			
			free(buffer);
		}
		if(write(fo, "\n", 1) != 1)	perror("Can't write to output file");
	}
	
	if(close(fo) == -1){
		perror("Failed to close output file\n");
		return 1;
	}
	
	tm = localtime(&ct);
	t = clock()-t;
	
	printf("%s - The process has written the output file. The total time spent is %lf seconds\n", asctime(tm), ((double)t)/CLOCKS_PER_SEC);
	
	free(thn);
	for(i=0; i<N; i++){
		free(res[i]);
	}
	free(res);
	for(i=0; i<N; i++){
		free(dftmat[i]);
	}
	free(dftmat);
	for(i=0; i<N; i++){
		free(mat1[i]);
	}
	free(mat1);
	
	for(i=0; i<N; i++){
		free(mat2[i]);
	}
	free(mat2);
	
	return 0;
}
