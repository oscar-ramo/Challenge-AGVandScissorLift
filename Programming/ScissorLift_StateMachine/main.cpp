/*
 * Project: AGV and Scissor Lift Control - Scissor Lift State Machine
 * File: main.cpp
 * Author: Oscar Gadiel Ramo Martínez
 * Description:
 *   Implements the finite state machine for the Scissor Lift, including:
 *     - LED actuator feedback
 *     - Keypad input logic
 *     - Load cell calibration and weight detection
 *     - Communication sensor detection
 *     - Lifting stepper motor control with height sensor
 *     - Tilting stepper motor control
 *     - Basket servomotor for unloading
 *     - State transitions (setup, load beans, wait for AGV, move mechanism,
 *       lifting, tilting, unloading)
 *
 * Date: June 2025
 * License: MIT (see LICENSE file in repository)
 */

#include <definitions.h>

enum states {state0, state1, state2, state3, state4, state5, state6};

// SUPPORT FUNCTIONS
//LED Actuator
void blinkLED(SimpleGPIO &sensor, int repetition, int ms = 200) {
    for (int i = 0; i < repetition; i++) {
        sensor.set(1);                                  // Turn on sensor
        vTaskDelay(pdMS_TO_TICKS(ms));                 // Wait for ms
        sensor.set(0);                                  // Turn off sensor
        vTaskDelay(pdMS_TO_TICKS(ms));                 // Wait for ms
    }
}

// Lift callback function
void IRAM_ATTR liftCallback(void* arg) {
    static bool state = false;
    state = !state;
    liftPul.set(state);                                 // Toggle the pulse signal for the lift motor
}

// Tilt callback function
void IRAM_ATTR tiltCallback(void *arg) {
    static bool state = false;
    state = !state;
    tiltPul.set(state);                                 // Toggle the pulse signal for the tilt motor
}

// Tilt stop callback function
void IRAM_ATTR tiltStopCallback(void* arg) {
    tiltTimer.stopPeriodic();                           // Stop generating steps
    tiltEna.set(1);                                     // Disable motor (if 1 = disable on your driver)
}

// Keypad
float keypadLogic() {
    // Setup
    lcdDisplay.setup(lcd_pins);
    keypad.setup();
    char buffer[3] = {'\0'};                            // Buffer to store the input weight
    int index = 0;                                      // Index for the buffer
    // Input weight from user
    lcdDisplay.printStr("Input load\nweight in kg");
    vTaskDelay(pdMS_TO_TICKS(3000));
    lcdDisplay.printStr("Press 'A' to\nconfirm");
    while(true) {
        char key = keypad.getKey();
        if (key != '\0') {
            if (key >= '0' && key <= '9' && index < sizeof(buffer) - 1) {
                buffer[index++] = key;                  // Store the digit in the buffer
                buffer[index] = '\0';                   // Null-terminate the string
                lcdDisplay.writeCommand(CMD_CLEAR);
                lcdDisplay.printStr(buffer);
            }
            else if (key == 'A' && index > 0) {
                return atof(buffer);             // Convert to float and return
            }
            else if (key == 'C') {
                lcdDisplay.writeCommand(CMD_CLEAR);
                buffer[0] = '\0';                       // Clear the buffer
                index = 0;                              // Reset index
            }
            else if (key == 'B' && index > 0) {
                index--;                                // Decrement index
                buffer[index] = '\0';                   // Remove last character
                lcdDisplay.writeCommand(CMD_CLEAR);
                lcdDisplay.printStr(buffer);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Load Cell
void loadCellLogic() {
    // Variables defined
    float inputWeight;                                  // User input weight
    float reads;                                        // Variable to store the load cell reading
    const float m = 0.1, b = 0.1;                       // Calibration constants
    float realWeight;                                   // Real weight from load cell
    float lastPrintedWeight = -1000;                    // Last printed weight to avoid flickering
    int stableCount = 0;                                // Counter for stable weight readings
    char msg[32];                                       // Buffer for messages
    inputWeight = keypadLogic();
    lcdDisplay.printStr("Loading beans\nPlease wait...");
    while(true) {
        reads = loadCell.read(ADC_READ_MV);             // Read load cell value
        realWeight = m*reads + b;                       // Real weight calculation
        //Show weight only if it changed
        if (fabs(realWeight - lastPrintedWeight) > 0.05f) {
            sprintf(msg, "Current Weight:\n%.2f kg", realWeight);
            lcdDisplay.printStr(msg);
            lastPrintedWeight = realWeight;             // Update last printed weight
        }
        
        // Check if weight is stable
        if (fabs(realWeight - inputWeight) < 0.05f) {
            stableCount++;
        }
        else {
            stableCount = 0;                            // Reset stable count if weight is not stable
        }
        if (stableCount >= 2) {                         // 2 second stability check
            ledAct.set(1);                              // Turn on buzzer
            lcdDisplay.printStr("Load weight\nreached!");
            vTaskDelay(pdMS_TO_TICKS(3000));            // Wait for 3 seconds
            ledAct.set(0);                              // Turn off buzzer
            break;                                      // Exit the loop)
        }
        vTaskDelay(pdMS_TO_TICKS(1000));                // Wait for 1 sec before next reading
    }
}

// Scissor Lift Communication Sensor
bool comSensorDetect(SimpleGPIO &sensor, int expectedState, int duration_ms, const char *msg, int interval_ms = 50) {
int read; // Variable to store comm sensor reading
int64_t now; // Variable to store current time
int64_t startTime = 0; // Variable to store start time of detection
while(true) {
    read = sensor.get(); // Comm sensor reading
    now = esp_timer_get_time() / 1000; // Current time in ms
    if (read == expectedState) {
        if (startTime == 0) { // First detection
            startTime = now; // Start detection time
        }
        else if (now - startTime >= duration_ms) { // Signal stable for specified duration
            lcdDisplay.printStr(msg);
            return true; // Successful detection
        }
    }
    else startTime = 0; // Reset timer if signal is lost
    vTaskDelay(pdMS_TO_TICKS(interval_ms)); // Check every interval
}
return false; // Detection failed
}

// MAIN FUNCTIONS
bool setup() {
    // LCD
    lcdDisplay.setup(lcd_pins);                         // LCD pins
    lcdDisplay.printStr("System Initializing...");
    // Servomotor
    servoMotor.setup(SERVOMOTOR_GPIO, 0);               // GPIO pin, channel, rest = default
    // Tilt Stepper motor
    tiltPul.setup(TILT_PUL_GPIO, GPO);                  // GPIO pin, output mode, default pull
    tiltDir.setup(TILT_DIR_GPIO, GPO);                  // GPIO pin, output mode, default pull
    tiltEna.setup(TILT_ENA_GPIO, GPO);                  // GPIO pin, output mode, default pull
    tiltDir.set(1);                                     // Direction for tilt motor
    tiltEna.set(1);                                     // tilt motor off
    tiltTimer.setup(tiltCallback, "tilt_timer");
    tiltStopTimer.setup(tiltStopCallback, "tilt_stop_timer");
    // Lift Stepper Motor
    liftPul.setup(LIFT_PUL_GPIO, GPO);                  // GPIO pin, output mode, default pull
    liftDir.setup(LIFT_DIR_GPIO, GPO);                  // GPIO pin, output mode, default pull
    liftEna.setup(LIFT_ENA_GPIO, GPO);                  // GPIO pin, output mode, default pull
    liftDir.set(1);                                     // Direction for lift motor
    liftEna.set(1);                                     // Lift motor off
    liftTimer.setup(liftCallback, "lift_timer");
    // Other components
    heightSensor.setup(HEIGHT_SEN_GPIO, GPI);           // GPIO pin, input mode, default pull
    loadCell.setup(LOAD_CELL_GPIO);                     // GPIO pin, default width = bit 12
    ledAct.setup(BUZZER_GPIO, GPO);                     // GPIO pin, output mode, default pull
    keypad.setup();
    slComSensor.setup(COMM_SENSOR_GPIO, GPI);           // GPIO pin, input mode, default pull
    return true;
}

bool load_beans() {
    keypadLogic();
    // loadCellLogic();
    return true;
}

bool waiting_agv() {
    char msg[] = "AGV coupled succesfully!\nMoving mechanism...";
    lcdDisplay.printStr("Waiting for AGV\nto couple...");
    while(true) {
        return comSensorDetect(slComSensor, 1, 3000, msg); // Detect if mechanism is fully coupled 
    }
    return true;
}

bool move_mechanism() {
    char msg_1[] = "Obstacle detected!";
    char msg_2[] = "The mechanism has arrived at the unloading station!";
    lcdDisplay.printStr("Moving to unload\nstation...");
    while(true) {
        comSensorDetect(slComSensor, 0, 200, msg_1); // Detect if mechanism encounters an obstacle
        return comSensorDetect(slComSensor, 0, 3000, msg_2); // Detect if mechanism arrives at unloading station
    }
    return true;
}

bool lifting_motor() {
    // LCD Setup
    lcdDisplay.setup(lcd_pins);
    heightSensor.setup(HEIGHT_SEN_GPIO, GPI);           // GPIO pin, input mode, default pull
    // Lift Stepper Motor Setup
    liftPul.setup(LIFT_PUL_GPIO, GPO);                  // GPIO pin, output mode, default pull
    liftDir.setup(LIFT_DIR_GPIO, GPO);                  // GPIO pin, output mode, default pull
    liftEna.setup(LIFT_ENA_GPIO, GPO);                  // GPIO pin, output mode, default pull
    liftDir.set(0);                                     // Direction for lift motor
    liftEna.set(1);                                     // Lift motor off
    liftTimer.setup(liftCallback, "lift_timer");
    // Variables
    int read;
    char msg[] = "Lifting mechanism...\nPlease wait...";
    liftEna.set(0); // Enable lift motor
    liftTimer.startPeriodic(3500); // Constant stepping speed
    lcdDisplay.printStr(msg);
    while(true) {
        read = heightSensor.get(); // Read height sensor
        if (read ==0) { // Height sensor triggered
            liftTimer.stopPeriodic(); // Stop generating steps
            liftEna.set(1); // Disable lift motor
            lcdDisplay.printStr("Desired height\nreached!");
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // Wait ms before next reading
    }
    return true;
}

bool tilting_motor() {
    // LCD Setup
    lcdDisplay.setup(lcd_pins);
    // Tilt stepper motor setup
    tiltPul.setup(TILT_PUL_GPIO, GPO);                  // GPIO pin, output mode, default pull
    tiltDir.setup(TILT_DIR_GPIO, GPO);                  // GPIO pin, output mode, default pull
    tiltEna.setup(TILT_ENA_GPIO, GPO);                  // GPIO pin, output mode, default pull
    tiltDir.set(0);                                     // Direction for tilt motor
    tiltEna.set(1);                                     // tilt motor off
    // Timer setup
    tiltTimer.setup(tiltCallback, "tilt_timer");
    tiltStopTimer.setup(tiltStopCallback, "tilt_stop_timer");
    // Variables
    char msg[] = "Tilting basket...\nPlease wait...";
    lcdDisplay.printStr(msg);
    tiltEna.set(0); // Tilt motor ON
    tiltTimer.startPeriodic(15000);                      // Generate steps every 2 seconds
    tiltStopTimer.startOnce(4500000);                   // Stop after 3 seconds
    vTaskDelay(pdMS_TO_TICKS(5500)); // Wait 5.5 seconds to ensure the movement completes
    lcdDisplay.printStr("Tilting complete!");
    return true;
}

bool servomotor() {
    // Initialize
    lcdDisplay.setup(lcd_pins);
    servoMotor.setup(SERVOMOTOR_GPIO, 0);
    servoMotor.setDuty(0);
    // Actions
    lcdDisplay.printStr("Opening basket...\nUnloading beans...");
    servoMotor.setDuty(10);
    vTaskDelay(pdMS_TO_TICKS(1000));
    servoMotor.setDuty(0);
    lcdDisplay.printStr("Unloading\ncomplete!");
    return true;
}

extern "C" void app_main() {
    int next_state;
    bool good;
    states state = state0;
    for (int i = 0; i < 7; i++) {
        switch (state) {
            case state0: // Setup all components
                good = setup();
                if (good == true) {
                    blinkLED(ledAct, 1, 1000);
                    state = static_cast<states>(static_cast<int>(state) + 1);
                    continue;
                }
                else {
                    blinkLED(ledAct, 3);
                    exit(0);
                }
                break;
            case state1: //Move AGV without collision sensors
                good = load_beans();
                if (good == true) {
                    blinkLED(ledAct, 1, 1000);
                    state = static_cast<states>(static_cast<int>(state) + 1);
                    continue;
                }
                else {
                    blinkLED(ledAct, 3);
                    exit(0);
                }
                break;
            case state2: //Move AGV without collision sensors
                good = waiting_agv();
                if (good == true) {
                    blinkLED(ledAct, 1, 1000);
                    state = static_cast<states>(static_cast<int>(state) + 1);
                    continue;
                }
                else {
                    blinkLED(ledAct, 3);
                    exit(0);
                }
                break;
            case state3:
                good = move_mechanism();
                if (good == true) {
                    blinkLED(ledAct, 1, 1000);
                    state = static_cast<states>(static_cast<int>(state) + 1);
                    continue;
                }
                else {
                    blinkLED(ledAct, 3);
                    exit(0);
                }
                break;
            case state4:
                good = lifting_motor();
                if (good == true) {
                    blinkLED(ledAct, 1, 1000);
                    state = static_cast<states>(static_cast<int>(state) + 1);
                    continue;
                }
                else {
                    blinkLED(ledAct, 3);
                    exit(0);
                }
                break;
            case state5:
                good = tilting_motor();
                if (good == true) {
                    blinkLED(ledAct, 1, 1000);
                    state = static_cast<states>(static_cast<int>(state) + 1);
                    continue;
                }
                else {
                    blinkLED(ledAct, 3);
                    exit(0);
                }
                break;
            case state6:
                good = servomotor();
                if (good == true) {
                    blinkLED(ledAct, 1, 1000);
                    state = static_cast<states>(static_cast<int>(state) + 1);
                    continue;
                }
                else {
                    blinkLED(ledAct, 3);
                    exit(0);
                }
                break;
        }
    }
}