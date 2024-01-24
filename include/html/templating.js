function html_str_to_el(html) {
    let temp = document.createElement('template');
    html = html.trim(); // Never return a space text node as a result
    temp.innerHTML = html;
    return temp.content.firstChild;
}

function generate_templates() {
    // Generate templates
    var ungenerated = document.querySelectorAll('[data-template]');
    for (var i = 0; i < ungenerated.length; i++) {
        item = ungenerated.item(i);
        item.parentNode.replaceChild(html_str_to_el(window[item.dataset.template](JSON.parse(item.innerHTML))), item);
    }
}

try {
    if (TEMPLATING_AUTOLOAD === true)
        window.addEventListener("load", generate_templates);
} catch { }

/*
Example template definition and usage

<script>
    function math_template({x,y}) {
        return `${x} + ${y} = ${x+y}`;
    }
</script>

<div data-template="math_template">
    {"x":5, "y":4}
</div>
*/














