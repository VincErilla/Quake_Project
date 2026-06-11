% MATLAB Real-Time Monitor - Filtered Magnitude (10-175Hz) & Raw Z-Axis
clear; clc;
delete(serialportfind); % Clear any stuck serial handles

% --- Configuration ---
port = "COM8";            % <-- Your verified working port
baudrate = 115200;
Fs = 166.66;              % Data stream frequency (100 Hz)
viewWindowSeconds = 5; 
numSamples = round(Fs * viewWindowSeconds); % 2500 samples

% Default limits
defaultZRange   = 0.5;  % View window size around the 1.0g baseline (+/-0.5g)
defaultMagMax   = 1.5;  % Upper boundary for the filtered magnitude

% --- RESTORED: Big Boss Bandpass Filter Setup (10-175 Hz, 8th Order) ---
bpFilt = designfilt('bandpassiir','FilterOrder',8,...
    'HalfPowerFrequency1',10,'HalfPowerFrequency2',75,'SampleRate',Fs);

% --- Pre-allocate Fixed-Size Graphics Buffers ---
t_buf    = zeros(1, numSamples);
mag1_buf = zeros(1, numSamples); z1_buf = zeros(1, numSamples);
mag2_buf = zeros(1, numSamples); z2_buf = zeros(1, numSamples);

% --- Setup Serial Port ---
if ~isempty(serialportlist("available"))
    device = serialport(port, baudrate);
    configureTerminator(device, "LF");
    flush(device);
else
    error("The specified serial port (%s) is not available.", port);
end

% --- Setup Live 2x2 Plot Figure Layout ---
fig = figure('Name', 'Hybrid 10-175Hz Magnitude & Raw Z-Axis Monitor', 'NumberTitle', 'off', 'Position', [100, 100, 1100, 750], 'Color', 'w');
ax = zeros(2,2); hPlots = cell(2,2);
colors = {[0.85, 0.33, 0.1], [0, 0.45, 0.74]}; 

for row = 1:2
    for col = 1:2
        ax(row, col) = subplot(2, 2, (row-1)*2 + col);
        hPlots{row, col} = plot(t_buf, zeros(1, numSamples), 'Color', colors{col}, 'LineWidth', 1.3);
        grid on;
        if row == 1
            title(sprintf('Sensor %d: 3D Filtered Magnitude (10-175 Hz)', col));
            ylim(ax(row, col), [0, defaultMagMax]);
        else
            title(sprintf('Sensor %d: Z-Axis RAW Amplitude (1.0g Baseline)', col));
            ylim(ax(row, col), [1.0 - defaultZRange, 1.0 + defaultZRange]); 
            xlabel('Time (s)');
        end
        ylabel('Acceleration (g)');
    end
end
linkaxes(ax(:), 'x');
flush(device); 

disp('Streaming... Row 1 is 8th-order Filtered | Row 2 is RAW data. Press Ctrl+C to stop.');
cleanUpObj = onCleanup(@() clear('device')); 

renderCounter = 0;
renderSkipMax = 10; % Refresh screen at 50 FPS

% Create persistent tracking buffers for historical values to let filter step correctly
x1_raw_hist = zeros(1, 20); y1_raw_hist = zeros(1, 20); z1_raw_hist = zeros(1, 20);
x2_raw_hist = zeros(1, 20); y2_raw_hist = zeros(1, 20); z2_raw_hist = zeros(1, 20);

while ishandle(fig)
    if device.NumBytesAvailable > 0
        try
            dataLine = readline(device);
            vals = str2double(split(dataLine, ','));
            
            if numel(vals) == 7
                t  = vals(1);
                z1_raw = vals(4);
                z2_raw = vals(7);
                
                % Append newest sample to the mini-tracking arrays
                x1_raw_hist = [x1_raw_hist(2:end), vals(2)];
                y1_raw_hist = [y1_raw_hist(2:end), vals(3)];
                z1_raw_hist = [z1_raw_hist(2:end), z1_raw];
                
                x2_raw_hist = [x2_raw_hist(2:end), vals(5)];
                y2_raw_hist = [y2_raw_hist(2:end), vals(6)];
                z2_raw_hist = [z2_raw_hist(2:end), z2_raw];
                
                % --- Parallel Path 1: Filter using the native digitalFilter object directly ---
                % This structure lets MATLAB use its native backend optimizations to update states
                xf1_arr = filter(bpFilt, x1_raw_hist); xf1 = xf1_arr(end);
                yf1_arr = filter(bpFilt, y1_raw_hist); yf1 = yf1_arr(end);
                zf1_arr = filter(bpFilt, z1_raw_hist); zf1 = zf1_arr(end);
                
                xf2_arr = filter(bpFilt, x2_raw_hist); xf2 = xf2_arr(end);
                yf2_arr = filter(bpFilt, y2_raw_hist); yf2 = yf2_arr(end);
                zf2_arr = filter(bpFilt, z2_raw_hist); zf2 = zf2_arr(end);
                
                % Calculate Magnitude from clean dynamic signals
                mag1_dyn = sqrt(xf1^2 + yf1^2 + zf1^2);
                mag2_dyn = sqrt(xf2^2 + yf2^2 + zf2^2);
                
                % --- Push into fixed rolling windows (Routing RAW Z here) ---
                t_buf    = [t_buf(2:end), t];
                mag1_buf = [mag1_buf(2:end), mag1_dyn]; z1_buf = [z1_buf(2:end), z1_raw]; 
                mag2_buf = [mag2_buf(2:end), mag2_dyn]; z2_buf = [z2_buf(2:end), z2_raw]; 
                
                % --- Framerate Throttle Render ---
                renderCounter = renderCounter + 1;
                if renderCounter >= renderSkipMax
                    set(hPlots{1,1}, 'XData', t_buf, 'YData', mag1_buf);
                    set(hPlots{1,2}, 'XData', t_buf, 'YData', mag2_buf);
                    set(hPlots{2,1}, 'XData', t_buf, 'YData', z1_buf);
                    set(hPlots{2,2}, 'XData', t_buf, 'YData', z2_buf);
                    
                    % Auto-Scaling for Filtered Magnitudes (Row 1)
                    maxM = max([mag1_buf, mag2_buf]);
                    if maxM > defaultMagMax
                        ylim(ax(1,1), [0, maxM * 1.1]); ylim(ax(1,2), [0, maxM * 1.1]);
                    else
                        ylim(ax(1,1), [0, defaultMagMax]); ylim(ax(1,2), [0, defaultMagMax]);
                    end
                    
                    % Auto-Scaling for Raw Z-Axes (Row 2)
                    maxZ = max([abs(z1_buf - 1.0), abs(z2_buf - 1.0)]);
                    if maxZ > defaultZRange
                        ylim(ax(2,1), [1.0 - maxZ*1.1, 1.0 + maxZ*1.1]); 
                        ylim(ax(2,2), [1.0 - maxZ*1.1, 1.0 + maxZ*1.1]);
                    else
                        ylim(ax(2,1), [1.0 - defaultZRange, 1.0 + defaultZRange]); 
                        ylim(ax(2,2), [1.0 - defaultZRange, 1.0 + defaultZRange]);
                    end
                    
                    xlim(ax(1,1), [t_buf(1), t_buf(end)]);
                    drawnow;
                    renderCounter = 0; 
                end
            end
        catch
            continue; 
        end
    end
end