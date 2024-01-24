/*
░█▀▄░▀█▀░█▀█░█▀▀░█▀█░█▀█░█▀▀░▀█▀░▀█▀░█▀▀░░░█░░░█▀▀░█▀▄
░█░█░░█░░█▀█░█░█░█░█░█░█░▀▀█░░█░░░█░░█░░░░░█░░░█▀▀░█░█
░▀▀░░▀▀▀░▀░▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀▀▀░░░▀▀▀░▀▀▀░▀▀░
*/
#include "Arduino.h"
#include "DiagnosticLED.h"
/*
░▀▄░░█▀█░█▀▄░▀█▀░█░█░█▀█░▀█▀░█▀▀
░░▄▀░█▀▀░█▀▄░░█░░▀▄▀░█▀█░░█░░█▀▀
░▀░░░▀░░░▀░▀░▀▀▀░░▀░░▀░▀░░▀░░▀▀▀
*/
int DiagnosticLED::_currentDivision() {
    int numDivisions = (_fastFlashes * _DIVISIONS_PER_FAST) + _POST_FAST_PAUSE_DIVISIONS +
                       (_slowFlashes * _DIVISIONS_PER_SLOW) + _POST_SLOW_PAUSE_DIVISIONS;
    int frameTime = numDivisions * _TIME_PER_DIVISION;
    int timeInFrame = millis() % frameTime;
    int division = timeInFrame / _TIME_PER_DIVISION;

    return division;
}

bool DiagnosticLED::_state() {
    int currentDivision = _currentDivision();

    int fastDivisions = _fastFlashes * _DIVISIONS_PER_FAST;
    if (currentDivision < fastDivisions + _POST_FAST_PAUSE_DIVISIONS) {
        if (currentDivision >= fastDivisions) return false;
        if (currentDivision % _DIVISIONS_PER_FAST < _FAST_ACTIVE_DIVISIONS) return true;
    }
    else {
        currentDivision = currentDivision - (fastDivisions + _POST_FAST_PAUSE_DIVISIONS);
        int slowDivisions = _slowFlashes * _DIVISIONS_PER_SLOW;
        if (currentDivision >= slowDivisions) return false;
        if (currentDivision % _DIVISIONS_PER_SLOW < _SLOW_ACTIVE_DIVISIONS) return true;
    }

    return false;
}
/*
░▀▄░░█▀█░█░█░█▀▄░█░░░▀█▀░█▀▀
░░▄▀░█▀▀░█░█░█▀▄░█░░░░█░░█░░
░▀░░░▀░░░▀▀▀░▀▀░░▀▀▀░▀▀▀░▀▀▀
*/
void DiagnosticLED::begin(int pin, bool activeLevel) {
    _pin = pin;
    _activeLevel = activeLevel;

    pinMode(_pin, OUTPUT);
}

void DiagnosticLED::activePattern(int *newPattern) {
    activePattern(newPattern[0], newPattern[1]);
}

void DiagnosticLED::activePattern(int newFast, int newSlow) {
    _fastFlashes = newFast;
    _slowFlashes = newSlow;
}

void DiagnosticLED::update() {
    bool state = _state();

    if (_activeLevel)
        digitalWrite(_pin, state);
    else
        digitalWrite(_pin, !state);
}

void DiagnosticLED::settings(int DIVISIONS_PER_FAST, int DIVISIONS_PER_SLOW,
                               int FAST_ACTIVE_DIVISIONS, int SLOW_ACTIVE_DIVISIONS,
                               int POST_FAST_PAUSE_DIVISIONS, int POST_SLOW_PAUSE_DIVISIONS,
                               int TIME_PER_DIVISION) {
    _DIVISIONS_PER_FAST = DIVISIONS_PER_FAST;
    _DIVISIONS_PER_SLOW = DIVISIONS_PER_SLOW;

    _FAST_ACTIVE_DIVISIONS = FAST_ACTIVE_DIVISIONS;
    _SLOW_ACTIVE_DIVISIONS = SLOW_ACTIVE_DIVISIONS;

    _POST_FAST_PAUSE_DIVISIONS = POST_FAST_PAUSE_DIVISIONS;
    _POST_SLOW_PAUSE_DIVISIONS = POST_SLOW_PAUSE_DIVISIONS;

    _TIME_PER_DIVISION = TIME_PER_DIVISION;
}
