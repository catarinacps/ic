#!/bin/bash
PARTITIONS='draco hype tupi'
# the experiment id
EXPERIMENT_ID=$1

# the work (repo) dir
REPO_DIR=$2

if [[ $REPO_DIR != /* ]]; then
    echo "Path to repository is not absolute, please use the absolute path..."
    exit
fi

EXP_DIR=$REPO_DIR/experiments/vector_reduction/$EXPERIMENT_ID
pushd $REPO_DIR

# always update and overwrite the code dir
git pull
cp -r code $EXP_DIR/code

for name in $PARTITIONS; do
    nodes=$(gppd-info --long --Node -S NODELIST -p $name -h | awk '{print $1 "_" $5}' | paste -s -d" " -)

    for execution in $nodes; do
        # launch the slurm script for this node
        sbatch \
            -w ${execution%%_*} \
            -c ${execution#*_} \
            -J ${EXPERIMENT_ID}_${execution}_${USER} \
            $EXPERIMENT_ID/exp.slurm $EXPERIMENT_ID $EXP_DIR
    done
done

popd
