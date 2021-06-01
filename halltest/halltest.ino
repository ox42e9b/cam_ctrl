
void setup() 
{
    pinMode(13, OUTPUT);
    pinMode(11, INPUT);
}

void loop() {
    digitalWrite(13, !digitalRead(11));
    delay(100);
}
