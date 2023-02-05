EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 2400 950  0    50   Output ~ 0
PWM1_CH1
Wire Wire Line
	2400 950  3750 950 
Text HLabel 2400 1150 0    50   Output ~ 0
PWM0_CH3
Text HLabel 2400 550  0    50   Output ~ 0
PWM0_CH0
Text HLabel 2400 750  0    50   Output ~ 0
PWM0_CH1
Wire Wire Line
	5700 7450 5700 7350
$Comp
L power:GND #PWR?
U 1 1 5FFCBF3D
P 5700 7450
AR Path="/5FFCBF3D" Ref="#PWR?"  Part="1" 
AR Path="/5FFBEF4C/5FFCBF3D" Ref="#PWR01"  Part="1" 
F 0 "#PWR01" H 5700 7200 50  0001 C CNN
F 1 "GND" H 5705 7277 50  0000 C CNN
F 2 "" H 5700 7450 50  0001 C CNN
F 3 "" H 5700 7450 50  0001 C CNN
	1    5700 7450
	1    0    0    -1  
$EndComp
Text Notes 1750 450  0    50   ~ 0
PWM channels to DC motors
Wire Wire Line
	3750 1150 3750 2250
Wire Wire Line
	2400 1150 3750 1150
Wire Wire Line
	4500 750  2400 750 
Wire Wire Line
	2400 550  4500 550 
Wire Wire Line
	4500 2250 3750 2250
$Comp
L MCU_Microchip_SAME:ATSAME70Q21A-AN U?
U 1 1 5FFCBF31
P 5700 3750
AR Path="/5FFCBF31" Ref="U?"  Part="1" 
AR Path="/5FFBEF4C/5FFCBF31" Ref="U1"  Part="1" 
F 0 "U1" H 5792 61  50  0000 C CNN
F 1 "ATSAME70Q21A-AN" H 5792 -30 50  0000 C CNN
F 2 "Package_QFP:LQFP-144_20x20mm_P0.5mm" H 7100 7250 50  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/SAM-E70-S70-V70-V71-Family-Data-Sheet-DS60001527D.pdf" H 5700 3750 50  0001 C CNN
	1    5700 3750
	1    0    0    -1  
$EndComp
$EndSCHEMATC
