/* Referencias:
https://www.mouser.es/datasheet/2/302/PCF8583-1127587.pdf
http://www.valachnet.cz/lvanek/diy/homebrain/index.html
https://bitbucket.org/xoseperez/pcf8583/src/master/
https://harizanov.com/2013/02/power-saving-techniques-on-the-atmega32u4/
https://www.nongnu.org/avr-libc/user-manual/group__avr__power.html
https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7766-8-bit-AVR-ATmega16U4-32U4_Datasheet.pdf
*/
#include "LiquidCrystal.h"
#include "DHT.h"
#include "PCF8583.h"
#include <avr/power.h>

#define DHTPIN 9         // Pin digital
#define DHTTYPE DHT11     // Tipo sensor de humdad y temperatura
#define BAT_LEVEL_PIN A10 // Pin analógico monitorización batería
#define HORA_PIN 0
#define MIN_PIN 1
#define POWER_DHT 6       // Pin alimentación a sensor. Se activa 2 seg cada 30 seg, para ahorrar batería


// Inicialización de la pantalla LCD
const int rs = 19, en = 18, d4 = 20, d5 = 21, d6 = 22, d7 = 23;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Inicialización de DHT
DHT dht(DHTPIN, DHTTYPE);

// Inicialización de RTC (Dirección I2C)
PCF8583 rtc(0xA0);

//Variables globales
unsigned int ano = 2021;
unsigned int mes = 4;
unsigned int dia = 22;
unsigned int hora = 23;
unsigned int min = 4;
unsigned int seg = 0;
unsigned int activacion_sensor = 243; //para no actualizar constantemente el sensor
                                //Para que me muestre la temp y humedad pronto pongo un valor cerca del valor de reinicio del contador (254)
unsigned int activacion_lect_nivel_bat = 990; //aprox 1000 seg (1000 loops)                               
bool imprimir_fecha = true;

void nivel_bateria();
void temperatura_humedad();
void setup();
void test_rtc(const char * test, byte expected, byte real);
void imprime_hora();
void cambiar_hora();
void imprime_fecha();
void low_power();
void blink_test();

void loop() {
  
  while (1){
    
    imprime_fecha();
    temperatura_humedad();
    nivel_bateria();
    imprime_hora();
    cambiar_hora();
    imprime_fecha();
    delay(998);
  }
}

void setup() {

  // E/S
  pinMode(POWER_DHT, OUTPUT);
  digitalWrite(POWER_DHT, LOW);
  pinMode(HORA_PIN, INPUT);
  pinMode(MIN_PIN, INPUT);

  //Configuración de la pantalla
  lcd.begin(16, 2); 
  lcd.print("Starting...");
  delay(500);
  lcd.setCursor(0, 1);

  // Configuración de RTC. Solo para comprobar correcto conexionado
  //rtc.setTime(0, 0, 0);
  //rtc.setDateTime(seg, min, hora, dia, mes, ano); //Descomentar para poner correctamente la serie
  //test_rtc("sec", 0, rtc.getSecond());

  //Configuración de DHT
  dht.begin();

  // Configuración para ahorrar batería
  low_power();
}

void low_power()
{
  ////////////////////////////////////////////
  //Se parte de un consumo inicial de 12.5mV
  ////////////////////////////////////////////
  delay(10000); //para permitir resetear y cargar nuevo firmware
  // Se desactivan periféricos que no se usan del micro
  power_adc_disable(); //Hace que no funcione la lectura analógica del nivel de batería
  //power_aca_enable(); // Se activa solo temporalmente para leer el nivel de batería
  power_usart1_disable();
  power_usart0_disable();
  power_timer1_disable();  
  power_timer2_disable();
  power_timer3_disable();
  power_spi_disable();
  power_usb_disable();
  //power_twi_disable(); //NO USAR Hace que no funcione la pantalla LCD
  ////////////////////////////////////////////
  // Hasta aquí, se consiguen 11mA de consumo
  ////////////////////////////////////////////
  lcd.clear();
  lcd.print("Setting RC ON"); 
  delay(500);

  // Switch to RC Clock 
  UDINT  &= ~(1 << SUSPI); // UDINT.SUSPI = 0; Usb_ack_suspend
  USBCON |= ( 1 <<FRZCLK); // USBCON.FRZCLK = 1; Usb_freeze_clock
  PLLCSR &= ~(1 << PLLE); // PLLCSR.PLLE = 0; Disable_pll
 
  CLKSEL0 |= (1 << RCE); // CLKSEL0.RCE = 1; Enable_RC_clock()
  while ( (CLKSTA & (1 << RCON)) == 0){}    // while (CLKSTA.RCON != 1);  while (!RC_clock_ready())
  CLKSEL0 &= ~(1 << CLKS);  // CLKSEL0.CLKS = 0; Select_RC_clock()
  CLKSEL0 &= ~(1 << EXTE);  // CLKSEL0.EXTE = 0; Disable_external_clock
  
  lcd.clear();
  lcd.print("RC ON"); 
  delay(1000);

  ////////////////////////////////////////////
  // Hasta aquí, se consiguen 7.8mA de consumo
  ////////////////////////////////////////////

  clock_prescale_set(clock_div_4); // Reduce el consumo hasta 5,6mA, pero no funciona el sensor DHT
                                   // Cambiar a "clock_div_1" cuando se tenga que leer el sensor DHT y justo despues volver a "clock_div_2" para ahorrar energía


  ////////////////////////////////////////////
  // Consumo final conseguido: 
  /////                      5.4mA de consumo con "clock_div_2"
  /////                      3.8mA de consumo con "clock_div_4""
  ////////////////////////////////////////////

  lcd.clear();
  lcd.print("Low Power ON"); 
  delay(1000);
  //blink_test();
  lcd.clear();

}

void cambiar_hora(){
  while (digitalRead(MIN_PIN)){
    
    delay(50);
    if (digitalRead(MIN_PIN)){
        if (++min > 59){
          min = 0;
        }
        rtc.setTime(seg, min, hora);
    }
    imprime_hora();
    delay(400); 
  }

  while (digitalRead(HORA_PIN)){
    delay(50);
    if (digitalRead(HORA_PIN)){
        if (++hora > 23){
          hora = 0;
        }
        rtc.setTime(seg, min, hora);
    }
    imprime_hora();
    delay(400);
  }

}

void imprime_hora(){
    lcd.setCursor(0, 0);
    //Hora
    hora = rtc.getHour();
    if (hora<10){
      lcd.print(0);
    }
    lcd.print(hora);
    lcd.print(":");
    //Minuto
    min = rtc.getMinute();
    if (min<10){
      lcd.print(0);
    }
    lcd.print(min);
    lcd.print(":");
    //Segundo
    seg = rtc.getSecond();
    if (seg<10){
      lcd.print(0);
    }
    lcd.print(seg);

}

void test_rtc(const char * test, byte expected, byte real) {
    lcd.clear();
    lcd.setCursor(0, 1); // segunda fila, posición 0
    if (expected == real) {
        lcd.print("OK");
        lcd.print(real);

    } else {
        lcd.print("KO");
        lcd.print(test);
        delay(500);
        abort();
    }
}
void temperatura_humedad(){

  if (activacion_sensor == 245){
    digitalWrite(POWER_DHT, HIGH);
    clock_prescale_set(clock_div_1);
  }
  else if (activacion_sensor == 250){  
    int h = dht.readHumidity();
    float t = dht.readTemperature();
    digitalWrite(POWER_DHT, LOW);
    lcd.setCursor(11, 1); // segunda fila, posición 11

    // Si la lectura no es correcta, imprime fallo
    if (isnan(h) || isnan(t)) {
      lcd.print("Fallo");
    }
    else{
      //Imprime temperatura
      lcd.print(t,1);
      lcd.print("C");
      
      lcd.setCursor(9, 0);
      lcd.print("H");
      lcd.print(h);
      lcd.print("%");
    }
    activacion_sensor = 0;
    clock_prescale_set(clock_div_4);
  } 
    activacion_sensor ++;
}

void nivel_bateria(){
    
    if (activacion_lect_nivel_bat == 995){
      power_adc_enable();
    }
    else if(activacion_lect_nivel_bat == 1000){
      int bat_level = 0;
      int bat_level_maped = 0;
      bat_level = analogRead(BAT_LEVEL_PIN); //El nivel real está dividido entre 2 
      bat_level_maped = map(bat_level*2, 930, 1240, 0, 9); 
                          //1240 equivale a 4 volt
                          //930 equivale a 3 volt (batería gastada)
      lcd.setCursor(14, 0);
      lcd.print("B");
      lcd.print(bat_level_maped);
      lcd.print(bat_level);
      activacion_lect_nivel_bat = 0;
      power_adc_disable();      
    }
  activacion_lect_nivel_bat++;      
}

void imprime_fecha(){

  if ((hora == 0)&&(min == 0)&&(seg<5)){
    imprimir_fecha = true;
  }

  if (imprimir_fecha){
    lcd.setCursor(0,1);
    int day = rtc.getDay();
    if (day<10){
      lcd.print("0");
    }
    lcd.print(day);
    lcd.print("/");

    int mes = rtc.getMonth();
    if (mes<10){
      lcd.print("0");
    }
    lcd.print(rtc.getMonth());
    lcd.print("/");
    
    lcd.print(rtc.getYear()%2000);
    
    imprimir_fecha = false;
  }
}


void blink_test(){

  pinMode(13,OUTPUT); 
  for(int i=0;i<10;i++){
    digitalWrite(13,HIGH);
    delay(100);
    digitalWrite(13,LOW);
    delay(100);
  } 
}
