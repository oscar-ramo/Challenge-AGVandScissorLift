/*
 * Project: AGV and Scissor Lift Control - Scissor Lift State Machine Definitions
 * File: definitions.h
 * Author: Oscar Gadiel Ramo Martínez
 * Description:
 *   Contains GPIO pin mappings and object declarations
 *   for the Scissor Lift subsystems, including:
 *     - Basket servomotor
 *     - Tilting stepper motor
 *     - Lifting stepper motor
 *     - Height sensor
 *     - Load cell
 *     - Buzzer actuator
 *     - LCD display
 *     - Keypad
 *     - Communication sensor
 *
 * Date: June 2025
 * License: MIT (see LICENSE file in repository)
 */

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

//Libraries to use
#include <SimpleADC.h>              //Analog signals
#include <SimpleGPIO.h>             //Digital signals
#include <SimpleKeypad.h>           //Keypad
#include <NibbleLCD.h>              //LCD
#include <SimplePWM.h>              //Motors
#include <SimpleTimer.h>            //Control time
#include <cmath>                    //Math functions

//GPIO pins

//  Basket servomotor
#define SERVOMOTOR_GPIO 36
//  Tilting stepper motor
#define TILT_PUL_GPIO 0
#define TILT_DIR_GPIO 32
#define TILT_ENA_GPIO 33
//  Lifting stepper motor
#define LIFT_PUL_GPIO 25
#define LIFT_DIR_GPIO 26
#define LIFT_ENA_GPIO 27
//  Height sensor
#define HEIGHT_SEN_GPIO 34
//  Load cell
#define LOAD_CELL_GPIO 39
//  Buzzer
#define BUZZER_GPIO 1
//  LCD
//                      D0  D1    D2    D3    D4    D5 D6  D7  RS  RW    EN
uint8_t lcd_pins[11] = {13, NULL, NULL, NULL, NULL, 2, 16, 17, 14, NULL, 12};
//  Keypad
uint8_t keypad_rows[4] = {5, 18, 19, 21};
uint8_t keypad_cols[4] = {15, 4, 22, 23};
//  Communication sensor
#define COMM_SENSOR_GPIO 35

//Object creation
//  Basket servomotor
SimplePWM servoMotor;
//  Tilting stepper motor
SimpleGPIO tiltPul; //Pulse
SimpleGPIO tiltDir; //Direction
SimpleGPIO tiltEna; //Enable
SimpleTimer tiltTimer;
SimpleTimer tiltStopTimer;
//  Lifting stepper motor
SimpleGPIO liftPul;
SimpleGPIO liftDir;
SimpleGPIO liftEna;
SimpleTimer liftTimer;
//  Height sensor
SimpleGPIO heightSensor;
//  Load Cell
SimpleADC loadCell;
// Buzzer
SimpleGPIO ledAct;
//  LCD
NibbleLCD lcdDisplay;
char lcdBuffer[100]; // Buffer for LCD display
//  Keypad
SimpleKeypad keypad(keypad_rows, keypad_cols);
//  Communication
SimpleGPIO slComSensor;

#endif // _DEFINITIONS_H_