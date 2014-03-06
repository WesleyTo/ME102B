 unsigned int rpm;
 unsigned long timeold;
 unsigned int rpmStorage1[] = {0,0};
 unsigned int rpmStorage2[] = {0,0};
 byte currPos = 0;
 byte readRpm = 0;
 unsigned int rpms[3] = {0, 1, 2};
 byte storePos = 0;
 byte upperLimit = 70;
 byte lowerLimit = 50;
 byte resetVal = 60;
 byte flag = 0;

 void setup()
 {
   Serial.begin(9600);
   attachInterrupt(0, hallHigh1, RISING);
   attachInterrupt(1, hallHigh2, RISING);
   digitalWrite(2, HIGH);
   digitalWrite(3, HIGH);
   rpm = 0;
   timeold = 0;
 }
 
 unsigned int getRpm(){
   if (readRpm == 2) {
    return rpmStorage2[currPos] - rpmStorage1[currPos]; 
   }
   else{
     return 0;
   }
 }
 
 void storeRpm() {
   rpms[storePos] = getRpm();
   //if (rpms[storePos] != rpms[(storePos+2)%3]) {
     Serial.println(rpms[storePos]); 
   //}
   storePos = (storePos + 1) % 3;
 }

 void loop()
 {
   storeRpm();
 }

 void hallHigh1()
 {
   if (!flag){
      flag = 1;
      rpmStorage1[currPos] = millis();
   }
 }
 
  void hallHigh2()
 {

   if (flag) {
     if (readRpm < 2) {
       readRpm++;
     }
     rpmStorage2[currPos] = millis();
     currPos = (currPos + 1) % 2;
     flag = 0;
   } else {
     // Going the wrong way
     flag = 1;
   }

 }
 

