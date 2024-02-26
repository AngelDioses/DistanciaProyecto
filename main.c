

// DSPIC33FJ32MC204 Configuration Bit Settings

// 'C' source line config statements

// FBS
#pragma config BWRP = WRPROTECT_OFF     // Boot Segment Write Protect (Boot Segment may be written)
#pragma config BSS = NO_FLASH           // Boot Segment Program Flash Code Protection (No Boot program Flash segment)

// FGS
#pragma config GWRP = OFF               // General Code Segment Write Protect (User program memory is not write-protected)
#pragma config GSS = OFF                // General Segment Code Protection (User program memory is not code-protected)

// FOSCSEL
#pragma config FNOSC = PRIPLL           // Oscillator Mode (Primary Oscillator (XT, HS, EC) w/ PLL)
#pragma config IESO = OFF               // Internal External Switch Over Mode (Start-up device with user-selected oscillator source)

// FOSC
#pragma config POSCMD = XT              // Primary Oscillator Source (XT Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function (OSC2 pin has clock out function)
#pragma config IOL1WAY = OFF            // Peripheral Pin Select Configuration (Allow Multiple Re-configurations)
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Both Clock Switching and Fail-Safe Clock Monitor are disabled)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler (1:32,768)
#pragma config WDTPRE = PR128           // WDT Prescaler (1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog timer enabled/disabled by user software)

// FPOR
#pragma config FPWRT = PWR128           // POR Timer Value (128ms)
#pragma config ALTI2C = OFF             // Alternate I2C  pins (I2C mapped to SDA1/SCL1 pins)
#pragma config LPOL = ON                // Motor Control PWM Low Side Polarity bit (PWM module low side output pins have active-high output polarity)
#pragma config HPOL = ON                // Motor Control PWM High Side Polarity bit (PWM module high side output pins have active-high output polarity)
#pragma config PWMPIN = ON              // Motor Control PWM Module Pin Mode bit (PWM module pins controlled by PORT register at device Reset)

// FICD
#pragma config ICS = PGD1               // Comm Channel Select (Communicate on PGC1/EMUC1 and PGD1/EMUD1)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG is Disabled)

#pragma config FOSC = INTOSCIO // Oscilador interno
#pragma config WDTEN = OFF     // Perro guardián deshabilitado
#pragma config FCMEN = OFF     // Monitor de fallas deshabilitado
#pragma config MCLRE = ON      // Reset maestro habilitado



// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define BAUDRATE 9600        // Velocidad de transmisión en baudios

#include <xc.h>
#define    FCY    80000000UL // frecuencia del reloj
#define led1  _LATB14  //timer 1 de 16 bits
#define led2  _LATB15   // timers 32 32 bits
#define disparo  _LATA1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "libpic30.h"

#include<uart.h>
#include "pps.h"// incluye libreria de pines remapeables
#include  <timer.h>
#include <delay.h>



// registro para configurar el puerto serial
unsigned int config1=0,config2=0;

unsigned int  pwm_in1,pwm_in2,pwm_in3;
float ancho1,ancho2,ancho3;
float dis1,dis2,dis3;

char texto[50];
#define prescalador 64  // prescalador a usar



void Uart1_write_text_const(const char *info)
{
 while(*info)
 {
     WriteUART1(*info++);
     while (_UTXBF==1);//espera que se envíe el dato
 }
}

void Uart1_write_text( char *info)
{
 while(*info)
    {
     WriteUART1(*info++);
    while (_UTXBF==1);//espera que se envíe el dato
    }
}



void habilita()
{
 //nivel de prioridad interrupcion CPU  //0 habilita interrupciones
_IPL3=0;
INTCON1bits.NSTDIS=0; //PARA PODER HACER EL CAMBIO DEBE STAR EN CERO HABILITADO EL ANIDAMIENTO
_IPL2=0;
_IPL1=0;
_IPL0=0;
_NSTDIS=0; //HABILITA ANINAMIENTO INTERRUPCIONES
}

void __attribute__((__interrupt__,__auto_psv__)) _T1Interrupt(void)
{
        pwm_in1=TMR1;
        TMR1=0;
       _T1IF=0;//  LIMPIA BANDERA
}

void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void)
{
        pwm_in2=TMR2;
        TMR2=0;
       _T2IF=0;//  LIMPIA BANDERA
}

void __attribute__((__interrupt__,__auto_psv__)) _T3Interrupt(void)
{
        pwm_in3=TMR3;
        TMR3=0;
       _T3IF=0;//  LIMPIA BANDERA
}


int main(void)
{
   //Configuración de operacion.
//CONFIGURA VELOCIDAD DE OPERACION    A 80 MEGAHERT CON CRISTAL DE 4 MEGA

//DEJAMOS N1 Y N2  EN 2

//VELOCIDAD= 4M*M/(N1*N2)= 4M*80/4=80M
  CLKDIVbits.PLLPRE = 0;      // N1 = 2
  CLKDIVbits.PLLPOST = 0;     // N2 = 2
   PLLFBD=78; // selecciona  M=80;  siempre lo deseado menos 2;

  AD1PCFGL=0xffff;// todo digital
  //puertos como salida
 TRISB=0x0;
 TRISA=0x0; 
 _TRISA4=1;// t1ck entrada
 _TRISB3=1;// t2ck entrada
 _TRISB5=1;// t3ck entrada
 LATB=0;
 // configuracion de remapeables
 
 
 PPSUnLock;// desbloquea ci¿onfiguracion de perifericos

 iPPSOutput(OUT_PIN_PPS_RP0,OUT_FN_PPS_U1TX);    // con figura rp14 como salida a u1tx
 iPPSInput(IN_FN_PPS_T2CK,IN_PIN_PPS_RP3);  // con figuraRP3 como entrada timer2
 iPPSInput(RPINR3bits.T3CKR,IN_PIN_PPS_RP5);  // con figuraRP5 como entrada timer3
 

 
 PPSLock;// bloquea configuracion periferico
 
 // rx no se va a usar por ahora
 //ahora si configura el puerto serial
 config1= UART_EN & UART_IDLE_CON & UART_IrDA_DISABLE & UART_MODE_SIMPLEX &
          UART_UEN_00 & UART_DIS_WAKE & UART_DIS_LOOPBACK & UART_DIS_ABAUD & 
         UART_BRGH_SIXTEEN &  UART_NO_PAR_8BIT & UART_1STOPBIT;
 
 config2=  UART_INT_TX_BUF_EMPTY  & UART_TX_ENABLE & UART_INT_RX_3_4_FUL &
            UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR & UART_IrDA_POL_INV_ZERO &
            UART_SYNC_BREAK_DISABLED;
 
 OpenUART1 (config1, config2, 259);// se se saca de excel consume menos recursos
  Uart1_write_text_const("INICIANDO\r\n");

 OpenTimer1(T1_ON & T1_GATE_ON &T1_PS_1_64 & T1_SOURCE_INT & T1_INT_PRIOR_3, 31250);
 OpenTimer2(T2_ON & T2_GATE_ON &T2_PS_1_64 & T2_SOURCE_INT & T2_INT_PRIOR_3, 31250);
 OpenTimer3(T3_ON & T3_GATE_ON &T3_PS_1_64 & T3_SOURCE_INT & T3_INT_PRIOR_3, 31250);



 _T1IF=0;
_T1IE=1;

 _T2IF=0;
_T2IE=1;

 _T3IF=0;
_T3IE=1;



habilita(); //AUTORIZA LAS INTERRPCIONES
 while(1)
     
 {
 LATBbits.LATB13=!LATBbits.LATB13;
 __delay_ms(200);

  disparo=0;
  __delay_us(10);
  disparo=1;

    
    ancho1=((float)pwm_in1*(float)prescalador)/40e6;   //  40 es la velocidad de operacion/2
   //para pasarlo a distancia
     dis1=ancho1/58e-6;

    ancho2=((float)pwm_in2*(float)prescalador)/40e6;   //  40 es la velocidad de operacion/2
     //para pasarlo a distancia
    dis2=ancho2/58e-6;
   
      ancho3=((float)pwm_in3*(float)prescalador)/40e6;   //  40 es la velocidad de operacion/2
   //para pasarlo a distancia
    dis3=ancho3/58e-6;


   sprintf(texto,"dis1=%.1fcm    dis2=%.1fcm    dis3=%.1fcm\r\n", dis1,dis2,dis3);
   
 Uart1_write_text(texto);
 }
    return 0;
}

/*
void Configurar_UART(){
    //TRISFbits.TRISFS = 0; // Configura como pin Transmisor
    
    RPINR18 = 0x0000; // RP0: rp14 como U1RX
    RPOR0 = 0x0300; // RP1 como U1TX
    
    //Baudrate is set to 19200 (@ 7.37 MHz)
    U1BRG = BRGVAL;
    //8 - bits, 1 bit strop, Flow ctrl mode
    U1MODE = 0;
    U1STA = 0;
    
    //Enable UART1
    U1MODEbits.UARTEN = 1;
    
    //Enable Transmit
    U1STAbits.UTXEN = 1;
    
    //Reset RX flag
    IEFS0.U1RXIF = 0;
    
    //Enalbe RX interrupt
    IEC0bits.U1RXIE = 1;
}
 */


