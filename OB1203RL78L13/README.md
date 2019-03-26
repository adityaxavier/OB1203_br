# RL78/L13-OB1203 #

This project configures the RL78/L13 to communicate with the OB1203 sensor.
RL78/L13 reads sensor values and calculates Heart Rate and SPO2 values.
Calculated values are displayed on the Native LCD screen.

# System Requirements #
- [IAR Embedded Workbench for RL78 v4.10.1](https://www.iar.com/iar-embedded-workbench/#!?architecture=RL78)
- Applilet 4 for RL78 Microcontrollers AP4 1.10.02.06 distributed with IAR Embedded Workbench for RL78 v4.10.1

## Development Hardware Platform ##
- Renesas Starter Kit for RL78/L13 [YR0K5010WMS000BE](https://www.renesas.com/us/en/products/software-tools/boards-and-kits/starter-kits/renesas-starter-kit-for-rl78-l13.html)
- OB1203 breakout board

## OB1203 to RL78/L13 connection scheme ##

This project requires 3.3 volts DC power supply to the OB1203. The Renesas Starter Kit for RL78/L13 must therefore be configured to supply this voltage by setting
(J5:= Pin2-3 shorted; J6:= All open). See R20UT2124EG0100 sheet 3 of 8.

3.3V can be acquired by connecting to J7 or regulating JA1-1 (5VDC) to 3.3V


| OB1203	| RSKRL78L13
|-----------|----------------------------
| GND		| JA1-2 
| NC		| NC
| NC		| NC
| INTB		| JA5-9
| SDA		| JA1-25
| SCL		| JA1-26
| VLED		| 3V3  (J7) 
| VDD		| 3V3  (J7)


## Evaluation Platform ##
- Optimized H/W design to be released

# Appendix #
## Relevant documentation ##
- [OB1203 Short form Datasheet](https://www.idt.com/document/sds/ob1203-preliminary-short-form-datasheet)
- [OB1203 Datasheet](https://www.idt.com/support?nid=460026&issue_type=sales)
- RL78/L13 Hardware User's Manual
- [Renesas Starter Kit for RL78/L13 - Product Information](https://www.renesas.com/us/en/products/software-tools/boards-and-kits/starter-kits/renesas-starter-kit-for-rl78-l13.html)
- [R20UT2888EG0100 - Renesas Starter Kit LCD Application Board V2](https://www.renesas.com/en-sg/doc/products/tool/doc/003/r20ut2888eg0100_lcdappv2_lcd.pdf)
- [R20UT2518EG0100 - LCD Glass datasheet]()