/*
 * Project: AGV and Scissor Lift Control - Scissor Lift Test Definitions
 * File: definitions.h
 * Author: Oscar Gadiel Ramo Martínez
 * Description:
 *   Contains GPIO pin mappings and object declarations
 *   for Scissor Lift subsystem tests, including:
 *     - Basket servomotor
 *     - Tilting stepper motor
 *     - Lifting stepper motor
 *     - Height sensor
 *     - Load cell
 *     - LED actuator (buzzer)
 *     - LCD display
 *     - Keypad
 *     - Communication sensor
 *
 * Date: June 2025
 * License: MIT (see LICENSE file in repository)
 */

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

//Prueba
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
//Libraries to use
#include <SimpleADC.h>              //Analog signals
#include <SimpleGPIO.h>             //Digital signals
#include <SimpleKeypad.h>           //Keypad
#include <NibbleLCD.h>              //LCD
#include <SimplePWM.h>              //Motors
#include <SimpleTimer.h>            //Control time
#include <cmath>                    //Math functions
#include <algorithm>                //Process data
#include <stdio.h>                  //Get in the terminal answers

//GPIO PINS
// Basket servomotor
#define SERVOMOTOR_GPIO 1
// Tilting stepper motor
#define TILT_PUL_GPIO 35
#define TILT_DIR_GPIO 34
#define TILT_ENA_GPIO 25
// Lifting stepper motor
#define LIFT_PUL_GPIO 13
#define LIFT_DIR_GPIO 12
#define LIFT_ENA_GPIO 14
// Height sensor
#define HEIGHT_SEN_GPIO 39
#define TEST_HEIGHT_OUT_GPIO 25
// Load cell
#define LOAD_CELL_GPIO 36
// LED
#define LED_GPIO 33
// LCD
#define PIN_UNUSED 255
//                      D4  D5  D6  D7  RS  RW          EN
uint8_t lcd_pins[11] = {19, 21, 22, 23, 26, PIN_UNUSED, 27};
// 19, 21, 22, 23, 25, PIN_UNUSED, 27
//  Keypad
uint8_t keypad_rows[4] = {18, 5, 17, 16};
uint8_t keypad_cols[4] = {4, 0, 2, 15};
//  Communication sensor
#define COMM_SENSOR_GPIO 32

// SCISSOR LIFT OBJECTS
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
SimpleGPIO testOutHeight;
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