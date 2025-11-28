function dE = deltaE_euclid(Lab1, Lab2)
% Fast Euclidean DeltaE (not full CIEDE2000)
    diff = Lab1 - Lab2;
    dE = sqrt(sum(diff.^2));
end
