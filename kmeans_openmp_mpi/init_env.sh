#! /bin/bash

# Execute the script in order to install all the necessary commands on a cluster of 4 machines running Ubuntu 18
# 	- ssh key of the master node need to be included into ~/.ssh/authorized_key
# 	- supposed cluster configuration: node-0, node-1, node-2, node-3

NODES='node-0
node-1
node-2
node-3'

printf "$NODES" > ./slaves

# Test ssh connection between nodes
parallel-ssh -i -h ./slaves -t 0 -O StrictHostKeyChecking=no hostname
# Set up aliases
shopt -s expand_aliases
alias blast='parallel-ssh -i -h ./slaves -t 0'

echo 'Updating the system'
blast "sudo apt-get update --fix-missing" >/dev/null
echo 'Installing performance monitoring tools'
blast "sudo apt-get install --assume-yes htop dstat sysstat iperf3" >/dev/null 

exit
