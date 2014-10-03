% cls : class (e.g : car, mug, bed...)
% record : Annotation mat file
% cad_bunch : bunch of models in a corresponding category
% rextent : rotation extent 
% rextent is usually set pi/4

% azi_co : azimuth coarse which is initial azimuth set by manually rotating the CAD model 
% ele_co : elevation coarse which is initial azimuth set by manually rotating the CAD model
% num_anchor : number of visible anchor points selected on the image
% num_ob : number of corresponding objects in a input image
% filename : image file name
% Author: Taewon Kim

function [azimuth, elevation, azi_co, ele_co, distance, focal, px, py, theta,...
    error, interval_azimuth, interval_elevation, num_anchor, ob_index, record] = ...
    view_estimator(classes, record, cad_bunch, use_nonvisible, clear_nonvisible, rescales)

ttnum = numel(record.objects);
j = 1;
ob_index = [];
class_index = [];
for i = 1:ttnum
    index = find(strcmp(record.objects(i).class, classes) == 1);
    if isempty(index) == 0
        ob_index(j) = i;
        class_index(j) = index;
        j = j + 1;
    end
end

num_ob = numel(ob_index);
azimuth = zeros(num_ob,1);
elevation = zeros(num_ob,1);
azi_co = zeros(num_ob,1);
ele_co = zeros(num_ob,1);
distance = zeros(num_ob,1);
focal = zeros(num_ob,1);
px = zeros(num_ob,1);
py = zeros(num_ob,1);
theta = zeros(num_ob,1);
error = zeros(num_ob,1);
interval_azimuth = zeros(num_ob,1);
interval_elevation = zeros(num_ob,1);
num_anchor = zeros(num_ob,1);

% for each object
for i = 1:num_ob
    chrt = record.objects(1,ob_index(i));
    anchors = chrt.anchors;
    viewpoint = chrt.viewpoint;
    cad_index = chrt.cad_index;
    cad = cad_bunch{class_index(i)}(cad_index);

    % get anchor point annotations
    part_num = numel(cad.pnames);
    pnames = cad.pnames;
    x2d = [];
    x3d = [];
    for j = 1:part_num
        if anchors.(pnames{j}).status == 1  || (use_nonvisible == 1 && ...
                isempty(anchors.(pnames{j}).location) == 0)
            p = anchors.(pnames{j}).location;
            x2d = [x2d; p];
            x3d = [x3d; cad.(pnames{j})];
        elseif clear_nonvisible == 1
            if isempty(anchors.(pnames{j}).location) == 0
                record.objects(ob_index(i)).anchors.(pnames{j}).location = [];
                fprintf('%s object %d part %s status %d nonempty location\n', ...
                    record.filename, ob_index(i), pnames{j}, anchors.(pnames{j}).status);
            end
        end
    end
    x3d = rescales{class_index(i)}(cad_index) * x3d;
    
    % compute continous viewpoint
    num_anch = size(x2d,1);
    if num_anch < 2
          azi_co(i) = viewpoint.azimuth_coarse;
          ele_co(i) = viewpoint.elevation_coarse;
    else
        % inialization
        v0 = zeros(7,1);
        % azimuth
        a = viewpoint.azimuth_coarse;
        v0(1) = a*pi/180;
        margin = 22.5;
        aextent = [max(a-margin,0)*pi/180 min(a+margin,360)*pi/180];
        % elevation
        e = viewpoint.elevation_coarse;
        v0(2) = e*pi/180;
        margin = 22.5;
        eextent = [max(e-margin,-90)*pi/180 min(e+margin,90)*pi/180];        
        % distance
        dextent = [0, 100];
        v0(3) = compute_distance(v0(1), v0(2), dextent, x2d, x3d);
        d = v0(3);
        margin = 5;
        dextent = [max(d-margin,0) min(d+margin,100)];
        % focal length
        v0(4) = 1;
        fextent = [1 1];
        % principal point
        [principal_point, lbp, ubp] = compute_principal_point(v0(1), v0(2), v0(3), x2d, x3d);
        v0(5) = principal_point(1);
        v0(6) = principal_point(2);
        % in-plane rotation
        v0(7) = 0;
        rextent = [-pi, pi];
        % lower bound
        lb = [aextent(1); eextent(1); dextent(1); fextent(1); lbp(1); lbp(2); rextent(1)];
        % upper bound
        ub = [aextent(2); eextent(2); dextent(2); fextent(2); ubp(1); ubp(2); rextent(2)];
        
        % optimization
        v_out = zeros(10,1);
        [v_out(1), v_out(2), v_out(3), v_out(4), v_out(5), v_out(6), v_out(7), v_out(8), v_out(9), v_out(10)]...
            = compute_viewpoint_one(v0, lb, ub, x2d, x3d);
        
        % assign output
        azimuth(i) = v_out(1);
        elevation(i) = v_out(2);
        distance(i) = v_out(3); 
        focal(i) = v_out(4);
        px(i) = v_out(5);
        py(i) = v_out(6);
        theta(i) = v_out(7);
        error(i) = v_out(8);
        interval_azimuth(i) = v_out(9);
        interval_elevation(i) = v_out(10);
        num_anchor(i) = num_anch;
        azi_co(i) = viewpoint.azimuth_coarse;
        ele_co(i) = viewpoint.elevation_coarse;
    end
end

% compute the initial distance
function distance = compute_distance(azimuth, elevation, dextent, x2d, x3d)

% compute pairwise distance
n = size(x2d, 1);
num = n*(n-1)/2;
d2 = zeros(num,1);
count = 1;
for i = 1:n
    for j = i+1:n
        d2(count) = norm(x2d(i,:)-x2d(j,:));
        count = count + 1;
    end
end

% optimization
options = optimset('Algorithm', 'interior-point');
distance = fmincon(@(d)compute_error_distance(d, azimuth, elevation, d2, x3d),...
    (dextent(1)+dextent(2))/2, [], [], [], [], dextent(1), dextent(2), [], options);

function error = compute_error_distance(distance, azimuth, elevation, d2, x3d)

a = azimuth;
e = elevation;
d = distance;
f = 1;

% camera center
C = zeros(3,1);
C(1) = d*cos(e)*sin(a);
C(2) = -d*cos(e)*cos(a);
C(3) = d*sin(e);

a = -a;
e = -(pi/2-e);

% rotation matrix
Rz = [cos(a) -sin(a) 0; sin(a) cos(a) 0; 0 0 1];   %rotate by a
Rx = [1 0 0; 0 cos(e) -sin(e); 0 sin(e) cos(e)];   %rotate by e
R = Rx*Rz;

% perspective project matrix
M = 3000;
P = [M*f 0 0; 0 -M*f 0; 0 0 -1] * [R -R*C];

% project
x = P*[x3d ones(size(x3d,1), 1)]';
x(1,:) = x(1,:) ./ x(3,:);
x(2,:) = x(2,:) ./ x(3,:);
x = x(1:2,:)';

% compute pairwise distance
n = size(x, 1);
num = n*(n-1)/2;
d3 = zeros(num,1);
count = 1;
for i = 1:n
    for j = i+1:n
        d3(count) = norm(x(i,:)-x(j,:));
        count = count + 1;
    end
end

% compute error
error = norm(d2-d3);

function [center, lb, ub] = compute_principal_point(a, e, d, x2d, x3d)

f = 1;
% camera center
C = zeros(3,1);
C(1) = d*cos(e)*sin(a);
C(2) = -d*cos(e)*cos(a);
C(3) = d*sin(e);

a = -a;
e = -(pi/2-e);

% rotation matrix
Rz = [cos(a) -sin(a) 0; sin(a) cos(a) 0; 0 0 1];   %rotate by a
Rx = [1 0 0; 0 cos(e) -sin(e); 0 sin(e) cos(e)];   %rotate by e
R = Rx*Rz;

% perspective project matrix
M = 3000;
P = [M*f 0 0; 0 -M*f 0; 0 0 -1] * [R -R*C];

% project
x = P*[x3d ones(size(x3d,1), 1)]';
x(1,:) = x(1,:) ./ x(3,:);
x(2,:) = x(2,:) ./ x(3,:);
x = x(1:2,:)';

% project object center
c = P*[0 0 0 1]';
c = c ./ c(3);
c = c(1:2)';

% predict object center
cx2 = c(1);
cy2 = c(2);
center = [0 0];
for i = 1:size(x2d,1)
    cx1 = x(i,1);
    cy1 = x(i,2);
    dc = sqrt((cx1-cx2)*(cx1-cx2) + (cy1-cy2)*(cy1-cy2));
    ac = atan2(cy2-cy1, cx2-cx1);
    center(1) = center(1) + x2d(i,1) + dc*cos(ac);
    center(2) = center(2) + x2d(i,2) + dc*sin(ac);
end
center = center ./ size(x2d,1);

width = 0;
height = 0;
for i = 1:size(x2d,1)
    w = abs(x2d(i,1)-center(1));
    if width < w
        width = w;
    end
    h = abs(x2d(i,2)-center(2));
    if height < h
        height = h;
    end
end

% lower bound and upper bound
lb = [center(1)-width/10 center(2)-height/10];
ub = [center(1)+width/10 center(2)+height/10];

% compute viewpoint angle from 2D-3D correspondences
function [azimuth, elevation, distance, focal, px, py, theta, error, interval_azimuth, interval_elevation]...
    = compute_viewpoint_one(v0, lb, ub, x2d, x3d)

options = optimset('Algorithm', 'interior-point');
[vp, fval] = fmincon(@(v)compute_error(v, x2d, x3d),...
    v0, [], [], [], [], lb, ub, [], options);

viewpoint = vp;
error = fval;

azimuth = viewpoint(1)*180/pi;
if azimuth < 0
    azimuth = azimuth + 360;
end
if azimuth >= 360
    azimuth = azimuth - 360;
end
elevation = viewpoint(2)*180/pi;
distance = viewpoint(3);
focal = viewpoint(4);
px = viewpoint(5);
py = viewpoint(6);
theta = viewpoint(7)*180/pi;

% estimate confidence inteval
v = viewpoint;
v(7) = 0;
x = project(v, x3d);
% azimuth
v = viewpoint;
v(1) = v(1) + pi/180;
xprim = project(v, x3d);
error_azimuth = sum(diag((x-xprim) * (x-xprim)'));
interval_azimuth = error / error_azimuth;
% elevation
v = viewpoint;
v(2) = v(2) + pi/180;
xprim = project(v, x3d);
error_elevation = sum(diag((x-xprim) * (x-xprim)'));
interval_elevation = error / error_elevation;

function error = compute_error(v, x2d, x3d)

a = v(1);
e = v(2);
d = v(3);
f = v(4);
principal_point = [v(5) v(6)];
theta = v(7);

% camera center
C = zeros(3,1);
C(1) = d*cos(e)*sin(a);
C(2) = -d*cos(e)*cos(a);
C(3) = d*sin(e);

a = -a;
e = -(pi/2-e);

% rotation matrix
Rz = [cos(a) -sin(a) 0; sin(a) cos(a) 0; 0 0 1];   %rotate by a
Rx = [1 0 0; 0 cos(e) -sin(e); 0 sin(e) cos(e)];   %rotate by e
R = Rx*Rz;

% perspective project matrix
M = 3000;
P = [M*f 0 0; 0 M*f 0; 0 0 -1] * [R -R*C];

% project
x = P*[x3d ones(size(x3d,1), 1)]';
x(1,:) = x(1,:) ./ x(3,:);
x(2,:) = x(2,:) ./ x(3,:);
x = x(1:2,:);

% rotation matrix 2D
R2d = [cos(theta) -sin(theta); sin(theta) cos(theta)];
x = (R2d * x)';
% compute error
error = normal_dist(x, x2d, principal_point);

% re-projection error
function error = normal_dist(x, x2d, p_pnt)

error = 0;
for i = 1:size(x2d, 1)
     point = x2d(i,:) - p_pnt;
     point(2) = -1 * point(2);
     error = error + (point-x(i,:))*(point-x(i,:))'/size(x2d, 1);
end

function x = project(v, x3d)

a = v(1);
e = v(2);
d = v(3);
f = v(4);
theta = v(7);

% camera center
C = zeros(3,1);
C(1) = d*cos(e)*sin(a);
C(2) = -d*cos(e)*cos(a);
C(3) = d*sin(e);

a = -a;
e = -(pi/2-e);

% rotation matrix
Rz = [cos(a) -sin(a) 0; sin(a) cos(a) 0; 0 0 1];   %rotate by a
Rx = [1 0 0; 0 cos(e) -sin(e); 0 sin(e) cos(e)];   %rotate by e
R = Rx*Rz;

% perspective project matrix
M = 3000;
P = [M*f 0 0; 0 M*f 0; 0 0 -1] * [R -R*C];

% project
x = P*[x3d ones(size(x3d,1), 1)]';
x(1,:) = x(1,:) ./ x(3,:);
x(2,:) = x(2,:) ./ x(3,:);
x = x(1:2,:);

% rotation matrix 2D
R2d = [cos(theta) -sin(theta); sin(theta) cos(theta)];
x = (R2d * x)';