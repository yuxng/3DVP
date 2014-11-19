function copy_result_images_test

dir_occ = '../ContextW/result_images_test';

indexes = [5, 13, 23, 27, 35, 48, 57, 72, 93, 95, ...
    108, 133, 150, 184, 196, 198, 210, 257, 264, 310, ...
    348, 357, 397, 422, 439, 457, 510, 559, 702, 769];
N = numel(indexes);

for i = 1:N
    src_file = fullfile(dir_occ, sprintf('%06d.png', indexes(i)));
    dst_file = sprintf('../%06d.png', indexes(i));
    disp(src_file);
    copyfile(src_file, dst_file);    
end