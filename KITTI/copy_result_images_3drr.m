function copy_result_images_3drr

dir_occ = '../ContextW/result_images_3drr';

indexes = [17, 30, 50, 54, 61, 70, 81, 83, 86, 95, ...
    97, 100, 107, 109, 110, 128, 130, 143, 149, 153, ...
    155, 159, 166, 167, 180, 181, 184, 186, 191, 197, 198];
N = numel(indexes);

for i = 1:N
    src_file = fullfile(dir_occ, sprintf('%04d.png', indexes(i)));
    dst_file = sprintf('../%04d.png', indexes(i));
    disp(src_file);
    copyfile(src_file, dst_file);    
end