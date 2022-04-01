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

DigitalEncoder right_drive_encoder(FEHIO::P0_0);
DigitalEncoder left_drive_encoder(FEHIO::P3_5);
FEHMotor right_motor(FEHMotor::Motor2, 9.0);
FEHMotor left_motor(FEHMotor::Motor0, 9.0);
FEHServo trayServo(FEHServo::Servo1);
FEHServo armServo(FEHServo::Servo7);
//FEHServo ticketServo(FEHServo::Servo5);
// Line Sensor (3,0)

AnalogInputPin cds(FEHIO::P0_3);

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

void ProgramFinal() {
    ShowMessage("Final Program");
    WaitForStartLight();

    // Drive up the ramp
    armServo.SetDegree(45);
    DriveDistance(6.5, 1);
    Sleep(0.5);
    TurnAngle(-120);
    Sleep(0.5);
    armServo.SetDegree(179);
    Sleep(1.0);
    DriveDistance(33, -1, 60, 60, 1000.0);
    Sleep(0.5);
    TurnAngle(75);
    Sleep(0.5);
    DriveTime(30, 30, 3.0);

    // Tray Servo
    Sleep(0.5);
    DriveDistance(3.5, -1);
    Sleep(0.5);
    TurnAngle(-75);
    Sleep(0.5);
    DriveTime(30, 30, 2.0);
    Sleep(0.5);
    trayServo.SetDegree(90);
    Sleep(1.0);

    // Flip the ice cream lever
    DriveDistance(10.0, -1.0);
    Sleep(1.0);
    armServo.SetDegree(30);
    Sleep(1.0);
    DriveDistance(3.0, 1.0);
    Sleep(0.5);
    DriveDistance(3.0, -1.0);
    Sleep(1.0);
    armServo.SetDegree(180);
    Sleep(1.0);
    armServo.SetDegree(45);
    Sleep(0.5);

    // Back Up From Ice Cream
    DriveDistance(4.0, 1.0);
    Sleep(0.5);
    TurnAngle(73);
    Sleep(0.5);
    DriveDistance(14.5, -1.0, 40, 40, 4.0);
    Sleep(0.5);
    TurnAngle(-73);
    Sleep(0.5);
    DriveDistance(10.0, -1.0, 40, 40, 3.0);
    Sleep(0.5);

    // Flip Burger
    DriveDistance(2.0, 1.0);
    Sleep(0.5);
    armServo.SetDegree(120);
    Sleep(2.0);
    TurnAngle(70, 1.0, 80);
    Sleep(1.0);
    armServo.SetDegree(180);
    Sleep(1.0);
    TurnAngle(-30, 3.0, 40);
    Sleep(0.3);
    armServo.SetDegree(130);
    DriveTime(50, 50, 3.0);

    // Drive to Ticket
    DriveDistance(10.0, 1.0, 40, 40, 4.0);
    Sleep(0.5);
    DriveDistance(2.5, -1.0);
    Sleep(0.5);
    TurnAngle(-70);
    Sleep(0.5);
    DriveDistance(15.0, -1.0, 40, 40, 4.0);
    Sleep(0.5);
    DriveDistance(1.5, 1.0);
    Sleep(0.5);
    TurnAngle(70);
    Sleep(0.5);
    DriveDistance(8.0, 1.0);
    Sleep(0.5);
    TurnAngle(-30);
    Sleep(0.5);
    DriveTime(-30, -30, 1000.0);

    // Sleep(1.0);
    // DriveTime(-20, -20, 4.0);
    // Sleep(1.0);
    // DriveDisance(4, 1);
    // Sleep(1.0);

    DrivetrainStop();
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
    trayServo.SetMin(550);
    trayServo.SetMax(2325);
    trayServo.SetDegree(50);

    armServo.SetMin(500);
    armServo.SetMax(2441);
    armServo.SetDegree(0);
    // Initialize Servos
    //ticketServo.SetMin(515);
    //ticketServo.SetMax(1700); // Note: this could probably be higher
    //ticketServo.SetDegree(75);
    LCD.SetBackgroundColor(BACKGROUND_COLOR);

    char text[30];
    sprintf(text, "Robot Init: %f V", Battery.Voltage());
    ShowMessage(text);
    RPS.InitializeTouchMenu();

    //ProgramRPSTest();
    ProgramFinal();

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
    float no_light_min_value = 0.9;

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
    TurnAngle(degrees, 10000, 25);
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
    DriveDistance(inches, direction, 30, 30, 1000.0);
}

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

void DrivetrainStop()
{
    left_motor.SetPercent(0);
    right_motor.SetPercent(0);
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