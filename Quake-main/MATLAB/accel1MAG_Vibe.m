% MATLAB Real-Time Monitor - Filtered Magnitude (10-175Hz) & Raw Z-Axis (Single Sensor)
clear; clc;
delete(serialportfind); % Clear any stuck serial handles

% --- Configuration ---
port = "COM13";        % <-- Your verified working port
baudrate = 115200;
Fs = 250;              % Data stream frequency (~250-500 Hz depending on throttle)
viewWindowSeconds = 5; 
numSamples = round(Fs * viewWindowSeconds); 

% Default limits
defaultZRange   = 0.5;  % View window size around the 1.0g baseline (+/-0.5g)
defaultMagMax   = 1.5;  % Upper boundary for the filtered magnitude

% --- Bandpass Filter Setup (10-75 Hz, 8th Order) ---
bpFilt = designfilt('bandpassiir','FilterOrder',8,...
    'HalfPowerFrequency1',10,'HalfPowerFrequency2',75,'SampleRate',Fs);

% --- Pre-allocate Fixed-Size Graphics Buffers ---
t_buf    = zeros(1, numSamples);
mag1_buf = zeros(1, numSamples); 
z1_buf   = zeros(1, numSamples);

% --- Setup Serial Port ---
if ~isempty(serialportlist("available"))
    device = serialport(port, baudrate);
    configureTerminator(device, "LF");
    flush(device);
else
    error("The specified serial port (%s) is not available.", port);
end

% --- Setup Live 2x1 Plot Figure Layout ---
fig = figure('Name', 'Single Sensor Magnitude & Raw Z-Axis Monitor', ...
             'NumberTitle', 'off', 'Position', [100, 100, 800, 600], 'Color', 'w');
ax = zeros(2,1); 
hPlots = cell(2,1);
sensorColor = [0.85, 0.33, 0.1]; % Warm orange/red

% Top Subplot: Filtered Magnitude
ax(1) = subplot(2, 1, 1);
hPlots{1} = plot(t_buf, zeros(1, numSamples), 'Color', sensorColor, 'LineWidth', 1.3);
grid on;
title('Sensor 1: 3D Filtered Magnitude (10-75 Hz)');
ylim(ax(1), [0, defaultMagMax]);
ylabel('Acceleration (g)');

% Bottom Subplot: Raw Z Axis
ax(2) = subplot(2, 1, 2);
hPlots{2} = plot(t_buf, zeros(1, numSamples), 'Color', sensorColor, 'LineWidth', 1.3);
grid on;
title('Sensor 1: Z-Axis RAW Amplitude (1.0g Baseline)');
ylim(ax(2), [1.0 - defaultZRange, 1.0 + defaultZRange]); 
xlabel('Time (s)');
ylabel('Acceleration (g)');

linkaxes(ax, 'x');
flush(device); 

disp('Streaming Single Sensor... Top row: Filtered | Bottom row: RAW Z. Press Ctrl+C to stop.');
cleanUpObj = onCleanup(@() clear('device')); 

renderCounter = 0;
renderSkipMax = 10; % Framerate throttle 

% History tracking for the digital filter backend state consistency
x1_raw_hist = zeros(1, 20); 
y1_raw_hist = zeros(1, 20); 
z1_raw_hist = zeros(1, 20);

while ishandle(fig)
    if device.NumBytesAvailable > 0
        try
            dataLine = readline(device);
            vals = str2double(split(dataLine, ','));
            
            % FIXED CRITICAL GUARD: Now expects exactly 4 values (t, x, y, z)
            if numel(vals) == 4
                t      = vals(1);
                z1_raw = vals(4);
                
                % Append newest sample to historical mini-tracking arrays
                x1_raw_hist = [x1_raw_hist(2:end), vals(2)];
                y1_raw_hist = [y1_raw_hist(2:end), vals(3)];
                z1_raw_hist = [z1_raw_hist(2:end), z1_raw];
                
                % Run data streams through the native backend filter backends
                xf1_arr = filter(bpFilt, x1_raw_hist); xf1 = xf1_arr(end);
                yf1_arr = filter(bpFilt, y1_raw_hist); yf1 = yf1_arr(end);
                zf1_arr = filter(bpFilt, z1_raw_hist); zf1 = zf1_arr(end);
                
                % Calculate AC-coupled magnitude profile
                mag1_dyn = sqrt(xf1^2 + yf1^2 + zf1^2);
                
                % Push down into fixed size plot rolling queues
                t_buf    = [t_buf(2:end), t];
                mag1_buf = [mag1_buf(2:end), mag1_dyn]; 
                z1_buf   = [z1_buf(2:end), z1_raw]; 
                
                % --- Framerate Throttle Display Update ---
                renderCounter = renderCounter + 1;
                if renderCounter >= renderSkipMax
                    set(hPlots{1}, 'XData', t_buf, 'YData', mag1_buf);
                    set(hPlots{2}, 'XData', t_buf, 'YData', z1_buf);
                    
                    % Auto-Scaling Dynamic Rules for Filtered Magnitudes
                    maxM = max(mag1_buf);
                    if maxM > defaultMagMax
                        ylim(ax(1), [0, maxM * 1.1]);
                    else
                        ylim(ax(1), [0, defaultMagMax]);
                    end
                    
                    % Auto-Scaling Dynamic Rules for Raw Z-Axes 
                    maxZ = max(abs(z1_buf - 1.0));
                    if maxZ > defaultZRange
                        ylim(ax(2), [1.0 - maxZ*1.1, 1.0 + maxZ*1.1]); 
                    else
                        ylim(ax(2), [1.0 - defaultZRange, 1.0 + defaultZRange]); 
                    end
                    
                    xlim(ax(1), [t_buf(1), t_buf(end)]);
                    drawnow;
                    renderCounter = 0; 
                end
            end
        catch
            continue; 
        end
    end
end