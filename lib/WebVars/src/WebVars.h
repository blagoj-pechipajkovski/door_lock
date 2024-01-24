/*

[WebVars] // contains methods that web variables will need & static varibales for logging creation of web variables
    WebVarStr<size_t> // handles 'const char*'
    [WebVarGeneric<typename>]
        WebVarBool = WebVarGeneric<bool>
        WebVarInt  = WebVarGeneric<int>
        WebVarDec  = WebVarGeneric<double>
        
* [] - uninstantiable

*/

#include <Preferences.h>
#define USE_PREFERENCES true // use this instead of true in constructors
#define HIDE_VALUE true // use this instead of true in str_representation

typedef enum {
    WV_BOOL, // true|false       // true or false
    WV_INT,  // [+-]?\d+         // optional plus/minus followed by one or more digits
    WV_DEC,  // [+-]?\d+(\.\d*)? // optional plus/minus followed by one or more digits with optional dot and additional digits
    WV_STR   // .{0, LEN}        // any caharacters up to the buffer size
} WebVarType;
//TODO// make these return the regex somehow

typedef enum {
    PUBLIC,
    PROTECTED,
    PRIVATE
} WebVarProtectionLevel;
char WebVarProtectionLevel_char(WebVarProtectionLevel l) {
    switch (l) {
        case PUBLIC:
            return '*';
            break;
        case PROTECTED:
            return '+';
            break;
        case PRIVATE:
            return '-';
            break;
    }
    return 'x'; // should never reach this
}


#ifndef MAX_WEB_VARIABLES
    #define MAX_WEB_VARIABLES 20
    #warning MAX_WEB_VARIABLES was not defined, using default 20
#endif
/*
░█░█░█▀▀░█▀▄░░░█░█░█▀█░█▀▄░▀█▀░█▀█░█▀▄░█░░░█▀▀░█▀▀
░█▄█░█▀▀░█▀▄░░░▀▄▀░█▀█░█▀▄░░█░░█▀█░█▀▄░█░░░█▀▀░▀▀█
░▀░▀░▀▀▀░▀▀░░░░░▀░░▀░▀░▀░▀░▀▀▀░▀░▀░▀▀░░▀▀▀░▀▀▀░▀▀▀
*/
class WebVars { //TODO// change class to WebVars
    // █
    // ██ Static
    // █
    // Used for managing WebVars
  private:
  
    static WebVars* _web_variables[MAX_WEB_VARIABLES];
    static int _web_variables_len;
    
    
  protected:
  
    static void add(WebVars* wv) {
        if (_web_variables_len < MAX_WEB_VARIABLES) {
            _web_variables[_web_variables_len] = wv;
            _web_variables_len++;
        }
    }
    
    
  public:
  
    static int count() {
        return _web_variables_len;
    }
    
    static WebVars* get(int index) {
        return _web_variables[index];
    }
    
    static WebVars* get(const char* name) {
        for (int i=0; i<count(); i++) {
            if (strcmp(_web_variables[i]->name(), name) == 0)
                return get(i);
        }
        return nullptr;
    }
    
    
    // █
    // ██ Preferences
    // █
  protected:
    static Preferences* _preferences;
    
  public:
    static void set_preferences(Preferences* new_preferences) {
        WebVars::_preferences = new_preferences;
    }
    static void load_from_preferences() {
        if (_preferences == nullptr) return;
        for (int i=0; i<count(); i++) {
            WebVars* web_variable = get(i);
            if (_preferences->isKey(web_variable->name())) {
                if (_preferences->getType(web_variable->name()) == PT_STR) {
                    char out[100];
                    _preferences->getString(web_variable->name(), out, 100);
                    web_variable->str_to_val(out);
                }
            }
        }
    }
    static void set_load_preferences(Preferences* new_preferences) {
        set_preferences(new_preferences);
        load_from_preferences();
    }
    
    
    // █
    // ██ For derived classes
    // █
  protected:
  
    const char* _name;
    
    void (*_on_change)() = nullptr;
    
    bool _save_to_preferences;
    
    void _change() { // needs to be called from the overriden str_to_val()
        if (_save_to_preferences && WebVars::_preferences != nullptr) {
            char val[100];
            val_str(val);
            WebVars::_preferences->putString(_name, val);
        }
        if (_on_change != nullptr)
            _on_change();
    }
    
    WebVarProtectionLevel _read_pl;
    WebVarProtectionLevel _write_pl;
    
    WebVars(WebVars* wv, WebVarProtectionLevel read_pl=PUBLIC, WebVarProtectionLevel write_pl=PRIVATE, bool save_to_preferences=false) {
        add(wv);
        _read_pl  = read_pl;
        _write_pl = write_pl;
        _save_to_preferences = save_to_preferences;
    }
    
    // Derive value from input string
    virtual char _type_char() = 0;
    
    
  public:
    // Protected members that need to be publicly readable
    const char* name() { return _name; }
    WebVarProtectionLevel read_pl()  { return _read_pl;  }
    WebVarProtectionLevel write_pl() { return _write_pl; }
    
    // Derive value from input string
    virtual void str_to_val(const char* in) = 0;
    
    // Value to string representation
    virtual void val_str(char* out) = 0;
    
    //                                                /1//1//1//  * / /  *  /
    // Representation of variable with aditional info [T][R][W][name]:[value] // [type][read_pl][write_pl]
    void str_representation(char* out, bool hide_val=false) {
        strcpy(out, "");
        
        sprintf(out + strlen(out), "%c", _type_char());
        
        if (!hide_val)
            sprintf(out + strlen(out), "%c", 'n'); // no hide
        else
            sprintf(out + strlen(out), "%c", 'h'); // hide
        
        sprintf(out + strlen(out), "%c", WebVarProtectionLevel_char(_read_pl));
        sprintf(out + strlen(out), "%c", WebVarProtectionLevel_char(_write_pl));
        
        sprintf(out + strlen(out), "%s%c", name(), ':');
        if (!hide_val)
            val_str(out + strlen(out));
    }
    
    void on_change(void (*_on_ch)()) {
        _on_change = _on_ch;
    }
    
  private:
    static const char* _password;
    static const char* password_param_name;
  public:
    static void set_password(const char* new_password) {
        _password = new_password;
    }
    static bool has_valid_pass(AsyncWebServerRequest *request) {
        
        if (_password == nullptr)
            return false;
        
        String password = "";
        
        if (request->hasParam(password_param_name, true))
            password = request->getParam(password_param_name, true)->value();
        else
            return false;
        
        if (String(_password) == password)
            return true;
        
        return false;
    }
    
  private:
    /*
    Getting & setting the web variables
    Returned as plain text, one variable per line, 
    */
    static void set_get(AsyncWebServerRequest* request) {
        String output = "";
        
        //String the_faken_password = "ztk"; //TODO// add functions for specifying password
        //String _password_param_name = "_password";
        //String _password = "";
        // //TODO// add validation function that takes argument to check it for password sthlike: has_valid_pass(AsyncWebServerRequest *request)
        //if (request->hasParam(_password_param_name, true))
        //    _password = request->getParam(_password_param_name, true)->value();
        
        int params = request->params();
        for(int i=0;i<params;i++) {
        
            AsyncWebParameter* p = request->getParam(i);
            if (p->name() == password_param_name) continue;
            
            if(p->isPost()) {
                
                WebVars* web_variable = WebVars::get(p->name().c_str());
                
                if (web_variable != nullptr) {
                    if (request->url() == "/WVset") {
                        if (web_variable->write_pl() == PUBLIC || (web_variable->write_pl() == PROTECTED && has_valid_pass(request)))
                            web_variable->str_to_val(p->value().c_str());
                    }
                    
                    bool hide_value = !(web_variable->read_pl() == PUBLIC || (web_variable->read_pl() == PROTECTED && has_valid_pass(request)));
                    
                    char wv_str[50];
                    web_variable->str_representation(wv_str, hide_value);
                    output += wv_str;
                    output += "\n";
                }
            }
        }
        
        request->send(200, "text/plain", output);
    }
    
    
  public:
    
    static void begin(AsyncWebServer* server) {
        
        // set & get web var values
        server->on("/WVget", HTTP_POST, set_get);
        server->on("/WVset", HTTP_POST, set_get);
        
        server->on("/WV.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/html", "class WebVarElement {\n    \n    constructor(el) {\n        this.el = el;\n    }\n    \n    set_valid(validity=false) {\n        this.el.setAttribute(\'data-valid\', (validity===true).toString());\n    }\n    \n    set_val(new_val) {\n        if (this.el.tagName.toLowerCase() === \'input\')\n            this.el.value = new_val;\n        if (this.el.tagName.toLowerCase() === \'a\')\n            this.el.innerHTML = new_val;\n    }\n    \n    initial_conf(type, hidden, read, write, val) {\n        if (this.el.tagName.toLowerCase() === \'input\') {\n            \n            let classes = [];\n            \n            // Read permisions\n            if (read === WebVar.RW.PRI)\n                classes.push(\'wv-r-pri\');\n            if (read === WebVar.RW.PRO)\n                classes.push(\'wv-r-pro\');\n            if (read === WebVar.RW.PUB)\n                classes.push(\'wv-r-pub\');\n            \n            // Write permisions\n            if (write === WebVar.RW.PRI)\n                classes.push(\'wv-w-pri\');\n            if (write === WebVar.RW.PRO)\n                classes.push(\'wv-w-pro\');\n            if (write === WebVar.RW.PUB)\n                classes.push(\'wv-w-pub\');\n            \n            // is hidden\n            if (hidden === WebVar.Hidden.TRUE)\n                classes.push(\'wv-hdn\');\n            \n            let el = this.el;\n            classes.forEach(function(clas){\n                el.classList.add(clas);\n            });\n            \n            \n            if (hidden === WebVar.Hidden.TRUE) {\n                let placeholder_txt = \'\';\n                \n                if (read === WebVar.RW.PRI && write === WebVar.RW.PUB)\n                    placeholder_txt += \'Input value\';\n                if (read === WebVar.RW.PRI && write === WebVar.RW.PRO)\n                    placeholder_txt += \'Input value (password required)\';\n                if (read === WebVar.RW.PRI && write === WebVar.RW.PRI)\n                    placeholder_txt += \'Read/Write disabled\';\n                \n                if (read === WebVar.RW.PRO && write === WebVar.RW.PUB)\n                    placeholder_txt += \'Password required to read, Input value\';\n                if (read === WebVar.RW.PRO && write === WebVar.RW.PRO)\n                    placeholder_txt += \'Password required to read, Input value (password required)\';\n                if (read === WebVar.RW.PRO && write === WebVar.RW.PRI)\n                    placeholder_txt += \'Password required to read\';\n                \n                this.el.setAttribute(\'placeholder\', placeholder_txt);\n            }\n            \n            \n            var title_txt = \'Read: \';\n            \n            // Read permisions\n            if (read === WebVar.RW.PRI)\n                title_txt += \'no-one\';\n            if (read === WebVar.RW.PRO)\n                title_txt += \'w/ password\';\n            if (read === WebVar.RW.PUB)\n                title_txt += \'anyone\';\n            \n            title_txt += \' / Write: \';\n            \n            // Write permisions\n            if (write === WebVar.RW.PRI)\n                title_txt += \'no-one\';\n            if (write === WebVar.RW.PRO)\n                title_txt += \'w/ password\';\n            if (write === WebVar.RW.PUB)\n                title_txt += \'anyone\';\n            \n            this.el.setAttribute(\'title\', title_txt);\n            \n            \n            if (write === WebVar.RW.PRI)\n                this.el.setAttribute(\'disabled\', \'\');\n            \n            \n            let new_val_fn = function() {\n                if (this.getAttribute(\'type\').toLowerCase() === \'checkbox\')\n                    this.value = this.checked;\n                WebVars.WV.get(this.getAttribute(\'data-webvar\')).val = this.value;\n            };\n            \n            if (type === WebVar.Type.BOOLEAN) {\n                this.el.setAttribute(\'type\', \'checkbox\');\n                this.el.onchange = new_val_fn;\n                if (val==\'true\')\n                    this.el.setAttribute(\'checked\', \'\');\n            }\n            else {\n                this.el.setAttribute(\'type\', \'text\');\n                this.el.onkeyup = new_val_fn;\n            }\n            \n            this.set_val(val);\n        }\n        if (this.el.tagName.toLowerCase() === \'a\') {\n            if (hidden !== WebVar.Hidden.TRUE)\n                this.el.innerHTML = val;\n        }\n    }\n    \n    no_var() {\n        if (this.el.tagName.toLowerCase() === \'input\') {\n            this.el.classList.add(\'wv-invalid\');\n            this.el.setAttribute(\'type\', \'text\');\n            this.el.value = \'WebVar \\\'\' +\n                            this.el.getAttribute(\'data-webvar\') +\n                            \'\\\' does not exist\';\n            this.el.setAttribute(\'disabled\', \'\');\n        }\n        if (this.el.tagName.toLowerCase() === \'a\') {\n            this.el.style.textDecoration = \'line-through\';\n        }\n    }\n}\n\nclass WebVar {\n    \n    static Type = {\n        BOOLEAN : \'b\',\n        INTEGER : \'i\',\n        DECIMAL : \'d\',\n        STRING  : \'s\',\n        \n        check_valid: function (in_char) {\n            // wtf have I done\n            return WebVar.\n                   Type[Object.keys(this).find(key => this[key] === in_char)];\n        }\n    };\n    \n    static Hidden = {\n        TRUE  : \'h\',\n        FALSE : \'n\',\n        \n        check_valid: function (in_char) {\n            // wtf have I done\n            return WebVar.\n                   Hidden[Object.keys(this).find(key => this[key] === in_char)];\n        }\n    };\n    \n    static RW = {\n        PRI : \'-\',\n        PRO : \'+\',\n        PUB : \'*\',\n        \n        check_valid: function (in_char) {\n            // wtf have I done\n            return WebVar.\n                   RW[Object.keys(this).find(key => this[key] === in_char)];\n        }\n    };\n    \n    static get_regex(type) {\n        switch(type) {\n            case WebVar.Type.BOOLEAN:\n                return /^true$|^false$/;\n                break;\n            case WebVar.Type.INTEGER:\n                return /^[+-]?\\d+$/;\n                break;\n            case WebVar.Type.DECIMAL:\n                return /^[+-]?\\d+(\\.\\d*)?$/;\n                break;\n            case WebVar.Type.STRING:\n                return /^.*$/i;\n                break;\n            default:\n                return /.*/i;// /(?!.*)/i;\n        }\n    }\n    \n    #type;\n    get type() { return this.#type; }\n    \n    #hidden;\n    get hidden() { return this.#hidden; }\n    \n    #read;\n    get read() { return this.#read; }\n    #write;\n    get write() { return this.#write; }\n    \n    #name;\n    get name() { return this.#name; }\n    \n    #modified = false;\n    get modified() { return this.#modified; }\n    \n    #WebVar_elements = [];\n    \n    is_valid() {\n        return WebVar.get_regex(this.type).test(this.#val) || this.#val === \'\';\n    }\n    \n    #val = \'\';\n    get val() { return this.#val; }\n    set val(new_val) {\n        new_val = new_val.toString();\n        this.#val = new_val;\n        for (const WebVar_el of this.#WebVar_elements) {\n            WebVar_el.set_val(new_val);\n            WebVar_el.set_valid(this.is_valid());\n        }\n        this.#modified = true;\n    }\n    \n    #begun = false;\n    get begun() { return this.#begun; }\n    \n    constructor(name) {\n        if (name === undefined) throw \'Must define WebVar name\';\n        \n        this.#name = name.toString();\n    }\n    \n    add_el(new_el) {\n        this.#WebVar_elements.push(new WebVarElement(new_el));\n    }\n    \n    begin(WV_line) {\n        \n        // [T][H][R][W][name]:[value]\n        let name_and_val = WV_line.substring(4);\n        \n        let name = name_and_val.split(\':\')[0];\n        if (name !== this.#name)\n            throw \'The WebVaribale line does not match the name\';\n        try {\n            this.#val = name_and_val.split(\':\')[1];\n        }\n        catch { }\n        \n        this.#type   =   WebVar.Type.check_valid(WV_line.charAt(0));\n        this.#hidden = WebVar.Hidden.check_valid(WV_line.charAt(1));\n        this.#read   =     WebVar.RW.check_valid(WV_line.charAt(2));\n        this.#write  =     WebVar.RW.check_valid(WV_line.charAt(3));\n        \n        for (const WebVar_el of this.#WebVar_elements) {\n            WebVar_el.initial_conf(this.#type,\n                                   this.#hidden,\n                                   this.#read,\n                                   this.#write,\n                                   this.#val);\n        }\n        \n        this.#begun = true;\n        this.#modified = false;\n    }\n    \n    no_var() {\n        for (const WebVar_el of this.#WebVar_elements) {\n            WebVar_el.no_var();\n        }\n    }\n};\n\nconst WebVars = {\n    esp_ip: \'192.168.4.1\',\n    get_url: function (full) {\n        if (full === true)\n            return `http://${WebVars.esp_ip}${WebVars.get_url()}`;\n        else\n            return `/WVget`;\n    },\n    set_url: function (full) {\n        if (full === true)\n            return `http://${WebVars.esp_ip}${WebVars.set_url()}`;\n        else\n            return `/WVset`;\n    },\n    \n    generate_arg_list: function (WV_names) {\n        // add equal to every name and ampersand between\n        return WV_names.map(i => i + \"=\").join(\'&\');\n    },\n    \n    generate_GET_url: function (address, WV_names) {\n        return \'http://\' + address + \'/WVget?\' + WV_names.join(\'&\');\n    },\n    \n    name_from: {\n        el(el) {\n            return el.getAttribute(\'data-webvar\');\n        },\n        WV_line(str) {\n            return str.substring(4).split(\':\')[0];\n        }\n    },\n    \n    get_all_el: function() {\n        return Array.\n               prototype.\n               slice.\n               call( document.querySelectorAll(\'*[data-webvar]\') );\n    },\n    \n    css_added: false,\n    generate_css: function() {\n        if (this.css_added === true) return;\n        let styles = `\n            [class*=\"wv-\"] {\n                border-style: solid;\n                border-width: 1px;\n                outline: none;\n            }\n            \n            [class*=\"wv-\"][data-valid=\"false\"] {\n                /* offset-x | offset-y | blur-radius | spread-radius | color */\n                box-shadow: 0px 0px 3px 1px red;\n            }\n            \n            .wv-w-pri { color: red;    background-size: 7px 7px; background-image: repeating-linear-gradient(-45deg, rgba(255,0,0, 0.2) 0, rgba(255,0,0, 0.2) 1px, white 0, white 50%);}\n            .wv-w-pro { color: orange; }\n            .wv-w-pub { color: green;  }\n            \n            .wv-r-pri { border-color: red;    }\n            .wv-r-pro { border-color: orange; }\n            .wv-r-pub { border-color: green;  }\n                \n            .wv-hdn {}\n            \n            .wv-invalid { border-color: gray; }\n        `;\n\n        let style_sheet = document.createElement(\"style\");\n        style_sheet.type = \"text/css\";\n        style_sheet.innerHTML = styles;\n        document.head.prepend(style_sheet); // add at begining in order to be overwritable\n        \n        this.css_added = true;\n    },\n    \n    WV: new Map(), // main list,\n                   //contains the names of WebVars & elements for that name\n    \n    load_from_response: function(responseText) {\n        // for each line from the response find the apropriate WebVar\n        // and begin it with the line\n        for (const WV_line of responseText.split(\'\\n\')) {\n            if (WV_line != \'\') {\n                let name = WebVars.name_from.WV_line(WV_line);\n                WebVars.WV.get(name).begin(WV_line);\n            }\n        }\n        \n        for (const [webvar_name, webvar] of WebVars.WV) {\n            if (!webvar.begun)\n                webvar.no_var();\n        }\n    },\n    \n    password: undefined,\n    \n    begin: function (generate_css = true, prompt_pass = false) {\n        \n        if (generate_css === true)\n            this.generate_css();\n        \n        // populate WV with the available WebVars and elements\n        for (const el of WebVars.get_all_el()) {\n            let name = WebVars.name_from.el(el);\n            \n            if (!WebVars.WV.has(name)) {\n                WebVars.WV.set(name, new WebVar(name));\n                WebVars.WV.get(name).add_el(el);\n            }\n            else {\n                WebVars.WV.get(name).add_el(el);\n            }\n        }\n        \n        let password = \'\';\n        if (prompt_pass === true && this.password === undefined) {\n            password = prompt(\n                \'Some fields might require a password to read.\\n\' +\n                \'Enter password to be able to read them.\\n\' +\n                \'Or press cancel to ignore the password protected fields\');\n            if (password !== undefined)\n                this.password = password;\n            else\n                password = \'\';\n        }\n        \n        let xhr = new XMLHttpRequest();\n        xhr.open(\'POST\', WebVars.get_url(true), true);\n        xhr.setRequestHeader(\'Content-Type\', \'application/x-www-form-urlencoded\');\n        xhr.onreadystatechange = function () {\n            if (this.status == 200 && this.readyState == 4) {\n                console.log(this.responseText);\n                WebVars.load_from_response(this.responseText);\n            }\n        };\n        let password_arg = \'\';\n        if (this.password !== undefined)\n            password_arg = `&_password=${this.password}`;\n        xhr.send( WebVars.generate_arg_list(Array.from(WebVars.WV.keys())) + password_arg );\n    },\n    \n    save: function() {\n        let var_vals = [];\n        \n        let pass_required = false;\n        for (const [webvar_name, webvar] of WebVars.WV) {\n            if (webvar.modified) {\n                if (!webvar.is_valid()) {\n                    alert(`Variable \'${webvar_name}\' is not valid!\\n` +\n                          \'Make sure all fields have valid values to save.\');\n                    return;\n                }\n                if (webvar.write === WebVar.RW.PRO)\n                    pass_required = true;\n                \n                var_vals.push(`${webvar_name}=${webvar.val}`);\n            }\n        }\n        if (pass_required === true && this.password === undefined) {\n            let password = prompt(\n                \'Some fields require a password to save.\\n\' +\n                \'Enter password to be able to save them.\\n\' +\n                \'Or press cancel to skip the password protected fields\');\n            if (password !== undefined && password !== \'\') {\n                var_vals.push(`_password=${password}`);\n            }\n        }\n        \n        if (var_vals.length !== 0) {\n            query_str = var_vals.join(\'&\');\n            \n            console.log(query_str);\n            \n            let xhr = new XMLHttpRequest();\n            xhr.onreadystatechange = function () {\n                if (this.status == 200 && this.readyState == 4) {\n                    WebVars.load_from_response(this.responseText);\n                }\n            };\n            xhr.open(\'POST\', WebVars.set_url(true), true);\n            xhr.setRequestHeader(\'Content-Type\', \'application/x-www-form-urlencoded\');\n            let password_arg = \'\';\n            if (this.password !== undefined)\n                password_arg = `&_password=${this.password}`;\n            xhr.send(query_str + password_arg);\n        }\n    }\n};\n\ntry {\n    if (WEBVARS_AUTOLOAD === true)\n        window.addEventListener(\"load\", WebVars_begin);\n} catch { }\n");
        });
        
    }
};
// █
// ██ Static
// █
const char* WebVars::_password = nullptr;
const char* WebVars::password_param_name = "_password";
int WebVars::_web_variables_len = 0;
WebVars* WebVars::_web_variables[MAX_WEB_VARIABLES] = {nullptr};
Preferences* WebVars::_preferences = nullptr;

/*
░█▀▀░▀█▀░█▀▄░▀█▀░█▀█░█▀▀
░▀▀█░░█░░█▀▄░░█░░█░█░█░█
░▀▀▀░░▀░░▀░▀░▀▀▀░▀░▀░▀▀▀
*/
template <size_t buff_size=0>
class WebVarStr : public WebVars {
  private:
  
    char _str[buff_size];
    
    
  protected:
  
    char _type_char() { return 's'; }
    
    
  public:
    int len() {
        return min(buff_size, strlen(_str));
    }
    
    WebVarStr(const char* name, const char* intial_val, WebVarProtectionLevel read_pl=PUBLIC, WebVarProtectionLevel write_pl=PRIVATE, bool save_to_preferences=false)
    :WebVars(this, read_pl, write_pl, save_to_preferences) {
        strncpy(_str, intial_val, buff_size);
        _name = name;
    }
    
    operator const char*() const {
        return (const char*)_str;
    }
    /*
    void operator = (const WebVariable& wv ) { 
        strncpy(_str, wv._str, buff_size);
    }
    */
    void val_str(char* out) {
        sprintf(out, "%s", _str);
    }
    
    void str_to_val(const char* in) {
        
        bool change = (strcmp(in, _str) != 0);
        
        strncpy(_str, in, buff_size);
        
        if (change)
            _change(); 
    }
};



/*
░█▀▀░█▀▀░█▀█░█▀▀░█▀▄░▀█▀░█▀▀
░█░█░█▀▀░█░█░█▀▀░█▀▄░░█░░█░░
░▀▀▀░▀▀▀░▀░▀░▀▀▀░▀░▀░▀▀▀░▀▀▀
*/
template <typename _t>
class WebVarGeneric : public WebVars {
  protected:
    _t _val;
    
    WebVarGeneric(const char* name, _t intial_val, WebVarProtectionLevel read_pl=PUBLIC, WebVarProtectionLevel write_pl=PRIVATE, bool save_to_preferences=false)
    :WebVars(this, read_pl, write_pl, save_to_preferences) {
        _val = intial_val;
        _name = name;
    }
    
    virtual char _type_char() = 0;
    
    
  public:
    
    operator _t() const {
        return _val;
    }
    
    void operator = (const _t& val) { //NOTE// use this operator to trigger change detection
        bool change = (val != _val);
        
        _val = val;
        
        if (change)
            _change();
    }
    
    virtual void val_str(char* out) = 0;
    
    virtual void str_to_val(const char* in) = 0;
};



/*
░█▀▄░█▀█░█▀█░█░░
░█▀▄░█░█░█░█░█░░
░▀▀░░▀▀▀░▀▀▀░▀▀▀
*/
class WebVarBool : public WebVarGeneric<bool> {
    
  protected:
  
    char _type_char() { return 'b'; }
    
    
  public:
    WebVarBool(const char* name, bool intial_val, WebVarProtectionLevel read_pl=PUBLIC, WebVarProtectionLevel write_pl=PRIVATE, bool save_to_preferences=false)
    :WebVarGeneric(name, intial_val, read_pl, write_pl, save_to_preferences) {}
    
    using WebVarGeneric<bool>::operator=;
    
    void val_str(char* out) {
        if (_val)
            strcpy(out, "true");
        else
            strcpy(out, "false");
    }
    
    void str_to_val(const char* in) {
        if (strcmp(in, "true") == 0)
            (*this) = true;
        if (strcmp(in, "false") == 0)
            (*this) = false;
    }
};

/*
░▀█▀░█▀█░▀█▀
░░█░░█░█░░█░
░▀▀▀░▀░▀░░▀░
*/
class WebVarInt : public WebVarGeneric<int> {
    
  protected:
  
    char _type_char() { return 'i'; }
    
    
  public:
    WebVarInt(const char* name, int intial_val, WebVarProtectionLevel read_pl=PUBLIC, WebVarProtectionLevel write_pl=PRIVATE, bool save_to_preferences=false)
    :WebVarGeneric(name, intial_val, read_pl, write_pl, save_to_preferences) {}
    
    using WebVarGeneric<int>::operator=;
    
    void val_str(char* out) {
        sprintf(out, "%d", _val);
    }
    
    void str_to_val(const char* in) {
        bool only_numeric = true;
        for (int i=0; i<strlen(in); i++) {
            if (!(isdigit(in[i]) || in[i] == '+' || in[i] == '-')) {
                only_numeric = false;
                break;
            }
        }
        
        if (only_numeric)
            (*this) = atoi(in);
    }
};

/*
░█▀▄░█▀▀░█▀▀░▀█▀░█▄█░█▀█░█░░
░█░█░█▀▀░█░░░░█░░█░█░█▀█░█░░
░▀▀░░▀▀▀░▀▀▀░▀▀▀░▀░▀░▀░▀░▀▀▀
*/
class WebVarDec : public WebVarGeneric<double> {
    
  protected:
  
    char _type_char() { return 'd'; }
    
    
  public:
    WebVarDec(const char* name, double intial_val, WebVarProtectionLevel read_pl=PUBLIC, WebVarProtectionLevel write_pl=PRIVATE, bool save_to_preferences=false)
    :WebVarGeneric(name, intial_val, read_pl, write_pl, save_to_preferences) {}
    
    using WebVarGeneric<double>::operator=;
    
    void val_str(char* out) {
        sprintf(out, "%.2f", _val);
    }
    
    void str_to_val(const char* in) {
        bool only_numeric = true;
        for (int i=0; i<strlen(in); i++) {
            if (!(isdigit(in[i]) || in[i] == '+' || in[i] == '-' || in[i] == '.')) {
                only_numeric = false;
                break;
            }
        }
        
        if (only_numeric)
            (*this) = atof(in);
    }
};







#define  WEB_VAR_STR(name, len, iv) WebVarStr<len> name = WebVarStr<len>(#name, iv)
#define WEB_VAR_BOOL(name, iv)      WebVarBool     name = WebVarBool(#name, iv)
#define  WEB_VAR_INT(name, iv)      WebVarInt      name = WebVarInt(#name, iv)
#define  WEB_VAR_DEC(name, iv)      WebVarDec      name = WebVarDec(#name, iv)

#define  WEB_VAR_STR_S(name, len, iv) WebVarStr<len> name = WebVarStr<len>(#name, iv, PUBLIC, PRIVATE, USE_PREFERENCES)
#define WEB_VAR_BOOL_S(name, iv)      WebVarBool     name = WebVarBool(#name, iv, PUBLIC, PRIVATE, USE_PREFERENCES)
#define  WEB_VAR_INT_S(name, iv)      WebVarInt      name = WebVarInt(#name, iv, PUBLIC, PRIVATE, USE_PREFERENCES)
#define  WEB_VAR_DEC_S(name, iv)      WebVarDec      name = WebVarDec(#name, iv, PUBLIC, PRIVATE, USE_PREFERENCES)


