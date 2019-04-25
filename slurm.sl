#!/bin/bash
#SBATCH -N 2
#SBATCH -n 2
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=16
#SBATCH --time=00:30:00
#SBATCH --job-name=coms425_pathtracer

export OPM_PROC_BIND=true
export OMP_PLACES=cores
export OMP_NUM_THREADS=16

echo 1 Node, 1 Process, 4 Threads
srun ./main-mpi -s 250 -w 512 -t 16 -o slurm-test.ppm

