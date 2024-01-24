// not complete

WebVarDec v1 = WebVarDec("v1", 1, PRIVATE, PRIVATE);
WebVarDec v2 = WebVarDec("v2", 2, PRIVATE, PROTECTED);
WebVarDec v3 = WebVarDec("v3", 3, PRIVATE, PUBLIC);
WebVarDec v4 = WebVarDec("v4", 4, PROTECTED, PRIVATE);
WebVarDec v5 = WebVarDec("v5", 5, PROTECTED, PROTECTED);
WebVarDec v6 = WebVarDec("v6", 6, PROTECTED, PUBLIC);
WebVarDec v7 = WebVarDec("v7", 7, PUBLIC, PRIVATE);
WebVarDec v8 = WebVarDec("v8", 8, PUBLIC, PROTECTED);
WebVarDec v9 = WebVarDec("v9", 9, PUBLIC, PUBLIC);
WebVarBool v10 = WebVarBool("v10", true, PUBLIC, PUBLIC);

void setup() {
    // If we want to use preferences
    //preferences.begin("pref_vars");
    //WebVars::set_load_preferences(&preferences);
    
    
    WebVars::begin(&server);
}
