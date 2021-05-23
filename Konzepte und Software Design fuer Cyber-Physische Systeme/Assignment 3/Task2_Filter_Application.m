%% Filter
Wp = 40/500;     %Passband Rippe
Ws = 200/500;    %Stopband Attenuation
[n,Wn] = buttord(Wp,Ws,3,40);
[b,a] = butter(n,Wn);   %Coefficient
freqz(b,a,512,1000);
title('Filter');

%%Signal
f_s = 1000;     %Sample Rate
t = 0:1/f_s:1;  %Sample the Signal in 1Sec
%Generate the sin wave
phase_sin = 0;
amplitude_sin = 1;
f_sin = 20;     %The frequency of sin wave(0,40)Hz
omega_sin = 2*pi*f_sin;
signal_sin = amplitude_sin*sin(omega_sin*t + phase_sin);

%%Noise
phase_noise = 0;
amplitude_noise = 1;
f_noise = 400;  %The frequency of noise >200Hz
omega_noise = 2*pi*f_noise; 
signal_noise = amplitude_noise*sin(omega_noise*t+phase_noise);

%%Show the Signal and Noise together
signal_combined = signal_sin + signal_noise;
figure(2);
plot(t,signal_sin);
hold on;
plot(t,signal_combined);
title('Signal and Noise');

%Filter the noise
signal_filtered = filter(b,a,signal_combined);
figure(3);
plot(t,signal_filtered);
title('The Signal after filter in Matlab');

%%Output the Data to C_Program
writematrix(signal_combined.','signal_combined.txt');
filtered_signal = readmatrix('Filtered_Signal.txt');
figure(4);
plot(t,filtered_signal.');
title('The Signal after filter in C');