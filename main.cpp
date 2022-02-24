#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>
#include <FEHRPS.h>
#include <FEHBattery.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define TEXT_COLOR 0x00BFFF
#define ERROR_TEXT_COLOR 0xFF0000
#define BACKGROUND_COLOR 0x000000

DigitalEncoder right_drive_encoder(FEHIO::P0_0);
DigitalEncoder left_drive_encoder(FEHIO::P0_1);
FEHMotor right_motor(FEHMotor::Motor0, 9.0);
FEHMotor left_motor(FEHMotor::Motor1, 9.0);

AnalogInputPin cds(FEHIO::P0_2);

void ShowMessage(const char *text);
void DrawCenteredText(const char *text, int y, unsigned int color);
void DrawVar(const char *label, int data, int y, unsigned int color);
void DrawVar(const char *label, float data, int y, unsigned int color);
void ThrowError(int error_code, const char *message, const char *location);

void WaitForStartLight();

void DrivetrainSet(int left, int right);
void DrivetrainStop();
void DriveDistance(float inches);
void DriveEncoder(const char *label, int left_cts, int right_cts);

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

void ProgramPerformanceTest1()
{
    ShowMessage("Program: Perf Test 1");
    WaitForStartLight();

    DriveDistance(24);
}

int main(void)
{
    LCD.SetBackgroundColor(BACKGROUND_COLOR);

    char text[30];
    sprintf(text, "Robot Init: %f V", Battery.Voltage());
    ShowMessage(text);

    ProgramPerformanceTest1();

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
    float no_light_min_value = 1.5;

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

void DriveDistance(float inches) {
    float wheel_radius = 1.25; // 1.5;
    float wheel_circumference = 2.0 * 3.1415 * wheel_radius;
    float cpr = 318.0;
    int counts_total = (int) (cpr / wheel_circumference * inches);

    int drive_power = 25;
    int anti_turn_power = 8;
    int anti_turn_threshold = 5;

    left_drive_encoder.ResetCounts();
    right_drive_encoder.ResetCounts();

    bool drive_left = true;
    bool drive_right = true;
    while(drive_left || drive_right) {
        int left_count = left_drive_encoder.Counts();
        int right_count = right_drive_encoder.Counts();
        int left_power = 0;
        int right_power = 0;

        if(left_count < counts_total) {
            left_power = drive_power;

            if(right_count - left_count > anti_turn_threshold) {
                left_power += anti_turn_power;
            }
        } else {
            drive_left = false;
        }

        if(right_count < counts_total) {
            right_power = drive_power;
            
            if(left_count - right_count > anti_turn_threshold) {
                right_power += anti_turn_power;
            }
        } else {
            drive_right = false;
        }

        // Setup LCD
        LCD.Clear();
        DrawCenteredText("Drive Distance", 30, TEXT_COLOR);
        DrawVar("Left Power", left_power, 60, TEXT_COLOR);
        DrawVar("Right Power", right_power, 80, TEXT_COLOR);
        DrawVar("Left Remain", counts_total - left_count, 100, TEXT_COLOR);
        DrawVar("Right Remain", counts_total - right_count, 120, TEXT_COLOR);

        DrivetrainSet(left_power, right_power);
    }   
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

int Clamp(int val, int min, int max) {
    if(val < min) {
        return min;
    } else if(val > max) {
        return max;
    } else {
        return val;
    }
}