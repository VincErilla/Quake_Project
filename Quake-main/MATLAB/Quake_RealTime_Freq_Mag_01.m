%% Real-Time 500 Hz Monitor (Raw Time-Domain, Filtered FFT)
delete(serialportfind);
clear s;

% --- Configuration matched to your upgraded ESP32 500Hz stream ---
port = "COM13";       % Update to your laptop's specific COM port
baudrate = 115200;   % Note: If you experience data dropouts, change this to 921600 on both ESP32 and MATLAB
Fs = 500;            % New target sampling rate (500 Hz)
N = 256;             % Increased window size to keep the FFT resolution sharp at higher speeds

% Initialize Serial
s = serialport(port, baudrate);
configureTerminator(s, "LF");

% --- Bandpass Filter Setup (Restored to your original 10-175 Hz) ---
% At Fs = 500 Hz, the Nyquist limit is 250 Hz, so 175 Hz fits perfectly!
bpFilt = designfilt('bandpassiir','FilterOrder',8,...
    'HalfPowerFrequency1',10,'HalfPowerFrequency2',175,'SampleRate',Fs);

% Preallocate buffers for raw time-domain streaming
t_buf  = zeros(1, N);
z1_buf = zeros(1, N);
z2_buf = zeros(1, N);

% Map X-Axis to BPM (Max viewable: 250 Hz * 60 = 15,000 BPM)
f_Hz = (0:N/2-1)*(Fs/N);
f_BPM = f_Hz * 60;       

% --- Create 2x2 Layout ---
fig = figure('Color', 'w', 'Name', 'High-Speed Real-Time Seismic Analyzer', 'Position', [100, 100, 1100, 700]);

% Row 1: Unfiltered Raw Time Domain (Centered around your 1g baseline)
ax1 = subplot(2,2,1); hT1 = plot(t_buf, z1_buf, 'b', 'LineWidth', 1.2);
title('Sensor 1: Raw Z-Axis Time Domain'); ylabel('Acceleration (g)'); grid on;
ylim([-1.5 1.5]); % Gives some headroom for raw vibration spikes

ax2 = subplot(2,2,2); hT2 = plot(t_buf, z2_buf, 'r', 'LineWidth', 1.2);
title('Sensor 2: Raw Z-Axis Time Domain'); ylabel('Acceleration (g)'); grid on;
ylim([-1.5 1.5]);

% Row 2: Live FFT Frequency Plots (0 to 15,000 BPM)
ax3 = subplot(2,2,3); hF1 = plot(f_BPM, zeros(1, N/2), 'b', 'LineWidth', 1.2);
title('Sensor 1: Filtered FFT Spectrum (10-175 Hz)'); xlabel('Frequency (BPM)'); ylabel('Magnitude'); grid on;
xlim(ax3, [0, 12000]);

ax4 = subplot(2,2,4); hF2 = plot(f_BPM, zeros(1, N/2), 'r', 'LineWidth', 1.2);
title('Sensor 2: Filtered FFT Spectrum (10-175 Hz)'); xlabel('Frequency (BPM)'); ylabel('Magnitude'); grid on;
xlim(ax4, [0, 12000]);

linkaxes([ax1, ax2], 'x');
count = 0;
flush(s); 

while ishandle(fig)
    if s.NumBytesAvailable > 0
        dataLine = readline(s);
        values = str2double(split(dataLine, ','));
        
        if numel(values) == 5
            t   = values(1);
            z1_raw = values(4);
            z2_raw = values(5);
            
            % Shift arrays left and append raw data points
            t_buf  = [t_buf(2:end), t];
            z1_buf = [z1_buf(2:end), z1_raw];
            z2_buf = [z2_buf(2:end), z2_raw];
            
            count = count + 1;
            
            if count >= N
                % Update Raw Time Domain Lines
                set(hT1, 'XData', t_buf, 'YData', z1_buf);
                set(hT2, 'XData', t_buf, 'YData', z2_buf);
                xlim(ax1, [t_buf(1), t_buf(end)]);
                xlim(ax2, [t_buf(1), t_buf(end)]);
                
                % --- Filter Window & Calculate FFT for Sensor 1 ---
                z1_window_filtered = filter(bpFilt, z1_buf - mean(z1_buf));
                y1 = fft(z1_window_filtered); 
                P2_1 = abs(y1/N); P1_1 = P2_1(1:N/2);
                P1_1(2:end-1) = 2*P1_1(2:end-1);
                
                % --- Filter Window & Calculate FFT for Sensor 2 ---
                z2_window_filtered = filter(bpFilt, z2_buf - mean(z2_buf));
                y2 = fft(z2_window_filtered); 
                P2_2 = abs(y2/N); P1_2 = P2_2(1:N/2);
                P1_2(2:end-1) = 2*P1_2(2:end-1);
                
                % Update FFT Lines
                set(hF1, 'YData', P1_1);
                set(hF2, 'YData', P1_2);
                ylim(ax3, [0, max([max(P1_1), 0.05])]);
                ylim(ax4, [0, max([max(P1_2), 0.05])]);
            end
            
            drawnow limitrate
        end
    end
end