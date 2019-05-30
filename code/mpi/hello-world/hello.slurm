#!/bin/bash

#SBATCH --nodes=5
#SBATCH --ntasks=80
#SBATCH --time=02:00:00
#SBATCH --partition=draco
#SBATCH --workdir=.
#SBATCH --output=%x_%j.out
#SBATCH --error=%x_%j.err

# Prepare the machien file
MACHINEFILE="nodes.$SLURM_JOB_ID"
srun -l /bin/hostname | sort -n | awk '{print $2}' > $MACHINEFILE

# Get number of cores available
NP=$(cat $MACHINEFILE | wc -l)

# Compile the program
mpicc hello.c -o hello

# Execute the program
`which mpirun` \
	--mca oob_tcp_if_include 192.168.30.0/24 \
	--mca btl_tcp_if_include 192.168.30.0/24 \
	--mca btl_base_warn_component_unused 0 \
	-np $NP \
	-machinefile $MACHINEFILE \
	~/hello
