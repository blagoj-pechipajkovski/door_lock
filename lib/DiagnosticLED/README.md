# DiagnosticLED

Flash a LED quickly then slowly any number of times.

This library makes it easy to specify and visualy read a pattern of fast flashes folowed by slow flashes.

## Usage

### Instantiation

Include library and create an instance.

```c++
#include <DiagnosticLED.h>
DiagnosticLED dl;
```

### Initialization

Specify pin and active level. Active level is the pin state when the LED is ON.

In other words if ```digitalWrite(led_pin, HIGH);``` turns on the led, the active state is ```HIGH```.

```c++
dl.begin(LED_BUILTIN, HIGH); // Works for WeMos D1 MINI ESP32
```

### Patterns

Specifying a pattern can be done by calling ```activePattern``` with the number of fast and slow blinks or with an array of 2 integers.

```c++
int pattern1[2] = {1, 2}; // 1 fast flash, folowed by 2 slow ones
int pattern2[2] = {2, 3}; // 2 fast, 3 slow

dl.activePattern(pattern1); // flash with predefined pattern 1
dl.activePattern(pattern2); // flash with predefined pattern 2

dl.activePattern(3, 4); // flash with new pattern: 3 fast, 4 slow
```

### Loop

Make sure you update the LED status at least every 100ms.

```
dl.update();
```

## Operation

Time is organised in divisions each with equal length, default is 100ms. (```TIME_PER_DIVISION```)

Fast and slow blinks get some amount of divisions, making a blink cycle.

For fast flashes by deafult there are a total of 6 divisions(```DIVISIONS_PER_FAST```), with 2 of then beeing active(```FAST_ACTIVE_DIVISIONS```). So with F as the LED being on, and f as the LED being off, an infinite number of fast flashes would be:

```
FFffffFFffffFFffff...
```

For slow flashes by deafult there are a total of 13 divisions(```DIVISIONS_PER_SLOW```), with 10 of then beeing active(```SLOW_ACTIVE_DIVISIONS```). So with S as the LED being on, and s as the LED being off, an infinite number of slow flashes would be:

```
SSSSSSSSSSsssSSSSSSSSSSsssSSSSSSSSSSsss...
```

After both the fast and slow blink cycles there is a pause, the pause after the slow blinks is longer to more easily distinguish the start of the next pattern.

By default the pause after fast blinks is 2 divisons(```POST_FAST_PAUSE_DIVISIONS```), and after the slow blinks 12 divisions(```POST_SLOW_PAUSE_DIVISIONS```).

So a pattern of 2,2 would be represented as:

```
FFffffFFffffPPSSSSSSSSSSsssSSSSSSSSSSssspppppppppp
```

*assuming defaults*  
*each character ocupies 100ms*  
*F - fast cycle - LED on*  
*f - fast cycle - LED off*  
*P - pause after fast blinks*  
*S - slow cycle - LED on*  
*s - slow cycle - LED off*  
*p - pause after slow blinks*  

## Custom settings

All the settings above can be changed using the folowing

```c++
dl.settings(DIVISIONS_PER_FAST, DIVISIONS_PER_SLOW,
            FAST_ACTIVE_DIVISIONS, SLOW_ACTIVE_DIVISIONS,
            POST_FAST_PAUSE_DIVISIONS, POST_SLOW_PAUSE_DIVISIONS,
            TIME_PER_DIVISION);
```
