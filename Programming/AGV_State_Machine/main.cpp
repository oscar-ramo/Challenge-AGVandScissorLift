/*
 * Project: AGV and Scissor Lift Control - AGV State Machine
 * File: main.cpp
 * Author: Oscar Gadiel Ramo Martínez
 * Description:
 *   Implements the finite state machine for the AGV, including:
 *     - Line follower logic
 *     - Collision avoidance using ultrasonic sensors
 *     - Communication sensor signaling
 *     - LED indicators for status feedback
 *     - State transitions (setup, movement without collision sensors, movement with collision sensors)
 *
 * Date: June 2025
 * License: MIT (see LICENSE file in repository)
 */

#include <definitions.h>

// Constant definitions
#define MIN_DISTANCE 30 // cm
#define MAX_DISTANCE 10 // cm
#define SOUND_AIR_SPEED 343 // m/s

enum states {state0, state1, state2};

// SUPPORT-FUNCTIONS
// Line Follower
bool lineFollowerLogic(int a, int b) {
    switch ((a << 1) | b) {
        case 0b00: // Both sensors off
            dcMotor_1.setDuty(00);
            dcMotor_2.setDuty(00);
            comSensorObstacleLogic(3);
            return true;
            break;
        case 0b01: // Left On, Right Off
            dcMotor_1.setDuty(25);
            dcMotor_2.setDuty(75);
            return false;
            break;
        case 0b10: // Left Off, Right On
            dcMotor_1.setDuty(75);
            dcMotor_2.setDuty(25);
            return false;
            break;
        case 0b11: // Both sensors on
            dcMotor_1.setDuty(50);
            dcMotor_2.setDuty(50);
            return false;
            break;
    }

}

// Collision Avoidance
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
            break;
        case 3:
            agvComSensor.set(0);
            break;
    }
}

// Function for led blinking
void ledBlink(SimpleGPIO &sensor, int repetition, int duration) {
    for (int i = 0; i < repetition; i++) {
        sensor.set(1);                                                      // Turn on sensor
        vTaskDelay(pdMS_TO_TICKS(duration));                                    // Wait for 1 second
        sensor.set(0);                                                      // Turn off sensor
        vTaskDelay(pdMS_TO_TICKS(duration));                                    // Wait for m1 second
    }
}

// MAIN FUNCTIONS
bool setup() {
    lineFollower_1.setup(LINE_FOLLOWER1_GPIO, GPI); // GPIO, input mode, default pull
    lineFollower_2.setup(LINE_FOLLOWER2_GPIO, GPI); // GPIO, input mode, default pull
    dcMotor_1.setup(DCMOTOR1_GPIO, 0); // GPIO, channel, else = default setup
    dcMotor_2.setup(DCMOTOR2_GPIO, 1); // GPIO, channel, else = default setup
    colliAvoidance_1_trig.setup(COLL_AVOIDANCE1_TRIG_GPIO, GPIO); // GPIO, output mode, default pull
    colliAvoidance_1_echo.setup(COLL_AVOIDANCE1_ECHO_GPIO, GPI); // GPIO, input mode, default pull
    agvComSensor.setup(COMM_SENSOR_GPIO, GPIO); // GPIO pin, input mode, default pull
    greenLed.setup(GREEN_LED_GPIO, GPO); // GPIO pin, output mode, default pull
    redLed.setup(RED_LED_GPIO, GPO); // GPIO pin, output mode, default pull
    golpeAvisa.setup(GOLPE_AVISA_GPIO, GPI); // GPIO pin, input mode, default pull
    return true;
}

int move_agv(int agv_state) {
    // Variables defined
    float distance;
    bool obstacleDetected = false;
    bool read_collision;
    bool exit;
    // AGV moving state
    switch (agv_state) {
        case 1:
            read_collision = false;
            comSensorObstacleLogic(1);
            break;
        case 2:
            read_collision = true;
            break;
    }
    // Initialize motors
    dcMotor_1.setDuty(50); // Duty percentage
    dcMotor_2.setDuty(50); // Duty percentage
    // Infinite loop
    while(true) {
        // Readings
        int a = lineFollower_1.get(); //int a = gpio_get_level((gpio_num_t)LINE_FOLLOWER1_GPIO);
        int b = lineFollower_2.get(); //int b = gpio_get_level((gpio_num_t)LINE_FOLLOWER2_GPIO);
        int c = golpeAvisa.get();
        if (read_collision == true) distance = read_distance(colliAvoidance_1_trig, colliAvoidance_1_echo);
        // Infrarred sensors
        exit = lineFollowerLogic(a, b);
        if (exit == true) return 1;
        // Collision Avoidance Sensors
        if (distance <= MIN_DISTANCE && distance >= MAX_DISTANCE) {
            printf("Obstacle detected! At %.2f\n", distance);
            collisionAvoidanceLogic(distance);
            obstacleDetected = true;
            comSensorObstacleLogic(2, obstacleDetected);
        }
        else if (distance > MIN_DISTANCE) {
            printf("No obstacle nearby! Distance is %.2f\n", distance);
            obstacleDetected = false;
            comSensorObstacleLogic(2, obstacleDetected);
        }
        else obstacleDetected = false;
        // Switch button
        if (c == 1) return c;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

extern "C" void app_main() {
    int next_state;
    bool good;
    states state = state0;
    for (int i = 0; i < 3; i++) {
        switch (state) {
            case state0: // Setup all components
                good = setup();
                if (good == true) {
                    ledBlink(greenLed, 1, 1000);
                    state = static_cast<states>(static_cast<int>(state) + 1);
                    break;
                }
                else {
                    ledBlink(redLed, 1, 2000);
                    exit(0);
                }
            case state1: //Move AGV without collision sensors
                next_state = move_agv(1);
                if (next_state == 1) {
                    ledBlink(greenLed, 1, 1000);
                    state = static_cast<states>(static_cast<int>(state) + 1);
                    break;
                }
                else exit(0);
            case state2: //Move AGV without collision sensors
                next_state = move_agv(2);
                if (next_state == 1) {
                    ledBlink(greenLed, 1, 1000);
                    exit(0);
                    break;
                }
        }
    }
}