#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHMotor.h>

enum LineStates
{
LINE_ON_LEFT, MIDDLE, LINE_ON_RIGHT
};

int main(void)
{
    FEHMotor left(FEHMotor::Motor0, 9);
    FEHMotor right(FEHMotor::Motor1, 9);

    AnalogInputPin sensorLeft(FEHIO::P0_2);
    AnalogInputPin sensorMiddle(FEHIO::P0_1);
    AnalogInputPin sensorRight(FEHIO::P0_0);

    LineStates lineState = LineStates::MIDDLE;

    float outerTurnSpeed = 30;
    float innerTurnSpeed = 10;
    float straightSpeed = 20;

    float lineValue = 3.1;
    float noLineValue = 0.2;

    float x, y;
    LCD.Clear();
    LCD.WriteLine("Press to start...");
    while (!LCD.Touch(&x, &y)) {}
    LCD.WriteLine("Start");

    while (true)
    {
        switch (lineState)
        {
        case LineStates::LINE_ON_LEFT:
            right.SetPercent(outerTurnSpeed);
            left.SetPercent(innerTurnSpeed);

            if (sensorMiddle.Value() > 2.3)
            {
                LCD.Clear();
                LCD.WriteLine("On Line");
                lineState = LineStates::MIDDLE;
            }

            break;
        case LineStates::MIDDLE:
            left.SetPercent(straightSpeed);
            right.SetPercent(straightSpeed);

            if (sensorLeft.Value() > 2.7)
            {
                LCD.Clear();
                LCD.WriteLine("Line on Left.");
                lineState = LineStates::LINE_ON_LEFT;
            } else if(sensorRight.Value() > 2.7) {
                LCD.Clear();
                LCD.WriteLine("Line on Right.");
                lineState = LineStates::LINE_ON_RIGHT;
            }

            break;
        case LineStates::LINE_ON_RIGHT:
            right.SetPercent(innerTurnSpeed);
            left.SetPercent(outerTurnSpeed);

            if (sensorMiddle.Value() > 2.3)
            {
                LCD.Clear();
                LCD.WriteLine("Found line.");
                lineState = LineStates::MIDDLE;
            }

            break;
        default:
            LCD.Clear();
            LCD.WriteLine("Invalid Line State: ");
            LCD.WriteLine(lineState);
            return -1;
            break;
        }
    }

    return 0;
}
