# VNS-EVRP-2020

This repository contains the Variable Neighborhood Search metaheuristic algorithm for the Electric Vehicle Routing Problem (EVRP), the winning method of the IEEE WCCI 2020 competition on EVRP. 
The code is as submitted to the competition. 

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Run

Place EVRP instance files in the `data/` directory, then run the solver from the `build/` directory:

```sh
cd build
./VNS-David E-n22-k4.evrp
```

The executable now runs 10 trials automatically using seeds `1` through `10`.

## Output

For an instance named `E-n22-k4.evrp`, results are written under:

```text
stats/VNS/E-n22-k4/
```

This directory contains:

- `stats.E-n22-k4.txt`: summary statistics over the 10 runs
- `1/` ... `10/`: one subdirectory per seed
- `solution.E-n22-k4.txt`: best solution found for that seed
- `evols.E-n22-k4.csv`: objective value over time/evaluations for that seed

Example output layout:

```text
stats/VNS/E-n22-k4/
  stats.E-n22-k4.txt
  1/
    solution.E-n22-k4.txt
    evols.E-n22-k4.csv
  2/
  ...
  10/
```

## Instance Format

The current parser supports the EVRP instances in `data/`, including files that use:

- `NODE_COORD_SECTION`
- `DEMAND_SECTION`
- `STATIONS_COORD_SECTION`
- `DEPOT_SECTION`
