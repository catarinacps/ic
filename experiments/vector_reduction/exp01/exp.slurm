#!/bin/bash
#SBATCH --time=72:00:00
#SBATCH --workdir=.
#SBATCH --output=/home/users/hcpsilva/slurm_outputs/%x_%j.out
#SBATCH --error=/home/users/hcpsilva/slurm_outputs/%x_%j.err
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=hcpsilva@inf.ufrgs.br

# parameters
# the experiment ID, defined in the lab-book
EXP_ID=$1
# the experiment directory
EXP_DIR=$2

# as defined in the experiment design
MAX_VALUE=100

# experiment name (which is the ID and the machine and its core count)
EXP_NAME=${EXP_ID}_${HOST}_${SLURM_CPUS_ON_NODE}

# go to the scratch dir to execute our operations
cd $SCRATCH

# clean up my scratch dir
rm -rf $SCRATCH/*

# install spack
git clone https://github.com/spack/spack.git
# source spack env variables
. spack/share/spack/setup-env.sh
# add the solverstack INRIA repo
git clone https://gitlab.inria.fr/solverstack/spack-repo.git solverstack
# and add it to spack
spack repo add solverstack

# install starpu and cia
spack install starpu@develop+fxt+poti~examples~mpi~openmp
echo "StarPU installed!"
spack install intel-tbb
echo "TBB intalled!"

# create install dir and put StarPU in it
mkdir install
mkdir install/starpu
mkdir install/tbb
spack view soft install/starpu starpu
spack view soft install/tbb intel-tbb

STARPU_PATH=$(readlink -f install/starpu)
TBB_PATH=$(readlink -f install/tbb)

# set up path and ld path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$STARPU_PATH/lib:$TBB_PATH/lib
export PATH=$PATH:$STARPU_PATH/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$STARPU_PATH/lib/pkgconfig

# prepare our directory
mkdir $EXP_NAME
pushd $EXP_NAME

# copy the code folder
cp -r $EXP_DIR/code code
mkdir results

# compile
pushd code
make LIB_EXTRA="-L$TBB_PATH/lib" INC_EXTRA="-I$TBB_PATH/include"
popd

# init the results csv
results_csv=results/${HOST}_data.csv
echo "node,rep_id,vector_size,num_blocks,reduc_fac,compute_time" > $results_csv

# execute the program
while read -r id vector_size num_blocks reduc_fac; do
    echo "-> Parameters set to: $vector_size $num_blocks $reduc_fac"

    # execute with given configurations
    c_time=$(./code/build/starpu $vector_size $num_blocks $reduc_fac $MAX_VALUE)

    # add execution data to csv
    echo ${HOST},${id},${vector_size},${num_blocks},${reduc_fac},${c_time} >> $results_csv

    # stress the memory to prevent cache influence between runs
    stress-ng --vm 3 --vm-bytes 75% -t 5s &> /dev/null

    echo
done < $EXP_DIR/runs.plan

# zip everything and commit to EXP_DIR
tar czf $EXP_DIR/${EXP_NAME}_data.tar.gz *

popd
rm -rf $SCRATCH/*
