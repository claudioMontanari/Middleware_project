#! /bin/bash

# Execute the script in order to install all the necessary commands on a
# cluster of machines running Ubuntu 18. Requirements:
# 	- ssh key of the master node need to be included into ~/.ssh/authorized_key
#
# Usage: ./init_script.sh "node-1,node-2,node-3,node-4" 

if [ "$#" -ne 1 ]; then
    echo 'Error, usage: init_script.sh "node-1,node-2"'
    exit
fi

read -a NODES <<< $1
    
echo ${NODES[@]} | tr ',' '\n' > ./slaves


# Test ssh connection between nodes
parallel-ssh -i -h ./slaves -t 0 -O StrictHostKeyChecking=no hostname
# Set up aliases
shopt -s expand_aliases
alias blast='parallel-ssh -i -h ./slaves -t 0'

echo 'Updating the system'
blast "sudo apt-get update --fix-missing" >/dev/null
echo 'Installing performance monitoring tools'
blast "sudo apt-get install --assume-yes gnuplot htop dstat sysstat" >/dev/null

echo 'Installing MPI tools'
blast "sudo apt-get install --assume-yes openmpi-bin openmpi-common openssh-client openssh-server libopenmpi2 libopenmpi-dev" >/dev/null

echo 'Run make in order to generate all the executables'

exit
