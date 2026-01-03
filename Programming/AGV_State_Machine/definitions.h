/*
 * Project: AGV and Scissor Lift Control - AGV State Machine Definitions
 * File: definitions.h
 * Author: Oscar Gadiel Ramo Martínez
 * Description:
 *   Contains GPIO pin mappings and object declarations
 *   for the AGV subsystems, including:
 *     - DC motors
 *     - Line follower sensors
 *     - Collision avoidance sensors (ultrasonic)
 *     - Communication sensor
 *     - LED indicators
 *     - State switch button
 *
 * Date: June 2025
 * License: MIT (see LICENSE file in repository)
 */

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

//Libraries to use
#include <cstdlib>                  // To end program
#include <rom/ets_sys.h>            // Delay without interrumping all the program
#include <SimpleADC.h>              // Analog signals
#include <SimpleGPIO.h>             // Digital signals
#include <SimplePWM.h>              // Motors
#include <SimpleTimer.h>            // Control time
#include <algorithm>                // Process data

//GPIO pins
//  DC motor
#define DCMOTOR1_GPIO 25
#define DCMOTOR2_GPIO 26
//  Line follower
#define LINE_FOLLOWER1_GPIO 33
#define LINE_FOLLOWER2_GPIO 32
//  Collision avoidance
#define COLL_AVOIDANCE1_TRIG_GPIO 17
#define COLL_AVOIDANCE1_ECHO_GPIO 18
//  Comm sensor
#define COMM_SENSOR_GPIO 16
// LEDs to indicate phases
#define GREEN_LED_GPIO 2 // Check with teammate the number
#define RED_LED_GPIO 4 // Check with teammate the number
// Switch button to change agv state
#define GOLPE_AVISA_GPIO 15

//Object creation
//  DC Motor
SimplePWM dcMotor_1;
SimplePWM dcMotor_2;
//  Line follower
SimpleGPIO lineFollower_1;
SimpleGPIO lineFollower_2;
//  Collision avoidance
SimpleGPIO colliAvoidance_1_trig;
SimpleGPIO colliAvoidance_1_echo;
//  Communication sensor
SimpleGPIO agvComSensor;
// LEDs
SimpleGPIO greenLed;
SimpleGPIO redLed;
// Button
SimpleGPIO golpeAvisa;

#endif // _DEFINITIONS_H_