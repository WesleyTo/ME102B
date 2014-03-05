 unsigned int rpm;
 unsigned long timeold;
 unsigned int rpmStorage[] = {0,0};
 byte currPos = 0;
 byte readRpm = 0;
 unsigned int rpms[3] = {0, 1, 2};
 byte storePos = 0;
 byte upperLimit = 70;
 byte lowerLimit = 50;
 byte resetVal = 60;

 void setup()
 {
   Serial.begin(9600);
   attachInterrupt(0, hallHigh, RISING);
   digitalWrite(2, HIGH);
   rpm = 0;
   timeold = 0;
 }
 
 unsigned int getRpm(){
   if (readRpm == 2) {
    return 60000/(rpmStorage[(currPos+1)%2] - rpmStorage[currPos]); 
   }
   else{
     return 0;
   }
 }
 
 void storeRpm() {
   rpms[storePos] = getRpm();
   if (rpms[storePos] != rpms[(storePos+2)%3]) {
     Serial.println(rpms[storePos]); 
   }
   storePos = (storePos + 1) % 3;
 }

 void loop()
 {
   storeRpm();
 }

 void hallHigh()
 {
   if (readRpm < 2) {
     readRpm++;
   }
   rpmStorage[currPos] = millis();
   currPos = (currPos + 1) % 2;
 }

