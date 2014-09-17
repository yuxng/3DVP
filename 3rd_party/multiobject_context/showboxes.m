function showboxes(im, boxes, new)

if nargin < 4,
  partcolor = {'g','g','y','m','m','m','m','y','y','y','r','r','r','r','y','c','c','c','c','y','y','y','b','b','b','b'};
end

if new
imagesc(im);
end
%axis image; axis off;
if ~isempty(boxes)
  
        %for i = 1:numparts
            x1 = boxes(:,1);
            y1 = boxes(:,2);
            x2 = boxes(:,3);
            y2 = boxes(:,4);
            %if i ==1 || i == 2
            %    line([x1 x1 x2 x2 x1]',[y1 y2 y2 y1 y1]','color','w','linewidth',1);
            %else
            line([x1 x1 x2 x2 x1]',[y1 y2 y2 y1 y1]','color','r','linewidth',2);
           %end
        %end
  
end
drawnow;
