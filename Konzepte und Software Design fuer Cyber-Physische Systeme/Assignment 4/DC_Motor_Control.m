%% constant
J = 0.01;   %kgm^2,  moment of inertia
b = 0.1;    %Nms,    motor viscous friction constant
Kt= 0.01;   %Nm/A,   motor torque constant
L = 0.05;   %H,      motor inductance
R = 0.75;   %ou,     motor resistance
Ke= 0.01;   %V/rad/s electromotive force constant
Kp = 300;   %proportional control coefficient
Ki = 70;    %integral control coefficient
Kd = 10;    %derivative control coefficient

%% dynamic system
s = tf('s');
P = 20/(s^2+25*s+150.2);
pzmap(P);   %pole-zero plot
figure(2);  
step(P);    %step response

%% Proportional control
C = pid(Kp);            %create a pid controller
T = feedback(C*P,1);     %feedback connection of 2 input/output systems
t = 0:0.01:2;
figure(3);
step(T,t);
title('Proportional control');

%% Proportional-derivative control
C = pid(Kp,0,Kd);
T = feedback(C*P,1);
figure(4);
step(T,t);
title('Proportional-derivative control');

%% Proportional-integral-derivative control
C = pid(Kp,Ki,Kd);
T = feedback(C*P,1);
figure(5);
step(T,t);
title('Proportional-integral-derivative control');
