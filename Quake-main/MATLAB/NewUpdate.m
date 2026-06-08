close all;
clc;
clear;

%% ----------------------------
% User settings
% ----------------------------
t_initial     = 0;          % start time (s) relative to file
t_end         = inf;          % duration after t_initial to include (s); inf = no trim
viewWindow    = [];           % optional plot window [tmin tmax] after re-zero; [] = show all
metricsWindow = [0 1];        % [t_start t_end] for metrics; [] = full trimmed
rmsWindowSec  = 0.5;          % RMS window length in seconds
amp_ylim      = [-10 10];     % y-limits for amplitude plot
rms_ylim      = [0 8];        % y-limits for RMS plot
spec_fmax     = 200;          % spectrogram display cap (Hz)

%% ----------------------------
% Load data
% ----------------------------
data = readtable('quakeData_84.csv');  % Adjust filename
time_full = data{:,1};

% Select time range
start_idx = find(time_full >= t_initial, 1, 'first');
if isempty(start_idx), error('t_initial beyond data range.'); end

if isfinite(t_end)
    t_end_abs = t_initial + t_end;
    end_idx = find(time_full <= t_end_abs, 1, 'last');
    if isempty(end_idx) || end_idx < start_idx
        error('t_end leaves no data.');
    end
else
    end_idx = numel(time_full);
end

idx  = start_idx:end_idx;
time = time_full(idx) - t_initial;   % re-zeroed time

%% ----------------------------
% Sensor data
% ----------------------------
X2_raw = data{idx,2};
Y2_raw = data{idx,3};
Z2_raw = data{idx,4};         % 4 = sensor1, 7 = sensor2
fs = 1 / mean(diff(time));    % sampling frequency (Hz)

fprintf('Sampling Frequency = %.2f Hz\n', fs);
fprintf('Sampling Period = %.6f s\n', 1/fs);

% Bandpass Filter 10–175 Hz
bpFilt = designfilt('bandpassiir', 'FilterOrder', 8, ...
    'HalfPowerFrequency1', 10, 'HalfPowerFrequency2', 175, 'SampleRate', fs);
X2 = filtfilt(bpFilt, X2_raw);
Y2 = filtfilt(bpFilt, Y2_raw);
Z2 = filtfilt(bpFilt, Z2_raw);

% RMS
winRMS_samp = max(1, round(rmsWindowSec*fs));
RMS_Z2 = sqrt(movmean(Z2.^2, winRMS_samp));

%% ----------------------------
% Metrics window
% ----------------------------
if isempty(metricsWindow)
    t_metrics_min = time(1);
    t_metrics_max = time(end);
else
    t_metrics_min = max(min(metricsWindow), time(1));
    t_metrics_max = min(max(metricsWindow), time(end));
end
mask_metrics = (time >= t_metrics_min) & (time <= t_metrics_max) & ~isnan(RMS_Z2);

% RMS metrics
RMS_win = RMS_Z2(mask_metrics);
time_win = time(mask_metrics);
avg_g_RMS = mean(RMS_win, 'omitnan');
[max_g, idx_max_local] = max(RMS_win);
t_max_g = time_win(idx_max_local);

%% ----------------------------
% Spectrogram (Hz -> BPM)
% ----------------------------
winSpec = 256; overlap = 200; nfft = 256;
[S,F,T,P] = spectrogram(Z2, winSpec, overlap, nfft, fs, 'power');
PdB = 10*log10(P + eps);
F_BPM = F*60;

%% ----------------------------
% FFT Analysis
% ----------------------------
N = length(Z2);

% Remove DC offset
Z2_fft = Z2 - mean(Z2);

% FFT
Y = fft(Z2_fft);

% Frequency axis
f = (0:N-1)*(fs/N);

% Magnitude spectrum
P2 = abs(Y/N);
P1 = P2(1:floor(N/2)+1);

% Double amplitudes except DC/Nyquist
P1(2:end-1) = 2*P1(2:end-1);

% One-sided frequency axis
f1 = f(1:floor(N/2)+1);

% Convert to BPM
f1_BPM = f1 * 60;

% Find dominant FFT frequency
[peakAmp, peakIdx] = max(P1);
dominantFreq_Hz = f1(peakIdx);
dominantFreq_BPM = dominantFreq_Hz * 60;

% BPM metrics in window
mask_spec = (T >= t_metrics_min) & (T <= t_metrics_max);
P_win = P(:,mask_spec);
[~, idx_peak] = max(P_win, [], 1);
F_peak = F_BPM(idx_peak); 
max_BPM = max(F_peak);
avg_BPM = mean(F_peak);

%% ----------------------------
% Figure with 3 linked panels
% ----------------------------
fig = figure('Name', 'Sensor 2 — Amplitude, RMS, Spectrogram', 'Position', [100 80 1100 950]);
tiledlayout(fig, 3, 1, 'Padding', 'compact', 'TileSpacing', 'compact');

% 1) Amplitude
ax1 = nexttile;
plot(time, Z2, 'b'); hold on; grid on;
title('Z-axis amplitude'); xlabel('Time (s)'); ylabel('Amplitude');
if ~isempty(amp_ylim), ylim(ax1, amp_ylim); end

% 2) RMS
ax2 = nexttile;
plot(time, RMS_Z2, 'b'); hold on; grid on;
plot([t_metrics_min t_metrics_max], [avg_g_RMS avg_g_RMS], '--r', 'LineWidth', 1.6);
plot(t_max_g, max_g, 'ro', 'MarkerSize', 6, 'LineWidth', 1.5);
text(t_metrics_min + 0.02*(t_metrics_max - t_metrics_min), avg_g_RMS + 0.02*diff(ylim), ...
    'Avg = ' + string(avg_g_RMS), 'Color', 'r', 'FontWeight', 'bold');
title('RMS (Z-axis)'); xlabel('Time (s)'); ylabel('g (RMS)');
if ~isempty(rms_ylim), ylim(ax2, rms_ylim); end
legend('RMS(Z2)', 'Avg g (window)', 'Peak g', 'Location', 'best');

% 3) Spectrogram
ax3 = nexttile;
imagesc(T, F_BPM, PdB); axis xy; grid on;
ylim([0 min(spec_fmax*60, fs/2*60)]);
title('Spectrogram (Z-axis)'); xlabel('Time (s)'); ylabel('Frequency (BPM)');
cbar = colorbar; cbar.Label.String = 'Power (dB)';
hold on; plot(T(mask_spec), F_peak, 'r', 'LineWidth', 1.2);

linkaxes([ax1, ax2, ax3], 'x');
if ~isempty(viewWindow) && numel(viewWindow) == 2, xlim(viewWindow); end
legend(ax3, 'Peak freq (BPM)', 'Location', 'best');

%% ----------------------------
% Zoomed metrics window figure (RMS + BPM)
% ----------------------------
fig_win = figure('Name', 'Metrics Window Zoom', 'Position', [150 150 900 400]);
plot(time_win, RMS_win, 'b', 'LineWidth', 1.5); hold on; grid on;
plot([time_win(1) time_win(end)], [avg_g_RMS avg_g_RMS], '--r', 'LineWidth', 1.6);
plot(t_max_g, max_g, 'ro', 'MarkerSize', 8, 'LineWidth', 1.5);
text(time_win(1), max(RMS_win) + 0.05*diff([min(RMS_win) max(RMS_win)]), ...
    'Avg RMS = ' + string(avg_g_RMS), 'Color', 'r', 'FontWeight', 'bold');
text(t_max_g, max(RMS_win) + 0.05*diff([min(RMS_win) max(RMS_win)]), ...
    'Max RMS = ' + string(max_g), 'Color', 'r', 'FontWeight', 'bold', 'HorizontalAlignment', 'center');
text(time_win(1) + 0.6*(time_win(end) - time_win(1)), max(RMS_win) + 0.05*diff([min(RMS_win) max(RMS_win)]), ...
    sprintf('Max BPM = %.2f\nAvg BPM = %.2f', max_BPM, avg_BPM), 'Color', 'm', 'FontWeight', 'bold', 'HorizontalAlignment', 'left');
xlabel('Time (s)'); ylabel('g (RMS)');
title('RMS (Z-axis) — Metrics Window');
ylim([min(RMS_win) max(RMS_win)*1.15]);
legend('RMS(Z2)', 'Avg g (window)', 'Peak g', 'Location', 'best');

%% ----------------------------
% FFT Figure
% ----------------------------
fig_fft = figure('Name', 'FFT Spectrum', 'Position', [200 200 1000 450]);
plot(f1_BPM, P1, 'b', 'LineWidth', 1.2);
grid on; hold on;

% Mark dominant frequency
plot(dominantFreq_BPM, peakAmp, 'ro', 'MarkerSize', 8, 'LineWidth', 2);
xlabel('Frequency (BPM)'); ylabel('Amplitude');
title('FFT Spectrum (Z-axis)');
xlim([0 spec_fmax*60]);

text(dominantFreq_BPM, peakAmp, ...
    sprintf('  Peak = %.2f BPM (%.2f Hz)', dominantFreq_BPM, dominantFreq_Hz), ...
    'Color', 'r', 'FontWeight', 'bold');

%% ----------------------------
% Console reports
% ----------------------------
fprintf('--- Metrics (re-zeroed time) ---\n');
fprintf('Window: [%.3f, %.3f] s\n', t_metrics_min, t_metrics_max);
fprintf('Average g (RMS) = %.6f g\n', avg_g_RMS);
fprintf('Max g (RMS) = %.6f g at t = %.6f s\n', max_g, t_max_g);
fprintf('--- BPM Metrics ---\n');
fprintf('Highest BPM = %.2f BPM\n', max_BPM);
fprintf('Average BPM = %.2f BPM\n', avg_BPM);