function exemplar_make_pbs_scripts

% load occlusion patterns
filename = '../../KITTI/data.mat';
object = load(filename);
data = object.data;
cids = unique(data.idx_ap2);
num = numel(cids);

for o_i = 1:num
    
  fid = fopen(sprintf('run%d.sh', o_i), 'w');

  fprintf(fid, '#!/bin/bash\n');
  fprintf(fid, '#PBS -S /bin/bash\n');
  fprintf(fid, '#PBS -N run_it%d\n', o_i);
  fprintf(fid, '#PBS -l nodes=1:ppn=12\n');
  fprintf(fid, '#PBS -l mem=2gb\n');
  fprintf(fid, '#PBS -l walltime=48:00:00\n');
  fprintf(fid, '#PBS -q cvgl\n');
  
  fprintf(fid,'cd /scail/scratch/u/yuxiang/SLM/voc-release5\n');
  fprintf(fid, ['matlab.new -nodesktop -nosplash -r "exemplar_dpm_train(' int2str(o_i) '); exit;"']);
  
  fclose(fid);
end

fid = fopen('exemplar_qsub.sh', 'w');
fprintf(fid, 'for (( i = 1; i <= %d; i++))\n', num);
fprintf(fid, 'do\n');
fprintf(fid, '  qsub run$i.sh\n');
fprintf(fid, 'done\n');
fclose(fid);