#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <openacc.h>

#ifdef DEBUG
#define SIZE 8
#else
#define SIZE 32000
#endif

void print_loc( double * mat, int loc_size ){
  
  int i, j;
  for( i = 0; i < loc_size; i++ ){
    for( j = 0; j < SIZE; j++ ){
      fprintf( stdout, "%.3g ", mat[ j + ( i * SIZE ) ] );
    }
    fprintf( stdout, "\n" );
    
  }
}

void print_par( double * mat, int loc_size, int rank, int npes ){
  
  int count;
  
  if( rank ) MPI_Send( mat, loc_size * SIZE, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD );
  else{
    
    double * buf = (double *) calloc( loc_size * SIZE, sizeof(double) );
    print_loc( mat, loc_size );
    for( count = 1; count < npes; count ++){
      MPI_Recv( buf, loc_size * SIZE, MPI_DOUBLE, count, count, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
      print_loc( buf, loc_size );
    }
    free( buf );
  }
}

int main( int argc, char * argv[] ){
  
  int npes, rank;
  int i, j, count;
  double * mat, * buf;
  
  MPI_Init( &argc, & argv );
  MPI_Comm_size( MPI_COMM_WORLD, &npes );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );

#ifdef OPENACC  
  int ngpu = acc_get_num_devices(acc_device_nvidia);
  int igpu = rank % ngpu;
  acc_set_device_num(igpu, acc_device_nvidia);
  acc_init(acc_device_nvidia);
  if( !rank ) fprintf(stdout, "NUM GPU: %d\n", ngpu);
  fprintf(stdout, "GPU ID: %d, PID: %d\n", igpu, rank);
  fflush( stdout );
#endif

  MPI_Barrier( MPI_COMM_WORLD );
  
  int loc_size = SIZE / npes;

  mat = (double *) calloc( loc_size * SIZE, sizeof(double) );
  buf = (double *) calloc( SIZE * loc_size, sizeof(double) );

#pragma acc enter data create ( mat[ 0 : loc_size * SIZE ], buf[ 0 : SIZE * loc_size ] )

#pragma acc parallel loop collapse(2) present( mat )   
  for( i = 0; i < loc_size; i++ ){
    for( j = 0; j < SIZE; j++ ){
      mat[ j + ( i * SIZE ) ] = j + ( ( ( rank * loc_size ) + i ) * SIZE ) ;
    }
  }

#ifdef DEBUG  
#pragma acc update self ( mat[ 0 : loc_size * SIZE ] )  
  print_par( mat, loc_size, rank, npes );
#endif

  //1) prepare the contigous block of data for the All2all
#pragma acc parallel loop collapse(3) present( mat, buf )   
  for( count = 0; count < npes; count ++ ){
    for( i = 0; i < loc_size; i++ ){
      for( j = 0; j < loc_size; j++ ){
	int i_g = i + ( count * loc_size );
	int j_g = j + ( count * loc_size );
	buf[ j + ( i_g * loc_size ) ] = mat[ j_g + ( i * SIZE ) ];
      }
    }
  }
  
  //2) perform all2all in place
#pragma acc host_data use_device( buf )
  MPI_Alltoall( MPI_IN_PLACE, loc_size * loc_size, MPI_DOUBLE, buf, loc_size * loc_size, MPI_DOUBLE, MPI_COMM_WORLD);

  //3) local_tranposition of data into blocks
#pragma acc parallel loop collapse(3) present( mat, buf )   
  for( count = 0; count < npes; count ++ ){
    for( i = 0; i < loc_size; i++ ){
      for( j = 0; j < loc_size; j++ ){
	int i_g = i + ( count * loc_size );
	mat[ i_g + ( j * SIZE ) ] = buf[ j + ( i_g * loc_size ) ];
      }
    }
  }

#ifdef DEBUG  
#pragma acc update self ( mat[ 0 : loc_size * SIZE ] )  
  print_par( mat, loc_size, rank, npes );
#endif
  
  MPI_Finalize();

  return 0;
}
    
  
