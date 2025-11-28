function Lab_out = plant_mix(u, pigLAB)
% Approximate plant: linear mixing of pigment LAB + small noise
%   u      : 1x5 pigment % vector
%   pigLAB : 5x3 LAB table for pure pigments

    w = u / 100;             % 1x5 weights
    Lab_ideal = w * pigLAB;  % 1x3
    noise = [0.4 0.8 0.8] .* randn(1,3);
    Lab_out = Lab_ideal + noise;
end
