#include "mbed.h"
#include "platform_mbed.h"

// hw
// ultrasonic sensor
DigitalOut trig(D9);
InterruptIn echo(D10);

// motor driver (l298n or similar)
PwmOut motorLeftPWM(D6);
PwmOut motorRightPWM(D11);

DigitalOut leftDir1(D2);
DigitalOut leftDir2(D3);
DigitalOut rightDir1(D4);
DigitalOut rightDir2(D5);

// global
Timer pulseTimer;

float ultraSonicDist = 0.0f;      // dist in cm

const float TARGET_DIST = 20.0f;  // dist we want in between
const float MARGIN      = 5.0f;   // some margin for less jitter

// echo interrupts
void onEchoRise() {
    // start on high
    pulseTimer.start();
}

void onEchoFall() {
    // stop on low
    pulseTimer.stop();

    // turn time into distance using read_us() for older Mbed OS versions
    ultraSonicDist = (pulseTimer.read_us() * 0.0343f) / 2.0f;

    pulseTimer.reset();
}

// motion
void moveForward(float speed) {
    // move both motors forward
    leftDir1  = 1;
    leftDir2  = 0;
    rightDir1 = 1;
    rightDir2 = 0;

    motorLeftPWM  = speed;
    motorRightPWM = speed;
}

void stopRobot() {
    // stop both motors
    motorLeftPWM  = 0;
    motorRightPWM = 0;
}

// main
int main() {
    // setup echo intr
    echo.rise(&onEchoRise);
    echo.fall(&onEchoFall);

    // set period of pwm
    motorLeftPWM.period(0.02f);
    motorRightPWM.period(0.02f);

    printf("system started: object following on\n");

    while (true) {
        // trigger us sensor
        trig = 0;
        wait_us(2);

        trig = 1;
        wait_us(10);

        trig = 0;

        // decide
        if (ultraSonicDist > (TARGET_DIST + MARGIN) && ultraSonicDist < 100.0f) {
            // move closer if too far
            moveForward(0.5f);
            printf("following, distance: %.2f cm\n", ultraSonicDist);
        }
        else if (ultraSonicDist < TARGET_DIST || ultraSonicDist > 100.0f) {
            // stop if too close
            stopRobot();
            printf("stopped, distance: %.2f cm\n", ultraSonicDist);
        }
        else {
            // wait on good dist
            stopRobot();
        }

        // compatibility for older Mbed OS versions
        wait_ms(100);
    }
}