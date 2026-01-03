/*
 * Project: AGV and Scissor Lift Control
 * File: main.cpp
 * Author: Oscar Gadiel Ramo Martínez
 * Description:
 *   Implements DC motor test, line follower logic,
 *   communication sensor, and collision avoidance
 *   for the AGV project (4th Semester Challenge).
 *
 * Date: June 2025
 * License: MIT (see LICENSE file in repository)
 */

#include <definitions.h>

// AGV
// DC Motor Test
void dc_motor_test() {
    // Setup 
    dcMotor_1.setup(DCMOTOR1_GPIO, 0); // GPIO, channel, else = default setup
    dcMotor_2.setup(DCMOTOR2_GPIO, 0); // GPIO, channel, else = default setup
    // Make motors move
    dcMotor_1.setDuty(50); // Duty percentage
    dcMotor_2.setDuty(50); // Duty percentage
}

// Line Follower
void lineFollowerLogic(int a, int b) {
    switch ((a << 1) | b) {
        case 0b00: // Both sensors off
            dcMotor_1.setDuty(50);
            dcMotor_2.setDuty(50);
            break;
        case 0b01: // Left On, Right Off
            dcMotor_1.setDuty(25);
            dcMotor_2.setDuty(75);
            break;
        case 0b10: // Left Off, Right On
            dcMotor_1.setDuty(75);
            dcMotor_2.setDuty(25);
            break;
        case 0b11: // Both sensors on
            dcMotor_1.setDuty(0);
            dcMotor_2.setDuty(0);
            agvComSensor.set(0);
            break;
    }

}

void line_follower_test() {
    // Setup
    lineFollower_1.setup(LINE_FOLLOWER1_GPIO, GPI); // GPIO, input mode, default pull
    lineFollower_2.setup(LINE_FOLLOWER2_GPIO, GPI); // GPIO, input mode, default pull
    dcMotor_1.setup(DCMOTOR1_GPIO, 0); // GPIO, channel, else = default setup
    dcMotor_2.setup(DCMOTOR2_GPIO, 1); // GPIO, channel, else = default setup
    
    while (true) { // Infinite loop
        // Detect line
        int a = gpio_get_level((gpio_num_t)LINE_FOLLOWER1_GPIO); //lineFollower_1.get();
        int b = gpio_get_level((gpio_num_t)LINE_FOLLOWER2_GPIO); //lineFollower_2.get();
        lineFollowerLogic(a, b);
        vTaskDelay(pdMS_TO_TICKS(50));
    }    
    
}

// Communication Sensor
void comSensorObstacleLogic(int com_State, bool obstacleDetected = false) {
    // Communication sensor blink variables
    static bool blinkState = false; // Blink state
    static int64_t lastBlink = 0; // Last blink time
    int64_t now;
    switch (com_State) {
        case 1:
            agvComSensor.set(1);
            break;
        case 2:
            now = esp_timer_get_time() / 1000; // Get current time in milliseconds
            if (obstacleDetected && (now - lastBlink > 200)) {
                blinkState = !blinkState; // Toggle blink state
                agvComSensor.set(blinkState); // Toggle signal
                lastBlink = now; // Update last blink time
            }
            else if (obstacleDetected == false) agvComSensor.set(1);
    }
}

void com_sensor_test(int com_state) {
    comSensorObstacleLogic(com_state);
}

// Collision Avoidance
#define MIN_DISTANCE 30 // cm
#define MAX_DISTANCE 10 // cm
#define SOUND_AIR_SPEED 343 // m/s
float read_distance(SimpleGPIO &trig, SimpleGPIO &echo) {
    int64_t start_time = 0, end_time = 0, delta_time;
    float distance;
    // Activate trig
    trig.set(0);
    ets_delay_us(2); // Short delay
    trig.set(1);
    ets_delay_us(10); // 10 us high pulse
    trig.set(0);
    // Echo readings
    while (echo.get() == 0);
    start_time = esp_timer_get_time(); //Wait for echo to go high
    while (echo.get() == 1);
    end_time = esp_timer_get_time(); // Wait for echo to go low (signal complete)
    delta_time = end_time - start_time;
    if (delta_time > 0) distance = (delta_time * 1e-6 * SOUND_AIR_SPEED * 100) / 2; // us to seconds    
    else return -1; // Invalid distance
    return distance;
          
}

void collisionAvoidanceLogic(float distance) {
    const float m = 50/(MIN_DISTANCE - MAX_DISTANCE);
    const float b = -m * MAX_DISTANCE;
    int percentage = static_cast<int>(m * distance + b);
    percentage = std::clamp(percentage, 0, 100); // Ensure percentage is within 0-100
    dcMotor_1.setDuty(percentage);
    dcMotor_2.setDuty(percentage);
}

void collision_avoidance_test(int com_state) {
    float distance;
    bool obstacleDetected = false;
    // Setup
    colliAvoidance_1_trig.setup(COLL_AVOIDANCE1_TRIG_GPIO, GPIO);
    colliAvoidance_1_echo.setup(COLL_AVOIDANCE1_ECHO_GPIO, GPI);
    dcMotor_1.setup(DCMOTOR1_GPIO, 0); // GPIO, channel, else = default setup
    dcMotor_2.setup(DCMOTOR2_GPIO, 1); // GPIO, channel, else = default setup
    dcMotor_1.setDuty(50);
    dcMotor_2.setDuty(50);
    // Activate trig
    while(true) {
        //comSensorObstacleLogic(2, obstacleDetected);
        distance = read_distance(colliAvoidance_1_trig, colliAvoidance_1_echo);
        if (distance <= MIN_DISTANCE && distance >= MAX_DISTANCE) {
            printf("Obstacle detected! At %.2f\n", distance);
            collisionAvoidanceLogic(distance);
            obstacleDetected = true;
        }
        else if (distance > MIN_DISTANCE) {
            printf("No obstacle nearby! Distance is %.2f\n", distance);
            obstacleDetected = false;
        }
        else printf("Possible collision! Distance is %.2f\n", distance);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// MAIN
extern "C" void app_main() {
    // AGV
    //dc_motor_test();
    line_follower_test();
    //collision_avoidance_test(2);
    // com_sensor_test(1);
}