#!/bin/bash
#PBS -S /bin/bash
#PBS -N run_it1
#PBS -l nodes=1:ppn=12
#PBS -l mem=2gb
#PBS -l walltime=48:00:00
#PBS -q cvgl
echo "I ran on:"
cat $PBS_NODEFILE
cd /scail/scratch/u/yuxiang/SLM/ACF
matlab.new -nodesktop -nosplash -r "exemplar_dpm_train_and_test(1:2); exit;"