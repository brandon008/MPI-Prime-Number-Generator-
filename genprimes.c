#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <mpi.h>

int my_id,num_procs;


int main(int argc,char *argv[]){
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  int n = atoi(argv[1]);

  int start;
  int end;
  if(my_id == 0){
    start = 2;
  }else{
    start = (my_id*floor(n/num_procs))+1;
  }

  if(my_id == num_procs-1){
    end = n;
  } else{
    end = (my_id+1)*floor((n/num_procs));
  }
  // int count = 0;
  int i;
  bool *pList = malloc(100000001*sizeof(bool));
  for(int i=2; i<=n;i++){
    pList[i] = true;
  }
  double start_time = MPI_Wtime();
  MPI_Barrier(MPI_COMM_WORLD);
  for(int i=2; i<=ceil((double)end/2); i++){
      for(int j=ceil((double)start/i);j<=end/i;j++){
        if(j == 1){
          continue;
        }
        int k = j*i;
        if(pList[k]){
          pList[k]=false;
        }
      }
    }

  int primeCount=0;
  for (int i = start; i<=end;i++){
    if(pList[i]==true){
      primeCount++;
    }
  }
  int *primes = malloc(primeCount*sizeof(int)); //primes[]
  int j=0;
  for(int i = start; i<=end;i++){
    if(pList[i]==true){
      primes[j] = i;
      j++;
    }
  }
  if (my_id != 0) { // every other process
    int SData[3];
    SData[0] = primeCount;
    SData[1] = start;
    SData[2] = end;
    MPI_Send(SData, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(primes, primeCount, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
  }else{ //process 0



    int count2;
    int *totalPrimes = malloc(100000000*sizeof(int));
    int totalPrimeCount = 0;

    for(int i =0;i<primeCount;i++){
      totalPrimes[totalPrimeCount] = primes[i];
      totalPrimeCount++;
    }

    for (count2 = 1; count2<num_procs; count2++){
      int RData[3];
      MPI_Status status;

      MPI_Recv(RData, 3, MPI_INT, count2, MPI_ANY_TAG,MPI_COMM_WORLD, &status);
      MPI_Recv(totalPrimes+totalPrimeCount, RData[0], MPI_INT, count2, MPI_ANY_TAG,MPI_COMM_WORLD, &status);
      totalPrimeCount += RData[0];

    }
    printf("\nTotal number of primes between 2 and %d:  %d.\n", n, totalPrimeCount);
    MPI_Barrier(MPI_COMM_WORLD);
    double total_time = MPI_Wtime() - start_time;
    printf("\nTotal Time: %1.3f seconds\n",total_time);

    //write file
    char s[20];
    sprintf(s,"%d.txt",n) ;
    FILE *myfile = fopen(s,"w");
    for(int i = 0;i<totalPrimeCount;i++){
      fprintf(myfile,"%d ",totalPrimes[i]);
    }
    fclose(myfile);
  }
  MPI_Finalize();
  return 0;
}
