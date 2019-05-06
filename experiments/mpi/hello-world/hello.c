#include<stdio.h>
#include<mpi.h>
#include<unistd.h>

int main(int argc, char* argv[]) {
  int NP, rank;

  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &NP);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  printf ("Oi pessoal, do processo %d dentro de %d!\n", rank, NP);

  MPI_Finalize ();

  return 0;
}
