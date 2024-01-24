#include <Arduino.h>
#include <DiagnosticLED.h>

DiagnosticLED dl;

void setup() {
  // LED_BUILTIN is ON with HIGH (for wemos d1 mini esp32, check your board)
  dl.begin(LED_BUILTIN, HIGH);

  // do 2 fast flashes
  // folowed by 3 slow flashes
  dl.activePattern(2,3);
}

void loop() {
  // update the status
  // needs to be called at least once per 100ms
  dl.update();

  delay(10);
}
