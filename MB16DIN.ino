
/*
  MB16DIN.cpp - v1.00 - 12/10/2015:
  - Version inicial
  
  Sketch para el módulo de 16 entradas digitales modbus MB16DIN
  Copyright (c) 2015 Raimundo Alfonso
  Ray Ingeniería Electrónica, S.L.
  
  Este sketch está basado en software libre. Tu puedes redistribuir
  y/o modificarlo bajo los terminos de licencia GNU.

  Esta biblioteca se distribuye con la esperanza de que sea útil,
  pero SIN NINGUNA GARANTÍA, incluso sin la garantía implícita de
  COMERCIALIZACIÓN O PARA UN PROPÓSITO PARTICULAR.
  Consulte los terminos de licencia GNU para más detalles.
  
  * CARACTERISTICAS
  - Medida de 8+8 entradas digitales 24VDC (opcional 48VDC)
  - Selección de polaridad positiva o negativa indenpendiente para los dos grupos de 8 entradas
  - 6 interruptores dipswitch para direccionamiento modbus
  - Bus de comunicaciones RS485 con detección automática de dirección
  - Aislamiento galvánico en entradas digitales hasta 3Kv
  - Aislamiento galvánico en puerto RS485 hasta 3Kv  
  - Amplio rango de alimentación de 6.5VDC a 30VDC (opcional de 9VDC a 72VDC)
  - Regulador conmutado de alta eficiencia
  
  * MAPA MODBUS Y FUNCIONES SOPORTADAS
    MODO R: FUNCION 1 - READ BLOCK COILS
    MODO R: FUNCION 3 - READ BLOCK HOLDING REGISTERS
    MODO W: FUNCION 6 - WRITE SINGLE HOLDING REGISTER
    
  DIRECCION   TIPO    MODO  FORMATO    MAXIMO      MINIMO    UNIDADES    DESCRIPCION
  ---------------------------------------------------------------------------------------------------------
  0x0000      int     R     00000      65535       00000    ---          VALOR DIGITAL ENTRADAS
  0x0005      int     R     00000      00063       00000     ---         ESTADO DEL DIPSWITCH
*/


#include <ModbusSlave.h>
#include <avr/wdt.h> 

#define DIPSW1	10   // Dirección modbus 0
#define DIPSW2	9    // Dirección modbus 1
#define DIPSW3	8    // Dirección modbus 2
#define DIPSW4	7    // Dirección modbus 3
#define DIPSW5	6    // Dirección modbus 4
#define DIPSW6	5    // Dirección modbus 5
#define CLK     4    // CLK registro desplazamiento
#define SL      3    // S/L registro desplazamiento
#define QH      2    // QH registro desplazamiento

#define MAX_BUFFER_RX  15

// Mapa de registros modbus
enum {        
        MB_DIN,          // 16 entradas digitales
        MB_2,            // Reservado
        MB_3,            // Reservado
        MB_4,            // Reservado
        MB_5,            // Reservado
        MB_DIP,          // Estado dipswitch
        MB_REGS	 	 	 // Numero total de registros
};
int regs[MB_REGS];	

// Crea la clase para el modbus...
ModbusSlave modbus;

void setup()  { 
  wdt_disable();
  unsigned int velocidad;
  char paridad;

  // Configura puertos de Arduino  
  pinMode(DIPSW1,INPUT);
  pinMode(DIPSW2,INPUT);	
  pinMode(DIPSW3,INPUT);	
  pinMode(DIPSW4,INPUT);	
  pinMode(DIPSW5,INPUT);	
  pinMode(DIPSW6,INPUT);	
  pinMode(CLK, OUTPUT);
  pinMode(SL, OUTPUT);
  pinMode(QH, INPUT);
  
  digitalWrite(CLK, LOW);
  digitalWrite(SL, LOW);

  // configura modbus...
  modbus.config(9600,'n');
  modbus.direccion = leeDIPSW();

  // Activa WDT cada 4 segundos...   
  wdt_enable(WDTO_4S); 
} 



void loop()  { 
  unsigned int dato;
  char n;
 
  // Lee registro desplazamiento 16 bits...
  digitalWrite(SL, HIGH);
  dato = 0;
  for(n=0;n<=15;n++){
    if(digitalRead(QH) == LOW){
      bitSet(dato, 15-n);
    }    

    digitalWrite(CLK, LOW);
    delayMicroseconds(10);
    digitalWrite(CLK, HIGH);    
    delayMicroseconds(10);
  }  
  digitalWrite(SL, LOW);

  // Asigna valores a la tabla modbus...
  regs[MB_DIN] = dato;
  regs[MB_DIP] = leeDIPSW();

  // Actualiza registros modbus...
  modbus.actualiza(regs,MB_REGS);
       
  delay_modbus(100);
  wdt_reset();
  
}

// Rutina de espera que atiende la tarea modbus...
void delay_modbus(int t){
  int n,tt;
  tt = t/10;
  
  for(n=0;n<=tt;n++){
    modbus.actualiza(regs,MB_REGS);
    delay(10);
  }  
}

// Rutina para leer el dipswitch
byte leeDIPSW(void){
  byte a0,a1,a2,a3,a4,a5;
  
  // Lee dipswitch...
  a0 = !digitalRead(DIPSW1);  
  a1 = !digitalRead(DIPSW2);
  a2 = !digitalRead(DIPSW3);
  a3 = !digitalRead(DIPSW4);
  a4 = !digitalRead(DIPSW5);  
  a5 = !digitalRead(DIPSW6);  

  // Calcula dirección...
  return(a0 + a1*2 + a2*4 + a3*8 + a4*16 + a5*32);
}

