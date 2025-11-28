function u = ml_predict_pigments(Lt, Lcenters, pigBase)
% Linear interpolation in L* between nearest recipes
%   Lt       : scalar target L*
%   Lcenters : 1xN vector
%   pigBase  : Nx5 matrix [Black Brown White Yellow Red]

    idx = find(Lcenters >= Lt, 1, 'first');

    if isempty(idx)
        u = pigBase(end,:);
    elseif idx == 1
        u = pigBase(1,:);
    else
        L1 = Lcenters(idx-1); L2 = Lcenters(idx);
        w  = (Lt - L1) / (L2 - L1);
        u  = (1-w)*pigBase(idx-1,:) + w*pigBase(idx,:);
    end
end
