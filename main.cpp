#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>
#include <FEHRPS.h>
#include <FEHSD.h>

// RPS Delay time
#define RPS_WAIT_TIME_IN_SEC 0.35

// Shaft encoding counts for CrayolaBots
#define COUNTS_PER_INCH 40.5
#define COUNTS_PER_DEGREE 2.48

// Defines for pulsing the robot
#define PULSE_TIME 0.3
#define PULSE_POWER 20

// Define for the motor power
#define POWER 20

// Orientation of QR Code
#define PLUS 0
#define MINUS 1

//Declarations for encoders & motors
DigitalEncoder right_encoder(FEHIO::P0_0);
DigitalEncoder left_encoder(FEHIO::P0_1);
FEHMotor right_motor(FEHMotor::Motor1, 9.0);
FEHMotor left_motor(FEHMotor::Motor0, 9.0);

bool IsRPSConnected() {
    float x = RPS.X();

    if(x < -.99 && x > -1.01) {
        return false;
    } else if(x < -1.99 && x > -2.01) {
        return false;
    }

    return true;
}

/*
 * Pulse forward a short distance using time
 */
void pulse_forward(int percent, float seconds) 
{
    // Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(percent);

    // Wait for the correct number of seconds
    Sleep(seconds);

    // Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

/*
 * Pulse counterclockwise a short distance using time
 */
void pulse_counterclockwise(int percent, float seconds) 
{
    // Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(-percent);

    // Wait for the correct number of seconds
    Sleep(seconds);

    // Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

/*
 * Move forward using shaft encoders where percent is the motor percent and counts is the distance to travel
 */
void move_forward(int percent, int counts) //using encoders
{
    // Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    // Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(percent);

    // While the average of the left and right encoder are less than counts,
    // keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts);

    // Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

/*
 * Turn counterclockwise using shaft encoders where percent is the motor percent and counts is the distance to travel
 */
void turn_counterclockwise(int percent, int counts) 
{
    // Reset encoder counts
    right_encoder.ResetCounts();
    left_encoder.ResetCounts();

    // Set both motors to desired percent
    right_motor.SetPercent(percent);
    left_motor.SetPercent(-percent);

    // While the average of the left and right encoder are less than counts,
    // keep running motors
    while((left_encoder.Counts() + right_encoder.Counts()) / 2. < counts);

    // Turn off motors
    right_motor.Stop();
    left_motor.Stop();
}

/* 
 * Use RPS to move to the desired x_coordinate based on the orientation of the QR code
 */
void check_x(float x_coordinate, int orientation)
{
    // Determine the direction of the motors based on the orientation of the QR code 
    int power = PULSE_POWER;
    if(orientation == MINUS){
        power = -PULSE_POWER;
    }

    // Check if receiving proper RPS coordinates and whether the robot is within an acceptable range
    while(IsRPSConnected() && (RPS.X() < x_coordinate - 1 || RPS.X() > x_coordinate + 1))
    {
        if(RPS.X() < x_coordinate)
        {
            // Pulse the motors for a short duration in the correct direction
            pulse_forward(-power, PULSE_TIME);
        }
        else if(RPS.X() > x_coordinate)
        {
            // Pulse the motors for a short duration in the correct direction
            pulse_forward(power, PULSE_TIME);
        }
        Sleep(RPS_WAIT_TIME_IN_SEC);
    }
}

/* 
 * Use RPS to move to the desired y_coordinate based on the orientation of the QR code
 */
void check_y(float y_coordinate, int orientation)
{
    // Determine the direction of the motors based on the orientation of the QR code
    int power = PULSE_POWER;
    if(orientation == MINUS){
        power = -PULSE_POWER;
    }

    // Check if receiving proper RPS coordinates and whether the robot is within an acceptable range
    while(IsRPSConnected() && (RPS.Y() < y_coordinate - 1 || RPS.Y() > y_coordinate + 1))
    {
        if(RPS.Y() < y_coordinate)
        {
            // Pulse the motors for a short duration in the correct direction
            pulse_forward(-power, PULSE_TIME);
        }
        else if(RPS.Y() > y_coordinate)
        {
            // Pulse the motors for a short duration in the correct direction
           pulse_forward(power, PULSE_TIME);
        }
        Sleep(RPS_WAIT_TIME_IN_SEC);
    }
}

/* 
 * Use RPS to move to the desired heading
 */
void check_heading(float heading)
{
    //You will need to fill out this one yourself and take into account
    //checking for proper RPS data and the edge conditions
    //(when you want the robot to go to 0 degrees or close to 0 degrees)

    int turn_power = 25;
    bool reached_target = false;

    while(IsRPSConnected() && !reached_target) {
        float delta = RPS.Heading() - heading;

        if(delta > 5) {
            left_motor.SetPercent(turn_power);
            right_motor.SetPercent(-turn_power);
            Sleep(PULSE_TIME);
        } else if(delta < -5) {
            left_motor.SetPercent(turn_power);
            right_motor.SetPercent(-turn_power);
            Sleep(-PULSE_TIME);
        } else {
            reached_target = true;
        }

        left_motor.Stop();
        right_motor.Stop();
        Sleep(RPS_WAIT_TIME_IN_SEC);
    }

    /*
        SUGGESTED ALGORITHM:
        1. Check the current orientation of the QR code and the desired orientation of the QR code
        2. Check if the robot is within the desired threshold for the heading based on the orientation
        3. Pulse in the correct direction based on the orientation
    */
}

int main(void)
{
    float touch_x,touch_y;
    float A_x, A_y, B_x, B_y, C_x, C_y, D_x, D_y;
    float A_heading, B_heading, C_heading, D_heading;
    int B_C_counts, C_D_counts, turn_90_counts;

    RPS.InitializeTouchMenu();

    LCD.WriteLine("RPS & Data Logging Test");
    LCD.WriteLine("Press Screen To Start");
    while(!LCD.Touch(&touch_x,&touch_y));
    while(LCD.Touch(&touch_x,&touch_y));

    // COMPLETE CODE HERE TO READ SD CARD FOR LOGGED X AND Y DATA POINTS
    // FEHFile* fptr = SD.FOpen("RPS_TEST.txt", "r");
    // SD.FScanf(fptr, "%f%f", &A_x, &A_y);
    // <ADD CODE HERE>
    // <ADD CODE HERE>
    // <ADD CODE HERE>
    // SD.FClose(fptr);

    A_x = 26.4;
    A_y = 49.2;

    B_x = 27.3;
    B_y = 57.4;

    D_x = 9.8;
    D_y = 48.9;

    // WRITE CODE HERE TO SET THE HEADING DEGREES AND COUNTS VALUES
    A_heading = 90;
    B_heading = 180;
    C_heading = 270;
    D_heading = 0;

    B_C_counts = 647.824; // 16"
    C_D_counts = 404.89; // 10"

    turn_90_counts = 220; // 

    LCD.WriteLine("A to B");
    // A --> B
    check_y(B_y, PLUS);
    check_heading(B_heading);

    LCD.WriteLine("B to C");
    // B --> C
    move_forward(POWER, B_C_counts);
    check_x(C_x, MINUS);
    turn_counterclockwise(POWER, turn_90_counts);
    check_heading(C_heading);

    LCD.WriteLine("C to D");
    // C --> D
    move_forward(POWER, C_D_counts);
    check_y(D_y, MINUS);
    turn_counterclockwise(POWER, turn_90_counts);
    check_heading(D_heading);

    return 0;
}