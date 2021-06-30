bool ledOut = false;

void setup()
{
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
}



void loop()
{
    if(ledOut)
    {
        digitalWrite(2, LOW);
        ledOut = false;
    }
    else
    {
        digitalWrite(2, HIGH);
        ledOut = true;
    }


    delay(1000);
}