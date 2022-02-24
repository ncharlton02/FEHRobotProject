#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>
#include <FEHRPS.h>
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

    DriveDistance(12);
}

int main(void)
{
    LCD.SetBackgroundColor(BACKGROUND_COLOR);
    ShowMessage("Robot Initialized");

    // ProgramPerformanceTest1();

//    DrivetrainSet(25, 25);

    DriveDistance(48);
    // ShowMessage("Drove 12 inches");
    DriveEncoder("Turn", 245, -245);

    // We have completed the code
    LCD.Clear();
    DrawCenteredText("Program Complete", 100, TEXT_COLOR);
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

void DrivetrainStop()
{
    left_motor.Stop();
    right_motor.Stop();
}

void DrivetrainSet(int left, int right)
{
    left_motor.SetPercent(-(double)left);
    right_motor.SetPercent((double)right);
}

void DriveDistance(float inches)
{
    float wheel_radius = 1.25; // 1.5;
    float wheel_circumference = 2.0 * 3.1415 * wheel_radius;
    float cpr = 318.0;
    int counts_total = (int) (cpr / wheel_circumference * inches);

    DriveEncoder("DriveDistance", counts_total, counts_total);
}

void DriveEncoder(const char *label, int left_cts, int right_cts)
{
    int threshold = 20;
    int top_speed = 20;
    int left_direction = 1;
    int right_direction = 1;

    if(left_cts < 0) {
        left_direction = -1;
    }
    
    if(right_cts < 0) {
        right_direction = -1;
    }

    int target_left = left_drive_encoder.Counts() + abs(left_cts);
    int target_right = right_drive_encoder.Counts() + abs(right_cts);

    bool reached_left = false;
    bool reached_right = false;
    while (!reached_left || !reached_right)
    {
        int left_delta = target_left - left_drive_encoder.Counts();
        int right_delta = target_right - right_drive_encoder.Counts();

        int left_power = 0;
        int right_power = 0;

        if (abs(left_delta) < threshold)
        {
            reached_left = true;
        }
        else
        {
            left_power =  top_speed * left_delta / 100;
        }

        if (abs(right_delta) < threshold)
        {
            reached_right = true;
        }
        else if(right_delta > 0)
        {
            right_power = top_speed * right_delta / 100;
        }

        left_power = Clamp(left_power, -top_speed, top_speed);
        right_power = Clamp(right_power, -top_speed, top_speed);

        DrivetrainSet(left_power, right_power);

        LCD.Clear();
        DrawCenteredText(label, 30, TEXT_COLOR);
        DrawVar("Left Power", left_power, 60, TEXT_COLOR);
        DrawVar("Right Power", right_power, 80, TEXT_COLOR);
        DrawVar("Left EncDelta", left_delta, 100, TEXT_COLOR);
        DrawVar("Right EncDelta", right_delta, 120, TEXT_COLOR);
    }
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