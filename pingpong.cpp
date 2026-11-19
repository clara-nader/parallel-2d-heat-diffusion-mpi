#include <mpi.h>
#include <iostream>
#include <vector>
#include <iomanip>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0)
            std::cerr << "Run with exactly 2 processes\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0)
        std::cout << "bytes,rtt_seconds,oneway_seconds\n";

    for (int m = 8; m <= 16 * 1024 * 1024; m *= 2) {
        std::vector<char> buffer(m, 0);

        int repetitions = (m <= 65536) ? 20 : 5;
        int warmup = 2;

        for (int i = 0; i < warmup; ++i) {
            if (rank == 0) {
                MPI_Send(buffer.data(), m, MPI_BYTE, 1, 0,
                         MPI_COMM_WORLD);
                MPI_Recv(buffer.data(), m, MPI_BYTE, 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Recv(buffer.data(), m, MPI_BYTE, 0, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(buffer.data(), m, MPI_BYTE, 0, 0,
                         MPI_COMM_WORLD);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();

        for (int i = 0; i < repetitions; ++i) {
            if (rank == 0) {
                MPI_Send(buffer.data(), m, MPI_BYTE, 1, 0,
                         MPI_COMM_WORLD);
                MPI_Recv(buffer.data(), m, MPI_BYTE, 1, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Recv(buffer.data(), m, MPI_BYTE, 0, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(buffer.data(), m, MPI_BYTE, 0, 0,
                         MPI_COMM_WORLD);
            }
        }

        if (rank == 0) {
            double elapsed = MPI_Wtime() - start;
            double rtt = elapsed / repetitions;

            std::cout << m << ","
                      << std::setprecision(12) << rtt << ","
                      << rtt / 2.0 << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}
