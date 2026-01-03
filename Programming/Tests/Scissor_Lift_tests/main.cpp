/*
 * Project: AGV and Scissor Lift Control - Scissor Lift Test main code
 * File: main.cpp
 * Author: Oscar Gadiel Ramo Martínez
 * Description:
 *   Contains test routines for the Scissor Lift subsystems:
 *   - LCD display
 *   - Keypad input
 *   - Load cell calibration and weight detection
 *   - Communication sensor
 *   - LED actuator
 *   - Lifting stepper motor with height sensor
 *   - Tilting stepper motor
 *   - Servomotor for basket unloading
 *
 * Date: June 2025
 * License: MIT (see LICENSE file in repository)
 */


#include <definitions.h>

// SCISSOR LIFT
// LCD
void lcd_test() {
    lcdDisplay.setup(lcd_pins); // LCD pins
    lcdDisplay.printStr("LCD Test\nCompleted!");
}


//Keypad
float keypad_test() {
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
void load_cell_test() {
    // Variables defined
    float inputWeight;                                  // User input weight
    float reads;                                        // Variable to store the load cell reading
    const float m = 0.1, b = 0.1;                       // Calibration constants
    float realWeight;                                   // Real weight from load cell
    float lastPrintedWeight = -1000;                    // Last printed weight to avoid flickering
    int stableCount = 0;                                // Counter for stable weight readings
    char msg[32];                                       // Buffer for messages
    inputWeight = keypad_test();
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

// Communication Sensor test
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

//LED Actuator
void blinkLED(SimpleGPIO &sensor, int repetition, int ms = 200) {
    for (int i = 0; i < repetition; i++) {
        sensor.set(1);                                  // Turn on sensor
        vTaskDelay(pdMS_TO_TICKS(ms));                 // Wait for ms
        sensor.set(0);                                  // Turn off sensor
        vTaskDelay(pdMS_TO_TICKS(ms));                 // Wait for ms
    }
}


//Lifting stepper motor & Height Sensor Test
// Lift callback function
void IRAM_ATTR liftCallback(void* arg) {
    static bool state = false;
    state = !state;
    liftPul.set(state);                                 // Toggle the pulse signal for the lift motor
}

bool lifting_motor_test() {
    // LCD Setup
    lcdDisplay.setup(lcd_pins);
    heightSensor.setup(HEIGHT_SEN_GPIO, GPI);           // GPIO pin, input mode, default pull
    testOutHeight.setup(TEST_HEIGHT_OUT_GPIO, GPIO); // GPIO pin, output, default pull
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
        testOutHeight.set(1); // Output height sensor is ON
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


//Tilting stepper motor test
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
bool tilting_motor_test() {
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

//Servomotor test
bool servomotor_test() {
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
    //lcd_test();
    //keypad_test();
    lifting_motor_test();
    //tilting_motor_test();
    //servomotor_test();
    //load_cell_test();

}