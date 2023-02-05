EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 1900 3200 900  950 
U 5FFBCA58
F0 "Motor" 50
F1 "Motor.sch" 50
F2 "PWM0_CH3" I R 2800 3950 50 
F3 "PWM1_CH1" I R 2800 3800 50 
F4 "PWM0_CH1" I R 2800 3650 50 
F5 "PWM0_CH0" I R 2800 3500 50 
$EndSheet
$Sheet
S 4400 3300 600  1500
U 5FFBEF4C
F0 "CPU" 50
F1 "CPU.sch" 50
F2 "PWM0_CH1" O L 4400 3650 50 
F3 "PWM0_CH0" O L 4400 3500 50 
F4 "PWM0_CH3" O L 4400 3950 50 
F5 "PWM1_CH1" O L 4400 3800 50 
$EndSheet
Wire Wire Line
	2800 3500 4400 3500
Wire Wire Line
	4400 3650 2800 3650
Wire Wire Line
	2800 3800 4400 3800
Wire Wire Line
	4400 3950 2800 3950
Text Notes 3250 3450 0    50   ~ 0
5 kHz PWM signals
$EndSCHEMATC
