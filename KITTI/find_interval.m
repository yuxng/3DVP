function ind = find_interval(azimuth, num)

a = (360/(num*2)):(360/num):360-(360/(num*2));

for i = 1:numel(a)
    if azimuth < a(i)
        break;
    end
end
ind = i;
if azimuth > a(end)
    ind = 1;
end