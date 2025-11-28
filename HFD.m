clear; clc; close all;

rng(1);  % reproducible

% ===== Pigment base recipes vs L* =====
Lcenters = [85 75 65 50 35]; % very fair → dark

% [Black Brown White Yellow Red] in %
pigBase = [ ...
     0   5 80 12  3;  % very fair
     2  15 60 18  5;  % fair
     5  25 50 15  5;  % medium
    10  35 40 10  5;  % brown
    25  45 20  7  3]; % dark

% LAB of pure pigments  (fake but reasonable)
pigLAB = [ ...
    20   0   0;   % Black
    40  15  20;   % Brown
    95   0   0;   % White
    90  -5  60;   % Yellow
    50  60  30];  % Red

% Targets: [L* a* b*]
targets = [ ...
    82   3  16;   % fair
    68  10  22;   % medium
    42  12  18];  % brown/dark

nIters = 6;           % max closed‑loop iterations
deHistoryAll = {};    % store ΔE per trial
uHistoryAll  = {};    % store pigment vectors

for t = 1:size(targets,1)
    Lab_t = targets(t,:);      % target LAB
    fprintf('\n=== TRIAL %d, Target LAB = [%.1f %.1f %.1f] ===\n', ...
            t, Lab_t);

    % ----- ML feedforward prediction -----
    u0 = ml_predict_pigments(Lab_t(1), Lcenters, pigBase); % 1x5
    fprintf('Initial ML pigments (%%): '); disp(round(u0,2));

    u  = u0;
    Iacc = 0;                 % integral term (lightness)
    kP = 0.6; kI = 0.2;

    deHist = zeros(1,nIters);
    uHist  = zeros(nIters,5);

    for k = 1:nIters
        % ----- Plant: pumps + mixer + pigment→colour -----
        Lab_out = plant_mix(u, pigLAB);       % 1x3 LAB
        dE = deltaE_euclid(Lab_t, Lab_out);   % scalar

        deHist(k) = dE;
        uHist(k,:) = u;

        fprintf(' Iter %d: ΔE = %.3f, u = [', k, dE);
        fprintf('%.1f ', u);
        fprintf(']\n');

        if dE < 2
            fprintf('  --> Good match, stopping.\n');
            deHist = deHist(1:k);
            uHist  = uHist(1:k,:);
            break;
        end

        % ----- Error controller (acts mainly on L*) -----
        eL = Lab_t(1) - Lab_out(1);
        Iacc = Iacc + eL;
        ctrl = kP*eL + kI*Iacc;

        duBlack = +0.4*ctrl;
        duWhite = -0.4*ctrl;

        u(1) = u(1) + duBlack;
        u(3) = u(3) + duWhite;

        % clip & renormalize
        u(u < 0) = 0;
        u = u / sum(u) * 100;
    end

    deHistoryAll{t} = deHist;
    uHistoryAll{t}  = uHist;
end

% ====== PLOTS ======

figure;
hold on; grid on;
colors = lines(size(targets,1));
for t = 1:numel(deHistoryAll)
    plot(1:numel(deHistoryAll{t}), deHistoryAll{t}, ...
         '-o', 'Color', colors(t,:), 'LineWidth', 1.5);
end
xlabel('Iteration');
ylabel('\DeltaE (approx)');
title('\DeltaE vs Iteration for Different Skin Targets');
legend({'Fair','Medium','Dark'}, 'Location','northeast');

% Pigment evolution for last trial
last = numel(uHistoryAll);
uHist = uHistoryAll{last};

figure;
plot(1:size(uHist,1), uHist, '-o','LineWidth',1.5);
grid on;
xlabel('Iteration');
ylabel('Pigment percentage (%)');
title('Pigment Ratios Evolution (Last Trial)');
legend({'Black','Brown','White','Yellow','Red'}, 'Location','best');
