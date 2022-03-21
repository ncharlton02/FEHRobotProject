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

#define TEXT_COLOR 0x00BFFF
#define ERROR_TEXT_COLOR 0xFF0000
#define BACKGROUND_COLOR 0x000000

// Error Codes Here
#define ERROR_CODE_INVALID_CLAMP_ARGUMENT 1
#define ERROR_CODE_INVALID_DIRECTION 2
#define ERROR_CODE_INVALID_DISTANCE 3
#define ERROR_CODE_RPS_DISCONNECTED 4

DigitalEncoder right_drive_encoder(FEHIO::P0_0);
DigitalEncoder left_drive_encoder(FEHIO::P3_4);
FEHMotor right_motor(FEHMotor::Motor2, 9.0);
FEHMotor left_motor(FEHMotor::Motor3, 9.0);
FEHServo trayServo(FEHServo::Servo1);
FEHServo armServo(FEHServo::Servo0);
//FEHServo ticketServo(FEHServo::Servo7);

AnalogInputPin cds(FEHIO::P1_7);

void ShowMessage(const char *text);
void DrawCenteredText(const char *text, int y, unsigned int color);
void DrawVar(const char *label, int data, int y, unsigned int color);
void DrawVar(const char *label, float data, int y, unsigned int color);
void ThrowError(int error_code, const char *message, const char *location);

void WaitForStartLight();

void DrivetrainSet(int left, int right);
void DrivetrainStop();
void DriveDistance(float inches, int direction);
void DriveDistance(float inches, int direction, int drive_power_left, int drive_power_right);
void TurnAngle(float degrees);
void TurnAngle(float degrees, double timeout, int power);
void DriveTime(int percent_left, int percent_right, float time);

void RPSSetHeading(float heading);
bool IsRPSConnected();

int Clamp(int val, int min, int max);

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

void ProgramRPSTest() {
    while(true) {
        Sleep(1.0);
        LCD.Clear();

        float x = RPS.X();
        float y = RPS.Y();
        float heading = RPS.Heading();

        DrawCenteredText("RPS Test", 30, TEXT_COLOR);
        DrawVar("X", x, 60, TEXT_COLOR);
        DrawVar("Y", y, 80, TEXT_COLOR);
        DrawVar("H", heading, 100, TEXT_COLOR);
    }
}

void ProgramPerformanceTest3() {
    ShowMessage("Performance Test 3");
    WaitForStartLight();

    DriveDistance(10, 1);
    Sleep(0.5);
    TurnAngle(-110);
    Sleep(0.5);
    armServo.SetDegree(179);
    Sleep(1.0);
    DriveDistance(30, -1, 50, 50);
    Sleep(0.5);
    armServo.SetDegree(0);
    Sleep(1.0);
    TurnAngle(14);
    Sleep(0.5);
    // RPSSetHeading(339.0);
    Sleep(0.5);
    DriveDistance(9.5, -1, 25, 25);
    Sleep(0.5);
    armServo.SetDegree(80);
    TurnAngle(7, 2.0, 30);
    Sleep(0.2);
    armServo.SetDegree(130);
    TurnAngle(7, 2.0, 30);
    Sleep(0.2);
    DriveTime(-25, -25, 0.5);
    Sleep(0.2);
    armServo.SetDegree(180);
    Sleep(0.2);
    TurnAngle(30, 2.0, 30);
    Sleep(0.5);
    DriveTime(-25, -25, 0.25);
    Sleep(0.5);
    TurnAngle(-30, 2.0, 30);
    DriveTime(25, 25, 1.25);
    
    // ticketServo.SetDegree(40)-;
    // Drive to ticket slider
    // Lower ticket arm
    // Drive forward
}

void ProgramPerformanceTest4() {
    ShowMessage("Performance Test 3");
    WaitForStartLight();

    DriveDistance(10, 1);
    Sleep(0.5);
    TurnAngle(-110);
    Sleep(0.5);
    armServo.SetDegree(179);
    Sleep(1.0);
    DriveDistance(35, -1, 50, 50);
    Sleep(0.5);
    armServo.SetDegree(0);
    Sleep(0.5);
    TurnAngle(-30);
    armServo.SetDegree(179);
    Sleep(1.0);
    DriveDistance(4, -1);
    Sleep(0.5);
    armServo.SetDegree(0);
    Sleep(1.0);
    DriveDistance(3, 1);
    Sleep(7.0);
    DriveDistance(3, -1);
    Sleep(0.5);
    armServo.SetDegree(179);
    Sleep(1.0);
    armServo.SetDegree(0);
    Sleep(0.5);
    DriveDistance(7, 1);  
    Sleep(0.5);
    armServo.SetDegree(180); 
    Sleep(0.5);
    TurnAngle(35);
    Sleep(0.5);
    DriveDistance(28, 1);  
    Sleep(0.5);
    TurnAngle(-35);
    DriveDistance(100, 1);  
}

void ProgramTouchCalibrate() {
    ShowMessage("Serov Calibrate: Tray");
    trayServo.TouchCalibrate();
    ShowMessage("Servo Calibrate: Ticket");
    //ticketServo.TouchCalibrate();
}

bool DisplayCDSLight() { 
    float cds_value = cds.Value();

    if(cds_value < 0.6) {
        LCD.SetBackgroundColor(RED);
        LCD.Clear();
        LCD.SetBackgroundColor(BLACK);
        Sleep(1.0);
        return true;
    } else if(cds_value < 1.0) {
        LCD.SetBackgroundColor(BLUE);
        LCD.Clear();
        LCD.SetBackgroundColor(BLACK);
        Sleep(1.0);
        return false;
    } else {
        ThrowError(-1, "Invalid Color", "DisplayLight");
        return true; // Should never reach here
    }
}

int main(void)
{
    RPS.InitializeTouchMenu();
    // Initialize Servos
    //ticketServo.SetMin(515);
    //ticketServo.SetMax(1700); // Note: this could probably be higher
    //ticketServo.SetDegree(75);

    trayServo.SetMin(550);
    trayServo.SetMax(2325);
    trayServo.SetDegree(15);

    armServo.SetMin(500);
    armServo.SetMax(2441);
    armServo.SetDegree(0);
    LCD.SetBackgroundColor(BACKGROUND_COLOR);

    char text[30];
    sprintf(text, "Robot Init: %f V", Battery.Voltage());
    ShowMessage(text);

    //ProgramRPSTest();
    ProgramPerformanceTest4();

    // We have completed the code
    LCD.Clear();
    DrawCenteredText("Program Complete", 100, TEXT_COLOR);
    DrivetrainStop();
    while (true)
    {
    }

    return 0;
}

void WaitForStartLight()
{
    bool light_off = true;
    float no_light_min_value = 0.7;

    LCD.Clear();
    DrawCenteredText("Waiting for start light!", 40, TEXT_COLOR);
    DrawVar("Threshold", no_light_min_value, 75, TEXT_COLOR);

    // Initialize to zero. The screen will be updated in the first loop
    // then next_screen_update will be set to 0.5 seconds from now
    float next_screen_update = 0.0;
    while (light_off)
    {
        float value = cds.Value();
        light_off = value > no_light_min_value;

        if (next_screen_update < TimeNow())
        {
            next_screen_update = TimeNow() + 0.5;
            DrawVar("CDS", value, 100, TEXT_COLOR);
        }
    }
}

void TurnAngle(float degrees) { 
    TurnAngle(degrees, 10000, 20);
}

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

        LCD.Clear();
        DrawCenteredText("Drivetrain Turn", 30, TEXT_COLOR);
        DrawVar("Left Power", left_power, 60, TEXT_COLOR);
        DrawVar("Right Power", right_power, 80, TEXT_COLOR);
        DrawVar("Left Remain", counts_total - left_counts, 100, TEXT_COLOR);
        DrawVar("Right Remain", counts_total - right_counts, 120, TEXT_COLOR);

        DrivetrainSet(left_power, right_power);
    }

    DrivetrainStop();
}

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

void DriveDistance(float inches, int direction) {
    DriveDistance(inches, direction, 25, 25);
}

void DriveDistance(float inches, int direction, int drive_power_left, int drive_power_right)
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

    bool drive_left = true;
    bool drive_right = true;
    while (drive_left || drive_right)
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

        LCD.Clear();
        DrawCenteredText("Drive Distance", 30, TEXT_COLOR);
        DrawVar("Left Power", left_power, 60, TEXT_COLOR);
        DrawVar("Right Power", right_power, 80, TEXT_COLOR);
        DrawVar("Left Remain", counts_total - left_count, 100, TEXT_COLOR);
        DrawVar("Right Remain", counts_total - right_count, 120, TEXT_COLOR);

        DrivetrainSet(left_power, right_power);
    }

    DrivetrainStop();
}

void DrivetrainStop()
{
    left_motor.Stop();
    right_motor.Stop();
}

void DrivetrainSet(int left, int right)
{
    left_motor.SetPercent(-(double)left - 2);
    right_motor.SetPercent((double)right);
}

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

bool IsRPSConnected() {
    float x = RPS.X();

    if(x < -.99 && x > -1.01) {
        return false;
    } else if(x < -1.99 && x > -2.01) {
        return false;
    }

    return true;
}

void RPSSetHeading(float heading) {
    bool at_target = false;
    float deadzone = 3;
    float turn_power = 10;

    float turn_time = 0.2;
    float rps_time = 0.5;

    while(!at_target) {
        if(!IsRPSConnected()) {
            ThrowError(ERROR_CODE_RPS_DISCONNECTED, "RPS Disconnected", "RPSSetHeading");
        }

        float delta = RPS.Heading() - heading;

        LCD.Clear();
        DrawCenteredText("RPS Heading", 30, TEXT_COLOR);
        DrawVar("Delta", delta, 60, TEXT_COLOR);

        if(abs(delta) < deadzone) {
            at_target = true;
        } else {
             if(delta < 0.0) {
                DrivetrainSet(turn_power, -turn_power);
            } else {
                DrivetrainSet(-turn_power, turn_power);
            }

            Sleep(turn_time);
            DrivetrainStop();
            Sleep(rps_time);
        }
    }

    LCD.Clear();
}

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