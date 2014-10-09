function show_occlusion_patterns

is_save = 1;

opt = globals();
root_dir = opt.path_kitti_root;
data_set = 'training';
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

% annotations
path_ann = 'Annotations';
files = dir(fullfile(path_ann, '*.mat'));
N = numel(files);

% load the mean CAD model
cls = 'car';
filename = sprintf('../Geometry/%s_kitti.mat', cls);
object = load(filename);
cads = object.(cls);

hf = figure;
ind_plot = 1;
mplot = 1;
nplot = 3;
for i = 3825:N
    % load annotation
    filename = fullfile(path_ann, files(i).name);
    disp(filename);
    object = load(filename);
    record = object.record;
    objects = record.objects;
    
    for j = 1:numel(objects)
        object = objects(j);
        if strcmp(object.type, 'Car') == 1% && object.occ_per > 0.05 && object.occ_per < 0.95 
            subplot(mplot, nplot, ind_plot);
            cla;
            cad = cads(object.cad_index);
            draw_cad(cad, object.grid_origin);
            view(object.azimuth, object.elevation);
%             til = sprintf('%s, object %d, occ=%.2f', files(i).name, j, object.occ_per);
%             title(til);
            ind_plot = ind_plot + 1;
            
            % show 2D pattern
            subplot(mplot, nplot, ind_plot);
            cla;
            pattern = object.pattern;
            im = create_mask_image(pattern);
            imshow(im);
            ind_plot = ind_plot + 1;
            axis on;
            xlabel('x');
            ylabel('y');
            
            % show the image patch
            filename = sprintf('%s/%06d.png',image_dir, i - 1);
            I = imread(filename);
            bbox_gt = [object.x1 object.y1 object.x2 object.y2];
            rect = [bbox_gt(1) bbox_gt(2) bbox_gt(3)-bbox_gt(1) bbox_gt(4)-bbox_gt(2)];
            I1 = imcrop(I, rect);
            subplot(mplot, nplot, ind_plot);
            cla;
            ind_plot = ind_plot + 1;
            imshow(I1);            
            
            if ind_plot > mplot * nplot
                ind_plot = 1;
                if is_save
                    filename = sprintf('Patterns/%06d_%d.png', i - 1, j);
                    saveas(hf, filename);
                else
                    pause;
                end
            end
        end
    end
end