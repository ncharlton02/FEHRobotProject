#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>
#include <FEHRPS.h>
#include <FEHBattery.h>
#include <FEHServo.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <FEHRPS.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SCREEN_ENABLED true

#define TEXT_COLOR 0x00BFFF
#define ERROR_TEXT_COLOR 0xFF0000
#define BACKGROUND_COLOR 0x000000

// Error Codes Here
#define ERROR_CODE_INVALID_CLAMP_ARGUMENT 1
#define ERROR_CODE_INVALID_DIRECTION 2
#define ERROR_CODE_INVALID_DISTANCE 3
#define ERROR_CODE_RPS_DISCONNECTED 4

// CDS Values
#define CDS_VALUE_JUKEBOX_RED 0.6
#define CDS_VALUE_START_LIGHT 0.9

// Start Light Timeout
#define START_LIGHT_TIMEOUT 45.0

DigitalEncoder right_drive_encoder(FEHIO::P0_2);
DigitalEncoder left_drive_encoder(FEHIO::P3_5);
FEHMotor right_motor(FEHMotor::Motor2, 9.0);
FEHMotor left_motor(FEHMotor::Motor0, 9.0);
FEHServo trayServo(FEHServo::Servo1);
FEHServo armServo(FEHServo::Servo7);
FEHServo ticketServo(FEHServo::Servo5);
// Line Sensor (3,0)

AnalogInputPin cds(FEHIO::P0_0);

// Function Declerations
void ShowMessage(const char *text);
void DrawCenteredText(const char *text, int y, unsigned int color);
void DrawVar(const char *label, int data, int y, unsigned int color);
void DrawVar(const char *label, float data, int y, unsigned int color);
void ThrowError(int error_code, const char *message, const char *location);

void WaitForStartLight();

void DrivetrainSet(int left, int right);
void DrivetrainStop();
void DriveDistance(float inches, int direction);
void DriveDistance(float inches, int direction, int drive_power_left, int drive_power_right, double timeout);
void TurnAngle(float degrees);
void TurnAngle(float degrees, double timeout, int power);
void DriveTime(int percent_left, int percent_right, float time);

void RPSSetHeading(float heading);
bool IsRPSConnected();

int Clamp(int val, int min, int max);


//ADAM AND JEREMY ADDED THIS
bool DisplayCDSLight();

void ProgramFinal() {
    // Show a message on the final screen and wait for a press
    ShowMessage("Final Program");

    // Wait for the CDS cell start light
    WaitForStartLight();

    // Drive up the ramp
    armServo.SetDegree(45); // Raise the arm slightly off the ground
    DriveDistance(6.5, 1);
    Sleep(0.5);
    TurnAngle(-112, 5.0, 30);
    Sleep(0.5);
    armServo.SetDegree(179); // Raise the arm all the way up
    Sleep(1.0);
    DriveDistance(32, -1, 60, 60, 1000.0);
    Sleep(0.5);
    TurnAngle(75, 5.0, 30);
    Sleep(0.5);
    DriveTime(40, 40, 2.0);

    // Tray Servo
    Sleep(0.5);
    DriveDistance(3.75, -1);
    Sleep(0.5);
    TurnAngle(-75, 3.0, 25);
    Sleep(0.5);
    DriveTime(35, 35, 1.25);
    Sleep(0.5);
    trayServo.SetDegree(90); // Release the tray
    Sleep(0.5); // Give the tray time to slide down the ramp

    // Flip the ice cream lever
    DriveDistance(10.5, -1.0, 35, 35, 300.0);
    Sleep(0.5);
    armServo.SetDegree(30); // Swing the arm down
    Sleep(0.5);

    // Back up, pause, then drive forward
    DriveDistance(3.0, 1.0);
    Sleep(7.5);
    DriveDistance(2.5, -1.0);
    Sleep(0.5);
    
    // Flip the servo back up
    armServo.SetDegree(180);
    Sleep(1.0);
    armServo.SetDegree(45); // Lower the arm so its low, but not dragging
    Sleep(0.5);

    // Back Up From Ice Cream and navigate to burger
    DriveDistance(4.0, 1.0);
    Sleep(0.5);
    TurnAngle(73); 
    Sleep(0.5);
    DriveDistance(13.5, -1.0, 60, 60, 2.5);
    Sleep(0.5);
    RPSSetHeading(94.0);
    Sleep(0.5);
    DriveDistance(10.0, -1.0, 60, 60, 3.0);
    Sleep(0.5);

    // Flip Burger
    DriveDistance(2.0, 1.0); // backup from the burger wall
    Sleep(0.5);
    armServo.SetDegree(135); // Raise the arm partially
    Sleep(2.0); // Give the arm lots of time since we are pulling a lot of weight on it
    TurnAngle(70, 1.0, 80); // Turn the robot with LOTS of power (to flip burger)
    Sleep(1.0);
    armServo.SetDegree(180); // Raise the arm all the way
    Sleep(1.0);
    TurnAngle(-30, 3.0, 40); // Turn back
    Sleep(0.3);
    armServo.SetDegree(130);
    DriveTime(50, 50, 0.4);
    Sleep(0.5);

    // Navigate to wall
    RPSSetHeading(90.0);
    Sleep(0.5);
    armServo.SetDegree(180);
    Sleep(0.5);
    TurnAngle(-73, 100.0, 30.0); 
    Sleep(0.5);
    DriveDistance(1000.0, 1.0, 40, 40, 2.0);
    Sleep(0.5);

    // Navigate to ticket
    DriveDistance(3.4, -1.0);
    Sleep(0.5);
    TurnAngle(-70);
    Sleep(0.5);
    ticketServo.SetDegree(180);
    Sleep(0.5); 
    DriveDistance(1000.0, -1.0, 40, 40, 1.5);
    Sleep(0.5);
    TurnAngle(50, 2.0, 25);
    Sleep(0.5);
    
    // Line Up with the wall
    TurnAngle(-28, 2.0, 30);
    Sleep(0.5);
    DriveDistance(5.0, 1.0, 35,35,4.0);
    Sleep(0.5);
    ticketServo.SetDegree(50);
    Sleep(0.5);
    TurnAngle(70, 2.0, 30);
    Sleep(0.5);
    DriveDistance(5.0, 1.0, 40, 40, 2.0);
    Sleep(0.5);

    // Navigate down the ramp
    DriveDistance(16.0, -1.0, 40, 40, 3.0);
    Sleep(0.5);
    TurnAngle(-68, 300.0, 35.0);
    Sleep(0.5);
    DriveDistance(27.0, -1.0, 40, 40, 1000.0);
    Sleep(0.5);

    // Line up 
    TurnAngle(73, 300.0, 35.0);
    Sleep(0.5);
    DriveDistance(1000.0, -1.0, 60, 60, 2.0);
    Sleep(0.5);

    //Line up with trash
    DriveDistance(1.0,1);
    Sleep(0.5);
    TurnAngle(-85);
    Sleep(0.5);
    DriveDistance(15,1,40,40,1.0);
    Sleep(0.5);

    //Go to light
    DriveDistance(3,-1,40,40,3.0);
    Sleep(0.5);
    TurnAngle(-9, 300.0, 30.0);
    DriveDistance(6.5,-1,40,40,3.0);
    Sleep(0.5);
    TurnAngle(15, 300.0, 30.0);
    Sleep(0.5);
    armServo.SetDegree(75);


    //press light
    bool is_red = DisplayCDSLight();
    // DisplayCDSLight();
    // //PressButton();
    if(is_red) {
        TurnAngle(2);
    }else {
        TurnAngle(-3);
    }
    Sleep(0.5);
    DriveDistance(2,-1,40,40,3.0); // Press Button
    Sleep(0.5);
    DriveDistance(2,1,40,40,3.0); // Drive Back
    Sleep(0.5);
    if(is_red) {
        TurnAngle(-2);
    } else {
        TurnAngle(3); // Turn
    }
    
    //go to final button
    Sleep(0.5);
    armServo.SetDegree(70);
    TurnAngle(-85);
    Sleep(0.5);
    DriveDistance(13,-1,60,60,3.0);
    Sleep(0.5);
    TurnAngle(58, 300.0, 30);
    Sleep(0.5);
    DriveDistance(10,-1,60,60,3.0);

    Sleep(0.5);
    DriveDistance(4,1,40,40,3.0);
    Sleep(0.5);
    TurnAngle(5);
    Sleep(0.5);
    DriveDistance(14,-1,40,40,3.0);
    Sleep(0.5);
    TurnAngle(5);
    Sleep(0.5);
    DriveDistance(4,1,40,40,3.0);
    Sleep(0.5);
    TurnAngle(5);
    Sleep(0.5);
    DriveDistance(14,-1,40,40,3.0);
    Sleep(0.5);
    TurnAngle(5);
    

    // Drive down the ramp
    // DriveDistance(10.0, 1.0, 40, 40, 4.0);
    // Sleep(0.5);
    // DriveDistance(2.5, -1.0);
    // Sleep(0.5);
    // TurnAngle(-70);
    // Sleep(0.5);
    // DriveDistance(15.0, -1.0, 40, 40, 4.0);
    // Sleep(0.5);
    // DriveDistance(1.5, 1.0);
    // Sleep(0.5);
    // TurnAngle(70);
    // Sleep(0.5);
    // DriveDistance(8.0, 1.0);
    // Sleep(0.5);
    // TurnAngle(-30);
    // Sleep(0.5);
    // DriveTime(-30, -30, 1000.0);

    // End of the main program
    DrivetrainStop();
}

void ProgramCDSTest()
{
    ShowMessage("Program: CDS Test");

    while (true)
    {
        LCD.Clear();

        DrawCenteredText("CDS Cell Test", 30, TEXT_COLOR);
        DrawVar("CDS", cds.Value(), 60, TEXT_COLOR);
        Sleep(0.25);
    }
}

void ProgramTouchCalibrate() {
    ShowMessage("Serov Calibrate: Tray");
    trayServo.TouchCalibrate();
    ShowMessage("Servo Calibrate: Ticket");
    //ticketServo.TouchCalibrate();
}

// Detects the color of the jukebox light
// This will then show that light on the screen.
// This function returns true when the color is RED.
bool DisplayCDSLight() { 
    float cds_value = cds.Value();

    if(cds_value < CDS_VALUE_JUKEBOX_RED) {
        LCD.SetBackgroundColor(RED);
        LCD.Clear();
        LCD.SetBackgroundColor(BLACK);
        Sleep(1.0);
        return true;
    } else {
        LCD.SetBackgroundColor(BLUE);
        LCD.Clear();
        LCD.SetBackgroundColor(BLACK);
        Sleep(1.0);
        return false;
    }
}

// Starting point of program
int main(void)
{
    // Initialize servos
    trayServo.SetMin(550);
    trayServo.SetMax(2325);
    trayServo.SetDegree(50);

    armServo.SetMin(500);
    armServo.SetMax(2441);
    armServo.SetDegree(0);

    ticketServo.SetMin(515);
    ticketServo.SetMax(1700); // Note: this could probably be higher
    ticketServo.SetDegree(75);

    // Initialize Screen
    LCD.SetBackgroundColor(BACKGROUND_COLOR);

    // Display the battery on the screen.
    char text[30];
    sprintf(text, "Robot Init: %f V", Battery.Voltage());
    ShowMessage(text);

    // Enable RPS (and show the selection screen)
    RPS.InitializeTouchMenu();

    // ----------------------------
    // TO RUN CDS TEST, UNCOMMENT LINE BELOW!!!
    // ----------------------------
    // vvvvvvvvvvvvv
    // ProgramCDSTest();
    // ^^^^^^^^^^^^^

    // Call the function with this robot program
    ProgramFinal();

    // We have completed the code
    LCD.Clear();
    DrawCenteredText("Program Complete", 100, TEXT_COLOR);
    DrivetrainStop();
    while (true) // we done, so loop go brrrr
    {
    }

    return 0;
}

// This function will wait for the stop light.
// It will also update the screen to show the 
// value of the CDS cell.
void WaitForStartLight()
{
    bool light_off = true;

    LCD.Clear();
    DrawCenteredText("Waiting for start light!", 40, TEXT_COLOR);
    DrawVar("Threshold", (float) CDS_VALUE_START_LIGHT, 75, TEXT_COLOR);

    // Initialize to zero. The screen will be updated in the first loop
    // then next_screen_update will be set to 0.5 seconds from now
    float next_screen_update = 0.0;
    double start_time = TimeNow();

    // Loop until we detech the light
    while (light_off)
    {
        float value = cds.Value();
        light_off = value > CDS_VALUE_START_LIGHT;

        if (next_screen_update < TimeNow())
        {
            next_screen_update = TimeNow() + 0.5;
            DrawVar("CDS", value, 100, TEXT_COLOR);
            DrawVar("Time", (float) (TimeNow() - start_time), 130, TEXT_COLOR);
        }
    }

    //Light is on, so we can now return!
}

// Turns the robot at 25% power. 
void TurnAngle(float degrees) { 
    TurnAngle(degrees, 10000, 25);
}

// Turns the robot a specified amount with a specified timeout at a specified speed.
//
// Parameters:
// Degrees (how much to turn)
// Timeout (how long before the robot gives up on turning)
// Turn Power (what percent (0-100) power the motors should use)
void TurnAngle(float degrees, double timeout, int turn_power) {
    float turn_radius = 4.5;
    float circumference = 2.0 * 3.141516 * turn_radius;
    float turn_dist = circumference * abs(degrees) / 360.0;

    float wheel_radius = 1.5;
    float wheel_circumference = 2.0 * 3.1415 * wheel_radius;
    float cpr = 318.0;
    int counts_total = (int)(cpr / wheel_circumference * turn_dist);

    int left_dir = 1.0;
    int right_dir = -1.0;

    if(degrees < 0.0) {
        left_dir *= -1;
        right_dir *= -1;
    }

    left_drive_encoder.ResetCounts();
    right_drive_encoder.ResetCounts();

    bool drive_left = true;
    bool drive_right = true;
    float end_time = TimeNow() + timeout;

    while ((drive_left || drive_right) && (TimeNow() < end_time))
    {
        int left_counts = left_drive_encoder.Counts();
        int right_counts = right_drive_encoder.Counts();
        int left_power = 0;
        int right_power = 0;

        if (left_counts < counts_total)
        {
            left_power = turn_power;

            // TODO: add boost?
        }
        else
        {
            drive_left = false;
        }

        if (right_counts < counts_total)
        {
            right_power = turn_power;

            // TODO: Add boost?
        }
        else
        {
            drive_right = false;
        }

        left_power *= left_dir;
        right_power *= right_dir;

        if (SCREEN_ENABLED) {
            LCD.Clear();
            DrawCenteredText("Drivetrain Turn", 30, TEXT_COLOR);
            DrawVar("Left Power", left_power, 60, TEXT_COLOR);
            DrawVar("Right Power", right_power, 80, TEXT_COLOR);
            DrawVar("Left Remain", counts_total - left_counts, 100, TEXT_COLOR);
            DrawVar("Right Remain", counts_total - right_counts, 120, TEXT_COLOR);
        }

        DrivetrainSet(left_power, right_power);
    }

    DrivetrainStop();
}

// Drives the robot for a certain amount of time. Does not
// use encoders to help the robot drive straight.
//
// Parameters
// percent_left (how fast to drive left wheel)
// percent_right (how fast to drive right wheel)
// time (how long to drive, in seconds)
void DriveTime(int percent_left, int percent_right, float time) {
    LCD.Clear();
    DrawCenteredText("Drive Time", 30, TEXT_COLOR);
    DrawVar("Time", time, 60, TEXT_COLOR);
    DrawVar("DriveLeft", percent_left, 80, TEXT_COLOR);
    DrawVar("DriveRight", percent_right, 100, TEXT_COLOR);

    DrivetrainSet(percent_left, percent_right);
    Sleep(time);
    DrivetrainStop();
}

// Drives the robot, in a straight line, a certain number of inches.
// 
// Parameters: 
// inches (the amount of distance to drive)
// direction (the direction to drive, either 1 or -1)
// 
// NOTE: If you pass a direction that is not 1 or -1,
// the code will throw an error on the screen and terminate
// the program
void DriveDistance(float inches, int direction) {
    DriveDistance(inches, direction, 30, 30, 1000.0);
}

// Drives the robot, in a straight line.
// 
// Parameters: 
// inches (the amount of distance to drive)
// direction (the direction to drive, either 1 or -1)
// drive_power_left (the base speed of the left drivetrain)
// drive_power_right (the base speed of the right drivetrain)
// timeout (time, in seconds, before the robot should stop driving)
// 
// NOTE: If you pass a direction that is not 1 or -1,
// the code will throw an error on the screen and terminate
// the program
void DriveDistance(float inches, int direction, int drive_power_left, int drive_power_right, double timeout)
{
    if(abs(direction) != 1) {
        ThrowError(ERROR_CODE_INVALID_DIRECTION, "Invalid Direction", "Drive Distance");
    }

    if(inches < 0) {
        ThrowError(ERROR_CODE_INVALID_DISTANCE, "Invalid Distance", "Drive Distance");
    }

    float wheel_radius = 1.5;
    float wheel_circumference = 2.0 * 3.1415 * wheel_radius;
    float cpr = 318.0;
    int counts_total = (int)(cpr / wheel_circumference * inches);
    int anti_turn_power = 8;
    int anti_turn_threshold = 5;

    left_drive_encoder.ResetCounts();
    right_drive_encoder.ResetCounts();

    float end_time = TimeNow() + timeout;
    bool drive_left = true;
    bool drive_right = true;
    while ((drive_left || drive_right) && (TimeNow() < end_time))
    {
        int left_count = left_drive_encoder.Counts();
        int right_count = right_drive_encoder.Counts();
        int left_power = 0;
        int right_power = 0;

        if (left_count < counts_total)
        {
            left_power = drive_power_left;

            if (right_count - left_count > anti_turn_threshold)
            {
                left_power += anti_turn_power;
            }
        }
        else
        {
            drive_left = false;
        }

        if (right_count < counts_total)
        {
            right_power = drive_power_right;

            if (left_count - right_count > anti_turn_threshold)
            {
                right_power += anti_turn_power;
            }
        }
        else
        {
            drive_right = false;
        }

        left_power *= direction;
        right_power *= direction;

        if(SCREEN_ENABLED) {
            LCD.Clear();
            DrawCenteredText("Drive Distance", 30, TEXT_COLOR);
            DrawVar("Left Power", left_power, 60, TEXT_COLOR);
            DrawVar("Right Power", right_power, 80, TEXT_COLOR);
            DrawVar("Left Remain", counts_total - left_count, 100, TEXT_COLOR);
            DrawVar("Right Remain", counts_total - right_count, 120, TEXT_COLOR);
        }

        DrivetrainSet(left_power, right_power);
    }

    DrivetrainStop();
}

// Stops the drivetrain
void DrivetrainStop()
{
    left_motor.SetPercent(0);
    right_motor.SetPercent(0);
}

// Sets the speed of the drivetrain
void DrivetrainSet(int left, int right)
{
    left_motor.SetPercent(-(double)left - 2);
    right_motor.SetPercent((double)right);
}

// Draws text center on the screen at the specified y-value. 
void DrawCenteredText(const char *text, int y, unsigned int color)
{
    int char_width = 12;
    int text_width = strlen(text) * char_width;
    int x = (SCREEN_WIDTH / 2) - (text_width / 2);

    LCD.SetFontColor(color);
    LCD.WriteAt(text, x, y);
}

void DrawVar(const char *label, float data, int y, unsigned int color)
{
    char text[50];
    sprintf(text, "%s: %.4f", label, data);

    LCD.SetFontColor(color);
    LCD.WriteAt(text, 10, y);
}

void DrawVar(const char *label, int data, int y, unsigned int color)
{
    char text[50];
    sprintf(text, "%s: %d", label, data);

    LCD.SetFontColor(color);
    LCD.WriteAt(text, 10, y);
}

// Shows a message on the screen then pauses the program.
// This function will not return until the screen is tapped.
void ShowMessage(const char *text)
{
    LCD.SetBackgroundColor(BACKGROUND_COLOR);
    LCD.Clear();

    DrawCenteredText(text, 100, TEXT_COLOR);
    DrawCenteredText("Click to continue", SCREEN_HEIGHT - 30, TEXT_COLOR);

    // Wait for the user to touch
    float dummy;
    while (!LCD.Touch(&dummy, &dummy))
    {
    }
    while (LCD.Touch(&dummy, &dummy))
    {
    }

    LCD.Clear();
}

// Shows an error on the screen then 
// stops running the program. 
//
// Warning: THIS FUNCTION DOES NOT RETURN
void ThrowError(int error_code, const char *text, const char *location)
{
    // Stop the robot from moving
    DrivetrainStop();

    // Create title string
    char title[15];
    sprintf(title, "Error %d", error_code);

    LCD.SetBackgroundColor(BACKGROUND_COLOR);
    LCD.Clear();

    DrawCenteredText(title, 40, ERROR_TEXT_COLOR);
    DrawCenteredText(text, 140, ERROR_TEXT_COLOR);
    DrawCenteredText(location, 180, ERROR_TEXT_COLOR);

    // Failure so we loop
    while (true)
    {
    }
}

// Indicates if RPS is connected
// 
// Note: This function returns false in the deadzone
bool IsRPSConnected() {
    float x = RPS.X();

    // RPS is not connected
    if(x < -.99 && x > -1.01) {
        return false;
    } else if(x < -1.99 && x > -2.01) { // RPS Deadzone
        return false;
    }

    return true;
}

// A questionable function that sets the heading of the robot
// using RPS. 
void RPSSetHeading(float heading) {
    bool at_target = false;
    float deadzone = 2.0;
    float close = 7;
    float turn_power = 28;
    float slow_turn_power = 25;

    float turn_time = 0.25;
    float rps_time = 0.5;

    while(!at_target) {
        if(!IsRPSConnected()) {
            ThrowError(ERROR_CODE_RPS_DISCONNECTED, "RPS Disconnected", "RPSSetHeading");
        }

        float delta = RPS.Heading() - heading;

        LCD.Clear();
        DrawCenteredText("RPS Heading", 30, TEXT_COLOR);
        DrawVar("Delta", delta, 60, TEXT_COLOR);
        DrawVar("Heading", RPS.Heading(), 90, TEXT_COLOR);

        if(abs(delta) < deadzone) {
            at_target = true;
        } else if(abs(delta) < close) {
            if(delta < 0.0) {
                DrivetrainSet(-slow_turn_power, slow_turn_power);
            } else {
                DrivetrainSet(slow_turn_power, -slow_turn_power);
            }

            Sleep(turn_time / 2.0);
            DrivetrainStop();
            Sleep(rps_time);
        }else {
            if(delta < 0.0) {
                DrivetrainSet(-turn_power, turn_power);
            } else {
                DrivetrainSet(turn_power, -turn_power);
            }

            Sleep(turn_time);
            DrivetrainStop();
            Sleep(rps_time);
        }
    }

    LCD.Clear();
}

// Make sure a value in inside the specified range.
// 
// Parameters: 
// val - the value to check
// min - the minimum value to return
// max - the maximum value to return
//
// See examples below:
//
// clamp(0.5, 0.0, 1.0) -> Returns 0.5 since 0.0 < 0.5 < 1.0
// clamp(0.2, 0.4, 1.0) -> Returns 0.4 since 0.2 < 0.4
// clamp(1.5, 0.0, 1.0) -> Returns 1.0, since 1.5 > 1.0
//
int Clamp(int val, int min, int max)
{
    if (min > max)
    {
        ThrowError(ERROR_CODE_INVALID_CLAMP_ARGUMENT, "clamp: min < max", "Clamp");
    }

    if (val < min)
    {
        return min;
    }
    else if (val > max)
    {
        return max;
    }
    else
    {
        return val;
    }
}