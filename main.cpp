#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>
#include <FEHRPS.h>
#include <string.h>
#include <stdio.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define TEXT_COLOR 0x00BFFF

//Declarations for encoders & motors
DigitalEncoder right_encoder(FEHIO::P0_0);
DigitalEncoder left_encoder(FEHIO::P0_1);
FEHMotor right_motor(FEHMotor::Motor0,9.0);
FEHMotor left_motor(FEHMotor::Motor1,9.0);

void ShowMessage(const char *text);
void ThrowError(int error_code, const char *message, const char *location);

void DrivetrainSet(int left, int right);
void DrivetrainStop();

int main(void)
{
    //Initialize the screen
    LCD.Clear(BLACK);
    LCD.SetFontColor(WHITE);

    ShowMessage("Robot Initialized");

    float endTime = TimeNow() + 3.0;

    while(TimeNow() < endTime) {
        DrivetrainSet(30, 30);
    }

    DrivetrainStop();
    ThrowError(32, "Motor failed", "Main()");
	return 0;
}

void DrivetrainStop() { 
    left_motor.Stop();
    right_motor.Stop();
}

void DrivetrainSet(int left, int right) {
    left_motor.SetPercent((double) left);
    right_motor.SetPercent((double) right);
}

void ShowMessage(const char *text)
{
    int char_width = 12;
    // Find the length of the text, in pixels
    int text_length = strlen(text) * char_width;
    int x = (SCREEN_WIDTH / 2) - (text_length / 2);
    LCD.SetBackgroundColor(BLACK);
    LCD.Clear();
    // Draw the screen
    LCD.SetFontColor(TEXT_COLOR);
    
    LCD.WriteAt(text, x, 100);
    LCD.WriteAt("Click To Continue…", 45, SCREEN_HEIGHT - 30);
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
    
    char title[15];
    sprintf(title, "Error %d", error_code);

    int char_width = 12;

    // Find the length of the text, in pixels
    int text_length = strlen(text) * char_width;
    int title_length = strlen(title) * char_width;
    int location_length = strlen(location) * char_width;

    int text_x = (SCREEN_WIDTH / 2) - (text_length / 2);
    int title_x = (SCREEN_WIDTH / 2) - (title_length / 2);
    int location_x = (SCREEN_WIDTH / 2) - (location_length / 2);

    LCD.SetBackgroundColor(BLACK);
    LCD.Clear();
    // Draw the screen
    LCD.SetFontColor(0xFF0000);
    LCD.WriteAt(title, title_x, 40);
    LCD.WriteAt(text, text_x, 140);
    LCD.WriteAt(location, location_x, 180);

    // Failure so we loop
    while(true) {}
}