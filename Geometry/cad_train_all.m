function cad_train_all

% cls = {'aeroplane', 'bicycle', 'boat', 'bottle', 'bus', 'chair', 'diningtable', ...
%     'motorbike', 'sofa', 'train', 'tvmonitor'};

cls = {'car'};

for i = 1:numel(cls)
    switch cls{i}
        case 'aeroplane'
            N = 8;
        case 'bicycle'
            N = 6;
        case 'boat'
            N = 6;
        case 'bottle'
            N = 8;
        case 'bus'
            N = 6;     
        case 'car'
            N = 10;
        case 'car_kitti'
            N = 7;
        case 'chair'
            N = 10;
        case 'diningtable'
            N = 6;
        case 'motorbike'
            N = 5;
        case 'sofa'
            N = 6;
        case 'train'
            N = 4;
        case 'tvmonitor'
            N = 4;           
    end
    
    for j = 1:N
        cmd = sprintf('./render_depth_images %s %d', cls{i}, j);
        system(cmd);
    end
    
    cad_train(cls{i});
    cad_train_mean(cls{i});
end