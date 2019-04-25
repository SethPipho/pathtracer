#!/bin/bash
#SBATCH -N 10
#SBATCH -n 160
#SBATCH --ntasks-per-node=16
#SBATCH --time=00:30:00
#SBATCH --job-name=coms425_pathtracer

srun ./main-mpi -t 1 -s 1000 -w 1024 -o slurm-test.ppm

