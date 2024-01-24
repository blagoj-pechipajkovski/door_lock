/*
░█▀▄░▀█▀░█▀█░█▀▀░█▀█░█▀█░█▀▀░▀█▀░▀█▀░█▀▀░░░█▀▀░█░░░█▀█░█▀▀░█░█░█▀▀░█▀▄
░█░█░░█░░█▀█░█░█░█░█░█░█░▀▀█░░█░░░█░░█░░░░░█▀▀░█░░░█▀█░▀▀█░█▀█░█▀▀░█▀▄
░▀▀░░▀▀▀░▀░▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀▀▀░░░▀░░░▀▀▀░▀░▀░▀▀▀░▀░▀░▀▀▀░▀░▀
*/

#ifndef DIAGNOSTIC_LED
#define DIAGNOSTIC_LED
/*
    BIOS like LED flashing, number of fast flashes folowed by a number of slow flashes
*/
class DiagnosticLED {
    private:
        int _DIVISIONS_PER_FAST = 6;
        int _DIVISIONS_PER_SLOW = 13;

        int _FAST_ACTIVE_DIVISIONS = 2;
        int _SLOW_ACTIVE_DIVISIONS = 10;

        int _POST_FAST_PAUSE_DIVISIONS = 2;
        int _POST_SLOW_PAUSE_DIVISIONS = 12;

        int _TIME_PER_DIVISION = 100;

        int _pin;
        int _activeLevel;

        int _fastFlashes;
        int _slowFlashes;

        int _currentDivision();

        bool _state();

    public:
        void begin(int pin, bool activeLevel = true);

        void activePattern(int *newPattern);

        void activePattern(int newFast, int newSlow);

        void update();

        void settings(int DIVISIONS_PER_FAST, int DIVISIONS_PER_SLOW,
                      int FAST_ACTIVE_DIVISIONS, int SLOW_ACTIVE_DIVISIONS,
                      int POST_FAST_PAUSE_DIVISIONS, int POST_SLOW_PAUSE_DIVISIONS,
                      int TIME_PER_DIVISION);
};
#endif
