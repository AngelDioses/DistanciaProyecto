clc; % Limpia la ventana de comandos
clear; % Borra todas las variables en el espacio de trabajo
close all; % Cierra todas las figuras abiertas

% Generar datos de temperatura ficticios (por ejemplo, temperaturas diarias para un año)
dias = 365; % Número de días en el año
temperaturas = 20 + 5 * sin(2*pi*(1:dias)/365) + randn(1,dias); % Temperaturas simuladas con una tendencia sinusoidal y ruido

% Guardar los datos en un archivo .mat
save('datos_temperatura.mat', 'temperaturas'); % Guarda las temperaturas en un archivo .mat

% Cargar datos de temperatura
load datos_temperatura.mat; % Carga las temperaturas desde el archivo .mat

% Parámetros del filtro adaptativo
N = length(temperaturas); % Longitud de la señal de entrada
M = 11; % Longitud del filtro FIR
mu = 0.0001; % Tasa de aprendizaje para el algoritmo LMS (reducida aún más)
lambda = 0.95; % Factor de olvido para el algoritmo RLS
num_iteraciones = 1000; % Número de iteraciones para el algoritmo adaptativo (aumentado aún más)

% Inicialización de variables
error_medio_LMS = zeros(1,N);
error_medio_RLS = zeros(1,N);
x_i = zeros(1,M); % Inicializar el vector de entrada para el algoritmo LMS
x_r = zeros(1,M); % Inicializar el vector de entrada para el algoritmo RLS

for i = 1:num_iteraciones % Bucle para cada iteración del algoritmo adaptativo (incrementado)
    % Generar señal de entrada (datos de temperatura con ruido)
    temperatura_con_ruido = temperaturas + 0.1 * randn(size(temperaturas));

    % Inicialización de coeficientes del filtro
    W_LMS = zeros(1,M);
    W_RLS = zeros(1,M);
    P = eye(M)/lambda;

    % Aplicar algoritmo LMS
    for k = 1:N
        x_i = [temperatura_con_ruido(k) x_i(1:M-1)]; 
        y_LMS(k) = W_LMS * x_i.';
        error_LMS(k) = temperaturas(k) - y_LMS(k);
        W_LMS = W_LMS + mu * error_LMS(k) * x_i; 
    end

    % Aplicar algoritmo RLS
    for k = 1:N
        x_r = [temperatura_con_ruido(k), x_r(1:M-1)];
        Pk = P * x_r.';
        K = Pk / (lambda + x_r*Pk);
        s = temperaturas(k) - W_RLS * x_r.';
        W_RLS = W_RLS + K.' * conj(s);
        P = (1/lambda) * P - (1/lambda) * K * x_r * P;
        error_RLS(k) = s;
        y_RLS(k) = W_RLS * x_r.'; % Calcular la salida filtrada por RLS
    end

    % Calcular el error medio cuadrático para cada iteración
    error_medio_LMS = error_medio_LMS + error_LMS.^2;
    error_medio_RLS = error_medio_RLS + error_RLS.^2;
end

% Calcular el promedio de los errores cuadráticos medios
error_medio_LMS = error_medio_LMS / num_iteraciones; % Dividido por el número de iteraciones
error_medio_RLS = error_medio_RLS / num_iteraciones; % Dividido por el número de iteraciones

% Graficar resultados
figure;
subplot(3,1,1);
plot(temperaturas, 'LineWidth', 1.1);
hold on;
plot(temperatura_con_ruido, 'LineWidth', 1.1);
title('Datos de temperatura originales vs. con ruido');
legend('Original', 'Con ruido');
grid on;

subplot(3,1,2);
plot(y_LMS, 'LineWidth', 1.1);
hold on;
plot(temperaturas, 'LineWidth', 1.1);
title('Predicción LMS vs. temperatura original');
legend('Predicción LMS', 'Original');
grid on;

subplot(3,1,3);
plot(y_RLS, 'LineWidth', 1.1);
hold on;
plot(temperaturas, 'LineWidth', 1.1);
title('Predicción RLS vs. temperatura original');
legend('Predicción RLS', 'Original');
grid on;

% Graficar MSE
figure;
semilogy(error_medio_LMS, 'LineWidth', 1.5);
hold on;
semilogy(error_medio_RLS, 'LineWidth', 1.5);
legend('LMS', 'RLS');
xlabel('Iteración');
ylabel('MSE');
title('Error cuadrático medio (MSE)');
grid on;

