# Reproduction package: LTE equipment replacement impact on throughput

Simulation code, scripts and raw data for the paper:

> Sobolev N.S., Cheryapkin D.P. "Experimental evaluation of telecom equipment replacement impact on LTE quality" (2026).

## Requirements
- Ubuntu 24.04 LTS (or WSL2 on Windows)
- 15 GB free disk, 8 GB RAM
- Internet for downloading ns-3.41 and 5G-LENA

## Quick start

### 1. Install environment
    bash scripts/setup.sh

### 2. Copy scenario and build
    cp src/my-lte-5g.cc ~/workspace/ns-allinone-3.41/ns-3.41/scratch/
    cd ~/workspace/ns-allinone-3.41/ns-3.41
    ./ns3 build

### 3. Verify baseline (~15.5 Mbps)
    ./ns3 run "scratch/my-lte-5g --seed=1000 --load=high --enbConfig=foreign --simTime=60"

### 4. Reproduce Table 1 (30 runs, ~2 min)
    bash scripts/run_matrix.sh > my_matrix.csv
    diff my_matrix.csv data/matrix_results_5seed.csv

## Repository structure
- src/ — simulation scenario (my-lte-5g.cc)
- scripts/ — setup and run scripts
- data/ — raw results (CSV)
- figures/ — figure generation scripts (to be added)

## Versions
See versions.txt. Tested on Ubuntu 24.04.5 LTS, GCC 13.3.0, CMake 3.28.3, Python 3.12.3, ns-3.41, 5G-LENA v3.0.

## Known caveats
- simTime=60 gives 19/20 UE attach at seed=1000/load=low; use --simTime=300 for 20/20.
- Deterministic: same seed produces the same result bit-for-bit.

## License
MIT — see LICENSE.
