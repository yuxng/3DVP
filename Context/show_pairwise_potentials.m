function show_pairwise_potentials(I, dets, data)

N = size(dets, 1);

% for each pair of detections
for i = 1:N
    for j = i+1:N
        % show the two bounding boxes
        imshow(I);
        hold on;
        bbox_pr = dets(i, 1:4);
        bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
        rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
        
        bbox_pr = dets(j, 1:4);
        bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
        rectangle('Position', bbox_draw, 'EdgeColor', 'r', 'LineWidth', 2);
        pause;
    end
end