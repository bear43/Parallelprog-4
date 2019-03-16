#include <cstdio>
#include <cstdlib>
#include <memory.h>
#include <mpi/mpi.h>

#define DEFAULT_BUFFER_SIZE 128

int g_iBufferSize = DEFAULT_BUFFER_SIZE;
char *g_cBuffer = nullptr;
int g_iTotalProcessCount;
int g_iProcessRank;

/**
 * First param sets by default as path where program has been executed
 * Second param is set by user as buffer size
 * @param argc Arguments count
 * @param argv Pointer to args pointers
 * @return Exit code
 */
int main(int argc, char **argv)
{
    if(argc >= 2)
    {
        g_iBufferSize = atoi(argv[1]);
    }
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &g_iTotalProcessCount);
    MPI_Comm_rank(MPI_COMM_WORLD, &g_iProcessRank);
    int nextProcessNumber = g_iProcessRank+1;
    int prevProcessNumber = g_iProcessRank-1;
    g_cBuffer = new char[g_iBufferSize];//Allocate memory for buffer
    if(g_iProcessRank == 0)//Is this root process?
    {
        if(nextProcessNumber < g_iTotalProcessCount)
        {

            printf("Using buffer with its size %i\n", g_iBufferSize);
            volatile double starttime = MPI_Wtime();
            memcpy(g_cBuffer, (const void*)&starttime, sizeof(double));
            MPI_Send(g_cBuffer, g_iBufferSize, MPI_CHAR, nextProcessNumber, 0, MPI_COMM_WORLD);
            //g_cBuffer - destination point to set memory values
            //g_iBufferSize - buffer size. Like Array.Count in C# or Array.length in Java
            MPI_Status status;
            memset(&status, 0, sizeof(MPI_Status));//Fill memory with zero byte
            prevProcessNumber = g_iTotalProcessCount - 1;
            MPI_Recv(g_cBuffer, g_iBufferSize, MPI_CHAR, prevProcessNumber, 0, MPI_COMM_WORLD, &status);
            status.MPI_ERROR != 0 ? printf("[Root process] Catch error code by MPI_Recv: %i\n", status.MPI_ERROR)
                                  : printf("[Root process] Successful received %i bytes\n", g_iBufferSize);
            fflush(stdout);
            volatile double endtime = MPI_Wtime();
            printf("[Root process] Total work time: %lf\n", endtime - starttime);
            fflush(stdout);
        }
        else
        {
            printf("This process are single process in that running configuration. I can not send buffer to nowhere!\n");
        }
    }
    else//This is not root process
    {
        MPI_Status status;
        memset(&status, 0, sizeof(MPI_Status));//Fill memory with zero byte
        volatile double starttime;
        MPI_Recv(g_cBuffer, g_iBufferSize, MPI_CHAR, prevProcessNumber, 0, MPI_COMM_WORLD, &status);
        memcpy((void*)&starttime, (const void*)g_cBuffer, sizeof(double));
        volatile double endtime = MPI_Wtime();
        status.MPI_ERROR != 0 ? printf("[Process %i] Catch error code by MPI_Recv: %i\n", g_iProcessRank+1, status.MPI_ERROR)
                              : printf("[Process %i] Successful received %i bytes in %lf seconds\n", g_iProcessRank+1, g_iBufferSize, endtime - starttime);
        fflush(stdout);
        if(nextProcessNumber == g_iTotalProcessCount)
            nextProcessNumber = 0;
        memcpy(g_cBuffer, (const void*)&endtime, sizeof(double));
        MPI_Send(g_cBuffer, g_iBufferSize, MPI_CHAR, nextProcessNumber, 0, MPI_COMM_WORLD);
    }
    delete[] g_cBuffer;//Clean memory
    MPI_Finalize();
    return 0;
}