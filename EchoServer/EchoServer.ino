unsigned long i=0, j=5000;
String msg = "";

void setup() {
  // initialize serial:
  Serial.begin(9600);
  Serial.println("ARDUINO: Connected");
}

void loop() {
//  // if there's any serial available, read it:
//  if (i % j == 0)
//    Serial.println(i/j);
  
  if (Serial.available() > 0) {
    char c = char(Serial.read());
    if (c == '\n') {
      Serial.println("ARDUINO: Got \"" + msg + "\"");
      msg = "";
    }
    else
      msg += c;
  }
  i++;
  delay(1);
}


