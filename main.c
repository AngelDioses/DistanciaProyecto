/*
 * File:   main.c
 * Author: angel
 *
 * Created on 20 de febrero de 2024, 7:57 p.m.
 */


#include "xc.h"
#include <libpic30.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Para el FPOR
#pragma config PWMPIN = ON
#pragma config HPOL = ON
#pragma config LPOL = ON
#pragma config ALTI2C = OFF
#pragma config FPWRT = PWR128

// Para el FWDT
#pragma config WDTPOST = PS32768
#pragma config WDTPRE = PR128
#pragma config WINDIS = OFF
#pragma config FWDTEN = OFF

// Para el FOSC
#pragma config POSCMD = HS
#pragma config OSCIOFNC = OFF
#pragma config IOL1WAY = ON
#pragma config FCKSM = CSDCMD

// Para el FOSCSEL
#pragma config FNOSC = PRIPLL
#pragma config IESO = OFF

#define Speed 40 // MIPS deseados (maximo 40 para el dspic33fj32mc204)
#define Crystal_Externo 20 //Valor del crystal en MHZ
#define Freq Speed*1000000
#define delay_ms(x) __delay32((Freq/1000)*x) // Delay en milisegundos
#define delay_us(x) __delay32(Speed*x) // Delay en microsegundos

#define BAUDRATE 9600 
#define BRGVAL ((Freq/BAUDRATE)/16)-1
#define VOLTAJE 5 //Valor del voltaje de entrada

void adc(void);
void Reloj_PLL(void);
void  Configurar_UART();

float mV1;
float Cels1;

int main(void) {
    AD1PCFGL = 0xFFFF; //Todos son digitales
    Reloj_PLL();
    Configurar_UART(); // Llama funcion UART
    adc(); // Llama funcion ADC
    while (1) {
        AD1CON1bits.SAMP = 1; // Start sampling
        delay_us(10); // Wait for sampling time (10us)
        AD1CON1bits.SAMP = 0; // Start the conversion
        while (!AD1CON1bits.DONE);
        mV1 = (float) ((ADC1BUF0 * VOLTAJE) / (1023.0)) * 1000; // Convierte el valor leido a mV
        Cels1 = mV1 / 10; // Convierte los mV leidos a grados Celsius
        char buffer[32]; // Asegúrate de que el buffer sea lo suficientemente grande para la cadena resultante

        // Convertir el valor de Cels1 a string y almacenarlo en buffer
        sprintf(buffer, "%.3f°C\r\n", (double) Cels1);

        // Enviar la cadena a través de UART
        Serial_SendString(buffer);
        delay_ms(1);
    }
    return 0;
}

void Reloj_PLL(void) {
    // Con oscilador externo    
    int M = Speed * 8 / Crystal_Externo;
    PLLFBD = M - 2; // M = 28;
    CLKDIVbits.PLLPRE = 0; // N1 = 2;
    CLKDIVbits.PLLPOST = 0; // N2 = 2; 
    while (_LOCK == 0);
}

void Configurar_UART() {
    RPINR18 = 0x0000; // RP0 como U1RX
    RPOR0 = 0x0300; // RP1 como U1TX

    U1BRG = BRGVAL;
    U1MODE = 0; // 8 bits, sin paridad, 1 bit de parada
    U1STA = 0; // Reset estados
    U1MODEbits.UARTEN = 1; // Habilita UART
    U1STAbits.UTXEN = 1; // Habilita transmisión
    IFS0bits.U1RXIF = 0; // Limpia flag RX
    IEC0bits.U1RXIE = 1; // Habilita interrupción RX
}

void Serial_SendString(char *str) {
    char *p;
    p = str;
    while (*p) Serial_PutChar(*p++);
}

void Serial_PutChar(char Ch) {
    while (U1STAbits.UTXBF); // Espera buffer vacío
    U1TXREG = Ch;
}


// CONFIGURACIÓN DEL ADC

void adc() {
    // 0 = 10, l = 12 bits resolucion y muestreo secuencial de los canales 0 = 10 bits ADC
    AD1CON1bits.AD12B = 0;

    // El muestreo inicia cuando bit SAMP = 1
    AD1CON1bits.ASAM = 0;

    // Termina el muestreo e inicia la conversion
    AD1CON1bits.SSRC = 0;

    // Selecciona como se presentan los resultados de la conversion 
    // En el buffer AD1CON1 <9:8>, LA SALIDA SERA DE TIPO INT, 3 ES FRACCION
    AD1CON1bits.FORM = 0;

    // La entrada negativa sera VSS
    AD1CHS0bits.CH0NA = 0;
    //AD1CHS0bits.CH0SA = 0 //Entrada positiva sera VSS

    // Muestreo secuencia (scan) habilitado
    AD1CON2bits.CSCNA = 1;

    // Referencia para la conversión Referencia positiva = AVdd, negativa = AVss
    AD1CON2bits.VCFG = 0;

    // Configurado como buffer de 16 palabras
    AD1CON2bits.BUFM = 0;

    // Solo muestra un canal
    AD1CON2bits.ALTS = 0;

    // Reloj del ADC es derivdo del sistema
    AD1CON3bits.ADRC = 0;

    // Tiempo de conversión
    AD1CON3bits.ADCS = 21;

    AD1CON2bits.SMPI = 0;

    // AD1CSSH/AD1CSSL: A/D Imput Scan Selection Register
    AD1CSSL = 0x0000;
    AD1CSSLbits.CSS0 = 1; // Enable AN0 for channel scan
    AD1PCFGLbits.PCFG0 = 0; // AN0 as Analog Input     
    AD1CON1bits.ADON = 1; // Habilita ADC
}
