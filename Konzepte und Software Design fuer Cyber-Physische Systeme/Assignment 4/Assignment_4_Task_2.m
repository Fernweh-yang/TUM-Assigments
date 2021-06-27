%% Konzepte und Software Design für Cyber-Physische Systeme
%
% Assignment 4: Basics of control theory and PID controller design
% Group: Lim, Seokkyun; Mijacevic, Matej; Xu, Yang
%
%% Plot log-file
csv = csvread('flight.csv',1,0);
time = csv(:,1);
height = csv(:,2);
plot(time, height, 'LineWidth', 2.0)
yline(1, ':', 'LineWidth', 1.5)