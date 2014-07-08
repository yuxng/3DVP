function visualizemodel(model, nplot, mplot, ind_plot)

% visualizemodel(model)
% Visualize a model.

components = 1:length(model.rules{model.start});
layers = 1;

k = 1;
for i = components
  for layer = layers
    visualizecomponent(model, i, layer, nplot, mplot, ind_plot);
    k = k+1;
  end
end

function visualizecomponent(model, c, layer, nplot, mplot, ind_plot)

rhs = model.rules{model.start}(c).rhs;
root = -1;
parts = [];
defs = {};
anchors = {};
% assume the root filter is first on the rhs of the start rules
if model.symbols(rhs(1)).type == 'T'
  % handle case where there's no deformation model for the root
  root = model.symbols(rhs(1)).filter;
else
  % handle case where there is a deformation model for the root
  root = model.symbols(model.rules{rhs(1)}(layer).rhs).filter;
end
for i = 2:length(rhs)
  defs{end+1} = model.rules{rhs(i)}(layer).def.w;
  anchors{end+1} = model.rules{model.start}(c).anchor{i};
  fi = model.symbols(model.rules{rhs(i)}(layer).rhs).filter;
  parts = [parts fi];
end
% make picture of root filter
pad = 2;
bs = 20;
w = foldHOG(model.filters(root).w);
scale = max(w(:));
im = HOGpicture(w, bs);
im = imresize(im, 2);
im = padarray(im, [pad pad], 0);
im = uint8(im * (255/scale));

% draw root
numparts = length(parts);
subplot(nplot, mplot, ind_plot);
ind_plot = ind_plot + 1;
imagesc(im)
colormap gray;
axis equal;
axis off;

% draw parts and deformation model
if numparts > 0
  def_im = zeros(size(im));
  def_scale = 500;
  for i = 1:numparts
    % part filter
    w = model.filters(parts(i)).w;
    p = HOGpicture(foldHOG(w), bs);
    p = padarray(p, [pad pad], 0);
    p = uint8(p * (255/scale));    
    % border 
    p(:,1:2*pad) = 128;
    p(:,end-2*pad+1:end) = 128;
    p(1:2*pad,:) = 128;
    p(end-2*pad+1:end,:) = 128;
    % paste into root
    x1 = (anchors{i}(1))*bs+1;
    y1 = (anchors{i}(2))*bs+1;
    x2 = x1 + size(p, 2)-1;
    y2 = y1 + size(p, 1)-1;
    im(y1:y2, x1:x2) = p;
    
    % deformation model
    probex = size(p,2)/2;
    probey = size(p,1)/2;
    for y = 2*pad+1:size(p,1)-2*pad
      for x = 2*pad+1:size(p,2)-2*pad
        px = ((probex-x)/bs);
        py = ((probey-y)/bs);
        v = [px^2; px; py^2; py];
        p(y, x) = defs{i} * v * def_scale;
      end
    end
    def_im(y1:y2, x1:x2) = p;
  end
  
  % plot parts
  subplot(nplot, mplot, ind_plot);
  ind_plot = ind_plot + 1;
  imagesc(im); 
  colormap gray;
  axis equal;
  axis off;
  
  % plot deformation model
  subplot(nplot, mplot, ind_plot);
  imagesc(def_im);
  colormap gray;
  axis equal;
  axis off;
end

set(gcf, 'Color', 'white');