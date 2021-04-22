# Feather_32u4_Clock_DHT
 RTC clock with liquid crystal display, humidity and temperature sensor, powered by lithium battery. Developed on a Feather 32u4 basic board.

![IMG_3784](https://user-images.githubusercontent.com/59566401/114938077-f3c70700-9e3e-11eb-80e3-961b13e4c4c4.jpeg)


## Initial Functional Version

Clock connected to RTC PCF8583.

__It is composed of__:

- Liquid crystal display
- Feather 32u4 basic
- Battery
- DHT11 sensor
- PCF8583

__Features__:

- Displays: time, date, battery level, temperature and humidity.
- Allows to change the time by push buttons



https://user-images.githubusercontent.com/59566401/114939320-90d66f80-9e40-11eb-9616-02955f184454.mp4


## New features. Low power consumption

Using the functions provided by the "avr/power.h" library, the average power consumption has been reduced to 3.8 mA. This allows powering the clock with a 3000mA battery for almost 30 days.

The power consumption reduction is achieved by deactivating the power supply to all the peripherals that are not necessary, and activating the necessary peripherals only during the strictly necessary time.

The following image shows the power consumption reduction achieved.

![low_power_demo](https://user-images.githubusercontent.com/59566401/115783289-f84e6b00-a3bc-11eb-9a0d-353087dca6e8.jpeg)
