/* Referencias:
https://www.mouser.es/datasheet/2/302/PCF8583-1127587.pdf
http://www.valachnet.cz/lvanek/diy/homebrain/index.html
https://bitbucket.org/xoseperez/pcf8583/src/master/
*/
#include "LiquidCrystal.h"
#include "DHT.h"
#include "PCF8583.h"

#define DHTPIN 12         // Pin digital
#define DHTTYPE DHT11     // Tipo sensor de humdad y temperatura
#define BAT_LEVEL_PIN A10 // Pin analógico monitorización batería
#define HORA_PIN 0
#define MIN_PIN 1
#define POWER_DHT 6       // Pin alimentación a sensor. Se activa 2 seg cada 30 seg, para ahorrar batería

// Inicialización de la pantalla LCD
const int rs = 18, en = 19, d4 = 20, d5 = 21, d6 = 22, d7 = 23;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Inicialización de DHT
DHT dht(DHTPIN, DHTTYPE);

// Inicialización de RTC (Dirección I2C)
PCF8583 rtc(0xA0);

//Variables globales
unsigned int ano = 2021;
unsigned int mes = 4;
unsigned int dia = 15;
unsigned int hora = 0;
unsigned int min = 0;
unsigned int seg = 0;
unsigned int activacion_sensor = 0; //para no actualizar constantemente el sensor
bool imprimir_fecha = true;

void nivel_bateria();
void temperatura_humedad();
void setup();
void test_rtc(const char * test, byte expected, byte real);
void imprime_hora();
void cambiar_hora();
void imprime_fecha();

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
  lcd.print("starting...");
  delay(500);
  lcd.setCursor(0, 1);

  // Configuración de RTC
  //rtc.setTime(0, 0, 0);
  rtc.setDateTime(seg, min, hora, dia, mes, ano);
  test_rtc("sec", 0, rtc.getSecond());

  //Configuración de DHT
  dht.begin();
  delay(500);

  lcd.print("Ready"); 
  delay(1000);
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
        lcd.print("OK ");
        lcd.print(real);

    } else {
        lcd.print("KO: ");
        lcd.print(test);
        delay(500);
        abort();
    }
}
void temperatura_humedad(){

  if (activacion_sensor == 25){
    digitalWrite(POWER_DHT, HIGH);
  }
  else if (activacion_sensor == 30){  
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
  } 
    activacion_sensor ++;
}

void nivel_bateria(){
    
    int bat_level = 0;
    int bat_level_maped = 0;
    bat_level = analogRead(BAT_LEVEL_PIN);
                          //1023 equivale a 4,2volt (regulado por potenciometro)
                          //600 a 2,5volt (batería gastada)
    bat_level_maped = map(bat_level, 600, 1023, 0, 9); 
    lcd.setCursor(14, 0);
    lcd.print("B");
    lcd.print(bat_level_maped);
    lcd.print(bat_level);  
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
