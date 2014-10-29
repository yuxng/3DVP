function write_kitti_result(filename, det, data)

imsize = [1224, 370]; % kittisize

disp(filename);
fid = fopen(filename, 'w');

if isempty(det) == 1
    fprintf('no detection for image %s\n', filename);
    fclose(fid);
    return;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% truncated..
det(det(:, 1) < 0, 1) = 0;
det(det(:, 2) < 0, 2) = 0;
det(det(:, 1) > imsize(1), 1) = imsize(1);
det(det(:, 2) > imsize(2), 2) = imsize(2);
det(det(:, 3) < 0, 1) = 0;
det(det(:, 4) < 0, 2) = 0;
det(det(:, 3) > imsize(1), 3) = imsize(1);
det(det(:, 4) > imsize(2), 4) = imsize(2);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% write detections
num = size(det, 1);
for k = 1:num
    cid = det(k, 5);
    truncation = data.truncation(cid);

    occ_per = data.occ_per(cid);
    if occ_per > 0.5
        occlusion = 2;
    elseif occ_per > 0
        occlusion = 1;
    else
        occlusion = 0;
    end

    azimuth = data.azimuth(cid);
    alpha = azimuth + 90;
    if alpha >= 360
        alpha = alpha - 360;
    end
    
    alpha = alpha*pi/180;
    if alpha > pi
        alpha = alpha - 2*pi;
    end

    h = data.h(cid);
    w = data.w(cid);
    l = data.l(cid);

    fprintf(fid, '%s %f %d %f %f %f %f %f %f %f %f %f %f %f %f %f\n', ...
        'Car', truncation, occlusion, alpha, det(k,1), det(k,2), det(k,3), det(k,4), ...
        h, w, l, -1, -1, -1, -1, det(k,6));
end
fclose(fid);

end