class WebVarElement {
    
    constructor(el) {
        this.el = el;
    }
    
    set_valid(validity=false) {
        this.el.setAttribute('data-valid', (validity===true).toString());
    }
    
    set_val(new_val) {
        if (this.el.tagName.toLowerCase() === 'input')
            this.el.value = new_val;
        if (this.el.tagName.toLowerCase() === 'a')
            this.el.innerHTML = new_val;
    }
    
    initial_conf(type, hidden, read, write, val) {
        if (this.el.tagName.toLowerCase() === 'input') {
            
            let classes = [];
            
            // Read permisions
            if (read === WebVar.RW.PRI)
                classes.push('wv-r-pri');
            if (read === WebVar.RW.PRO)
                classes.push('wv-r-pro');
            if (read === WebVar.RW.PUB)
                classes.push('wv-r-pub');
            
            // Write permisions
            if (write === WebVar.RW.PRI)
                classes.push('wv-w-pri');
            if (write === WebVar.RW.PRO)
                classes.push('wv-w-pro');
            if (write === WebVar.RW.PUB)
                classes.push('wv-w-pub');
            
            // is hidden
            if (hidden === WebVar.Hidden.TRUE)
                classes.push('wv-hdn');
            
            let el = this.el;
            classes.forEach(function(clas){
                el.classList.add(clas);
            });
            
            
            if (hidden === WebVar.Hidden.TRUE) {
                let placeholder_txt = '';
                
                if (read === WebVar.RW.PRI && write === WebVar.RW.PUB)
                    placeholder_txt += 'Input value';
                if (read === WebVar.RW.PRI && write === WebVar.RW.PRO)
                    placeholder_txt += 'Input value (password required)';
                if (read === WebVar.RW.PRI && write === WebVar.RW.PRI)
                    placeholder_txt += 'Read/Write disabled';
                
                if (read === WebVar.RW.PRO && write === WebVar.RW.PUB)
                    placeholder_txt += 'Password required to read, Input value';
                if (read === WebVar.RW.PRO && write === WebVar.RW.PRO)
                    placeholder_txt += 'Password required to read, Input value (password required)';
                if (read === WebVar.RW.PRO && write === WebVar.RW.PRI)
                    placeholder_txt += 'Password required to read';
                
                this.el.setAttribute('placeholder', placeholder_txt);
            }
            
            
            var title_txt = 'Read: ';
            
            // Read permisions
            if (read === WebVar.RW.PRI)
                title_txt += 'no-one';
            if (read === WebVar.RW.PRO)
                title_txt += 'w/ password';
            if (read === WebVar.RW.PUB)
                title_txt += 'anyone';
            
            title_txt += ' / Write: ';
            
            // Write permisions
            if (write === WebVar.RW.PRI)
                title_txt += 'no-one';
            if (write === WebVar.RW.PRO)
                title_txt += 'w/ password';
            if (write === WebVar.RW.PUB)
                title_txt += 'anyone';
            
            this.el.setAttribute('title', title_txt);
            
            
            if (write === WebVar.RW.PRI)
                this.el.setAttribute('disabled', '');
            
            
            let new_val_fn = function() {
                if (this.getAttribute('type').toLowerCase() === 'checkbox')
                    this.value = this.checked;
                WebVars.WV.get(this.getAttribute('data-webvar')).val = this.value;
            };
            
            if (type === WebVar.Type.BOOLEAN) {
                this.el.setAttribute('type', 'checkbox');
                this.el.onchange = new_val_fn;
                if (val=='true')
                    this.el.setAttribute('checked', '');
            }
            else {
                this.el.setAttribute('type', 'text');
                this.el.onkeyup = new_val_fn;
            }
            
            this.set_val(val);
        }
        if (this.el.tagName.toLowerCase() === 'a') {
            if (hidden !== WebVar.Hidden.TRUE)
                this.el.innerHTML = val;
        }
    }
    
    no_var() {
        if (this.el.tagName.toLowerCase() === 'input') {
            this.el.classList.add('wv-invalid');
            this.el.setAttribute('type', 'text');
            this.el.value = 'WebVar \'' +
                            this.el.getAttribute('data-webvar') +
                            '\' does not exist';
            this.el.setAttribute('disabled', '');
        }
        if (this.el.tagName.toLowerCase() === 'a') {
            this.el.style.textDecoration = 'line-through';
        }
    }
}

class WebVar {
    
    static Type = {
        BOOLEAN : 'b',
        INTEGER : 'i',
        DECIMAL : 'd',
        STRING  : 's',
        
        check_valid: function (in_char) {
            // wtf have I done
            return WebVar.
                   Type[Object.keys(this).find(key => this[key] === in_char)];
        }
    };
    
    static Hidden = {
        TRUE  : 'h',
        FALSE : 'n',
        
        check_valid: function (in_char) {
            // wtf have I done
            return WebVar.
                   Hidden[Object.keys(this).find(key => this[key] === in_char)];
        }
    };
    
    static RW = {
        PRI : '-',
        PRO : '+',
        PUB : '*',
        
        check_valid: function (in_char) {
            // wtf have I done
            return WebVar.
                   RW[Object.keys(this).find(key => this[key] === in_char)];
        }
    };
    
    static get_regex(type) {
        switch(type) {
            case WebVar.Type.BOOLEAN:
                return /^true$|^false$/;
                break;
            case WebVar.Type.INTEGER:
                return /^[+-]?\d+$/;
                break;
            case WebVar.Type.DECIMAL:
                return /^[+-]?\d+(\.\d*)?$/;
                break;
            case WebVar.Type.STRING:
                return /^.*$/i;
                break;
            default:
                return /.*/i;// /(?!.*)/i;
        }
    }
    
    #type;
    get type() { return this.#type; }
    
    #hidden;
    get hidden() { return this.#hidden; }
    
    #read;
    get read() { return this.#read; }
    #write;
    get write() { return this.#write; }
    
    #name;
    get name() { return this.#name; }
    
    #modified = false;
    get modified() { return this.#modified; }
    
    #WebVar_elements = [];
    
    is_valid() {
        return WebVar.get_regex(this.type).test(this.#val) || this.#val === '';
    }
    
    #val = '';
    get val() { return this.#val; }
    set val(new_val) {
        new_val = new_val.toString();
        this.#val = new_val;
        for (const WebVar_el of this.#WebVar_elements) {
            WebVar_el.set_val(new_val);
            WebVar_el.set_valid(this.is_valid());
        }
        this.#modified = true;
    }
    
    #begun = false;
    get begun() { return this.#begun; }
    
    constructor(name) {
        if (name === undefined) throw 'Must define WebVar name';
        
        this.#name = name.toString();
    }
    
    add_el(new_el) {
        this.#WebVar_elements.push(new WebVarElement(new_el));
    }
    
    begin(WV_line) {
        
        // [T][H][R][W][name]:[value]
        let name_and_val = WV_line.substring(4);
        
        let name = name_and_val.split(':')[0];
        if (name !== this.#name)
            throw 'The WebVaribale line does not match the name';
        try {
            this.#val = name_and_val.split(':')[1];
        }
        catch { }
        
        this.#type   =   WebVar.Type.check_valid(WV_line.charAt(0));
        this.#hidden = WebVar.Hidden.check_valid(WV_line.charAt(1));
        this.#read   =     WebVar.RW.check_valid(WV_line.charAt(2));
        this.#write  =     WebVar.RW.check_valid(WV_line.charAt(3));
        
        for (const WebVar_el of this.#WebVar_elements) {
            WebVar_el.initial_conf(this.#type,
                                   this.#hidden,
                                   this.#read,
                                   this.#write,
                                   this.#val);
        }
        
        this.#begun = true;
        this.#modified = false;
    }
    
    no_var() {
        for (const WebVar_el of this.#WebVar_elements) {
            WebVar_el.no_var();
        }
    }
};

const WebVars = {
    esp_ip: '192.168.4.1',
    get_url: function (full) {
        if (full === true)
            return `http://${WebVars.esp_ip}${WebVars.get_url()}`;
        else
            return `/WVget`;
    },
    set_url: function (full) {
        if (full === true)
            return `http://${WebVars.esp_ip}${WebVars.set_url()}`;
        else
            return `/WVset`;
    },
    
    generate_arg_list: function (WV_names) {
        // add equal to every name and ampersand between
        return WV_names.map(i => i + "=").join('&');
    },
    
    generate_GET_url: function (address, WV_names) {
        return 'http://' + address + '/WVget?' + WV_names.join('&');
    },
    
    name_from: {
        el(el) {
            return el.getAttribute('data-webvar');
        },
        WV_line(str) {
            return str.substring(4).split(':')[0];
        }
    },
    
    get_all_el: function() {
        return Array.
               prototype.
               slice.
               call( document.querySelectorAll('*[data-webvar]') );
    },
    
    css_added: false,
    generate_css: function() {
        if (this.css_added === true) return;
        let styles = `
            [class*="wv-"] {
                border-style: solid;
                border-width: 1px;
                outline: none;
            }
            
            [class*="wv-"][data-valid="false"] {
                /* offset-x | offset-y | blur-radius | spread-radius | color */
                box-shadow: 0px 0px 3px 1px red;
            }
            
            .wv-w-pri { color: red;    background-size: 7px 7px; background-image: repeating-linear-gradient(-45deg, rgba(255,0,0, 0.2) 0, rgba(255,0,0, 0.2) 1px, white 0, white 50%);}
            .wv-w-pro { color: orange; }
            .wv-w-pub { color: green;  }
            
            .wv-r-pri { border-color: red;    }
            .wv-r-pro { border-color: orange; }
            .wv-r-pub { border-color: green;  }
                
            .wv-hdn {}
            
            .wv-invalid { border-color: gray; }
        `;

        let style_sheet = document.createElement("style");
        style_sheet.type = "text/css";
        style_sheet.innerHTML = styles;
        document.head.prepend(style_sheet); // add at begining in order to be overwritable
        
        this.css_added = true;
    },
    
    WV: new Map(), // main list,
                   //contains the names of WebVars & elements for that name
    
    load_from_response: function(responseText) {
        // for each line from the response find the apropriate WebVar
        // and begin it with the line
        for (const WV_line of responseText.split('\n')) {
            if (WV_line != '') {
                let name = WebVars.name_from.WV_line(WV_line);
                WebVars.WV.get(name).begin(WV_line);
            }
        }
        
        for (const [webvar_name, webvar] of WebVars.WV) {
            if (!webvar.begun)
                webvar.no_var();
        }
    },
    
    password: undefined,
    
    begin: function (generate_css = true, prompt_pass = false) {
        
        if (generate_css === true)
            this.generate_css();
        
        // populate WV with the available WebVars and elements
        for (const el of WebVars.get_all_el()) {
            let name = WebVars.name_from.el(el);
            
            if (!WebVars.WV.has(name)) {
                WebVars.WV.set(name, new WebVar(name));
                WebVars.WV.get(name).add_el(el);
            }
            else {
                WebVars.WV.get(name).add_el(el);
            }
        }
        
        let password = '';
        if (prompt_pass === true && this.password === undefined) {
            password = prompt(
                'Some fields might require a password to read.\n' +
                'Enter password to be able to read them.\n' +
                'Or press cancel to ignore the password protected fields');
            if (password !== undefined)
                this.password = password;
            else
                password = '';
        }
        
        let xhr = new XMLHttpRequest();
        xhr.open('POST', WebVars.get_url(true), true);
        xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
        xhr.onreadystatechange = function () {
            if (this.status == 200 && this.readyState == 4) {
                console.log(this.responseText);
                WebVars.load_from_response(this.responseText);
            }
        };
        let password_arg = '';
        if (this.password !== undefined)
            password_arg = `&_password=${this.password}`;
        xhr.send( WebVars.generate_arg_list(Array.from(WebVars.WV.keys())) + password_arg );
    },
    
    save: function() {
        let var_vals = [];
        
        let pass_required = false;
        for (const [webvar_name, webvar] of WebVars.WV) {
            if (webvar.modified) {
                if (!webvar.is_valid()) {
                    alert(`Variable '${webvar_name}' is not valid!\n` +
                          'Make sure all fields have valid values to save.');
                    return;
                }
                if (webvar.write === WebVar.RW.PRO)
                    pass_required = true;
                
                var_vals.push(`${webvar_name}=${webvar.val}`);
            }
        }
        if (pass_required === true && this.password === undefined) {
            let password = prompt(
                'Some fields require a password to save.\n' +
                'Enter password to be able to save them.\n' +
                'Or press cancel to skip the password protected fields');
            if (password !== undefined && password !== '') {
                var_vals.push(`_password=${password}`);
            }
        }
        
        if (var_vals.length !== 0) {
            query_str = var_vals.join('&');
            
            console.log(query_str);
            
            let xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (this.status == 200 && this.readyState == 4) {
                    WebVars.load_from_response(this.responseText);
                }
            };
            xhr.open('POST', WebVars.set_url(true), true);
            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            let password_arg = '';
            if (this.password !== undefined)
                password_arg = `&_password=${this.password}`;
            xhr.send(query_str + password_arg);
        }
    }
};

try {
    if (WEBVARS_AUTOLOAD === true)
        window.addEventListener("load", WebVars_begin);
} catch { }
