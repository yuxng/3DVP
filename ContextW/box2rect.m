function rect = box2rect(box)
rect(:, 1) = box(:, 1);
rect(:, 2) = box(:, 2);
rect(:, 3) = (box(:, 3) - box(:, 1));
rect(:, 4) = (box(:, 4) - box(:, 2));
end