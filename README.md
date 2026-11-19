Parallel 2D heat diffusion with MPI
Explicit finite-difference solver for the 2D heat equation on the unit square
(alpha = 1, Dirichlet boundaries), using the scheme of Task 5.2 of the
background notes:
    u_new(i,j) = (1 - 4r) u(i,j) + r * (u(i+1,j) + u(i-1,j) + u(i,j+1) + u(i,j-1)),
    r = alpha * dt / h^2,  h = 1/N

The scheme is stable only for r <= 1/4 (Task 6.3); the program refuses larger values.
Build
    make

(or by hand: `g++ -std=c++17 -O3 -ffp-contract=off -o heat src/heat.cpp`)
`-ffp-contract=off` disables fused multiply-add, so every machine computes
exactly the same floating-point results.
Run on one VM
    ./heat --case V --N 64 --r 0.2 --T 0.05
    ./heat --case 1 --N 256 --r 0.2 --T 0.1 --out case1.bin
    ./heat --case 2 --N 256 --r 0.25 --steps 20000 --out case2.bin

Run on the cluster
Not available yet: the parallel versions (`--decomp strips`, `--decomp blocks`)
come in Parts 2 and 3. They will be launched from one VM with:
    mpirun --hostfile hosts -np 6 \
           --mca oob_tcp_if_include tailscale0 --mca btl_tcp_if_include tailscale0 \
           --mca btl tcp,self ./heat --decomp strips ...

Options
Option	Meaning	Default	Example
`--case`	`1` (four hot edges), `2` (hot top and bottom, corners included), `V` (verification test)	`1`	`--case 2`
`--N`	number of intervals per direction, h = 1/N (at least 2)	`64`	`--N 1024`
`--r`	alpha dt / h^2; must be positive and <= 0.25	`0.2`	`--r 0.2`
`--T`	final time; dt is chosen so that T/dt is an integer and r is not exceeded		`--T 0.05`
`--steps`	fixed number of steps, with dt = r h^2	`500` if neither `--T` nor `--steps` is given	`--steps 500`
`--out`	write the final field to this file (optional)	no file	`--out u.bin`
`--decomp`	`serial` only for now	`serial`	`--decomp serial`
`--allow-unstable`	run even if r > 0.25 (only for Task 6.7)	off	
`--T` and `--steps` cannot be used together.
Standard output
One line of `key=value` pairs, for example:
    decomp=serial P=1 case=V N=64 r=0.2 steps=1024 T=0.05 t_total=0.001962885 err_inf=1.034424878135e-04 err_l2=5.172124390697e-05

`r` is the value actually used (with `--T` it can be slightly below the requested one).
`t_total` is the time of the time-stepping loop only (std::chrono::steady_clock),
without initialisation or output.
`err_inf` and `err_l2` (the norms of Definition 7.1, `err_l2` = sqrt(h^2 * sum e^2))
are printed for test V only, at the final time.
Field file (`--out`)
The full (N+1) x (N+1) grid, boundary included, as raw little-endian doubles in
row-major order: row j is the line y = y_j, and i varies fastest. No header.
Node (i, j) is value number `j*(N+1) + i`.
# parallel-2d-heat-diffusion-mpi

