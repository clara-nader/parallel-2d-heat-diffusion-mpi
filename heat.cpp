#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

const double PI = acos(-1.0);

int main(int argc, char** argv) {
    try {
        int N = 64, steps = -1;
        double r = 0.2, T = -1.0;
        string test = "1";
        string decomp = "serial";
        string output = "";
        bool allow_unstable = false;

        // Read command-line arguments
        for (int a = 1; a < argc; ++a) {
            string arg = argv[a];

            if (arg == "--allow-unstable") {
                allow_unstable = true;
                continue;
            }

            if (a + 1 >= argc)
                throw runtime_error("Missing value for " + arg);

            string val = argv[++a];

            if (arg == "--case") test = val;
            else if (arg == "--N") N = stoi(val);
            else if (arg == "--r") r = stod(val);
            else if (arg == "--steps") steps = stoi(val);
            else if (arg == "--T") T = stod(val);
            else if (arg == "--out") output = val;
            else if (arg == "--decomp") decomp = val;
            else throw runtime_error("Unknown option: " + arg);
        }

        if (N < 2)
            throw runtime_error("N must be at least 2");

        if (test != "1" && test != "2" && test != "V")
            throw runtime_error("Case must be 1, 2 or V");

        if (decomp != "serial")
            throw runtime_error("Only serial is implemented");

        if (!isfinite(r) || r <= 0)
            throw runtime_error("r must be positive and finite");

        if (r > 0.25 && !allow_unstable)
            throw runtime_error(
                "Unstable r: must satisfy r <= 0.25"
            );

        if (steps >= 0 && T >= 0)
            throw runtime_error("Use --steps or --T, not both");

        if (T >= 0 && !isfinite(T))
            throw runtime_error("Invalid final time");

        double h = 1.0 / N;
        double dt = r * h * h;

        // If T is supplied, adjust dt to reach T exactly
        if (T >= 0) {
            if (T <= 0)
                throw runtime_error("T must be positive");

            steps = static_cast<int>(ceil(T / dt));
            if (steps < 1) steps = 1;

            dt = T / steps;
            r = dt / (h * h);
        } else {
            if (steps < 0) steps = 500;
            if (steps < 1)
                throw runtime_error("steps must be positive");
            T = steps * dt;
        }

        int width = N + 1;

        // Single indexing function
        auto idx = [width](int i, int j) {
            return j * width + i;
        };

        // Two contiguous arrays
        vector<double> u_old(width * width, 0.0);
        vector<double> u_new(width * width, 0.0);

        // Initial and boundary conditions
        for (int j = 0; j <= N; ++j) {
            for (int i = 0; i <= N; ++i) {
                double value = 0.0;

                if (test == "V") {
                    value = sin(PI * i * h)
                          * sin(PI * j * h);

                    if (i == 0 || i == N ||
                        j == 0 || j == N)
                        value = 0.0;
                }
                else if (test == "1") {
                    if (i == 0 || i == N ||
                        j == 0 || j == N)
                        value = 1.0;
                }
                else if (test == "2") {
                    // Top and bottom, including corners, are hot
                    if (j == 0 || j == N)
                        value = 1.0;
                }

                u_old[idx(i, j)] = value;
                u_new[idx(i, j)] = value;
            }
        }

        // Time only the simulation loop
        auto start = chrono::steady_clock::now();

        for (int step = 0; step < steps; ++step) {
            for (int j = 1; j < N; ++j) {
                for (int i = 1; i < N; ++i) {
                    int k = idx(i, j);

                    u_new[k] =
                        (1.0 - 4.0*r) * u_old[k]
                        + r * (
                            u_old[idx(i+1, j)]
                          + u_old[idx(i-1, j)]
                          + u_old[idx(i, j+1)]
                          + u_old[idx(i, j-1)]
                        );
                }
            }

            // Swap instead of copying entire arrays
            u_old.swap(u_new);
        }

        auto stop = chrono::steady_clock::now();

        double elapsed =
            chrono::duration<double>(stop-start).count();

        // Error norms for verification case
        double err_inf = 0.0;
        double err_sum = 0.0;

        if (test == "V") {
            double decay = exp(-2.0*PI*PI*T);

            for (int j = 0; j <= N; ++j) {
                for (int i = 0; i <= N; ++i) {
                    double exact =
                        sin(PI*i*h)*sin(PI*j*h)*decay;

                    double error =
                        u_old[idx(i,j)] - exact;

                    err_inf = max(err_inf, abs(error));
                    err_sum += error * error;
                }
            }
        }

        double err_l2 = h * sqrt(err_sum);

        // Full grid, row-major, doubles, no header
        if (!output.empty()) {
            ofstream file(output, ios::binary);

            if (!file)
                throw runtime_error("Cannot open output file");

            file.write(
                reinterpret_cast<const char*>(u_old.data()),
                u_old.size() * sizeof(double)
            );

            if (!file)
                throw runtime_error("Cannot write output file");
        }

        // Summary line for automated measurements
        printf(
            "decomp=serial P=1 case=%s N=%d "
            "r=%.12g steps=%d T=%.12g "
            "t_total=%.9f",
            test.c_str(), N, r, steps, T, elapsed
        );

        if (test == "V") {
            printf(
                " err_inf=%.12e err_l2=%.12e",
                err_inf, err_l2
            );
        }

        printf("\n");
        return 0;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
