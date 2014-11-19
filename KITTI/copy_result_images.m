function copy_result_images

dir_dpm = '../Exemplar_DPM/result_images_train';
dir_nms = '../ACF/result_images_train';
dir_occ = '../ContextW/result_images_train';

indexes = [83, 99, 120, 173, 309, 323, 415, 493, 640, 649, ...
    757, 778, 986, 1591, 1679, 1723, 2158, 2544, 2570, 2592];
N = numel(indexes);

for i = 1:N
    src_file = fullfile(dir_dpm, sprintf('%06d_dpm.png', indexes(i)));
    dst_file = sprintf('../%06d_dpm.png', indexes(i));
    disp(src_file);
    copyfile(src_file, dst_file);
    
    src_file = fullfile(dir_nms, sprintf('%06d_nms.png', indexes(i)));
    dst_file = sprintf('../%06d_nms.png', indexes(i));
    disp(src_file);
    copyfile(src_file, dst_file);
    
    src_file = fullfile(dir_occ, sprintf('%06d_occ.png', indexes(i)));
    dst_file = sprintf('../%06d_occ.png', indexes(i));
    disp(src_file);
    copyfile(src_file, dst_file);    
end