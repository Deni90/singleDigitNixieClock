class Backlight {
    constructor(state, r, g, b) {
        this.state = state;
        this.r = r;
        this.g = g;
        this.b = b;
    }

    setColor(r, g, b) {
        this.r = r;
        this.g = g;
        this.b = b;
    }

    setBrightness(brightness) {
        this.a = brightness;
    }

    toJson() {
        return {
            "R": this.r,
            "G": this.g,
            "B": this.b,
            "state": this.state
        }
    }

    fromJson(message) {
        this.r = message.R;
        this.g = message.G;
        this.b = message.B;
        this.state = message.state;
    }
}

class HSV {
    constructor(h, s, v) {
        this.h = h;
        this.s = s;
        this.v = v;
    }

    print() {
        console.log("H:" + this.h + " S:" + this.s + " V:" + this.v);
    }

    toRGB() {
        return HSVtoRGB(this.h, this.s, this.v);
    }
}

let backlight = new Backlight(0, 0, 0, 0, 0);
let hsvColor = new HSV(0, 0, 0)

function HSVtoRGB(h, s, v) {
    var r, g, b, i, f, p, q, t;
    if (arguments.length === 1) {
        s = h.s, v = h.v, h = h.h;
    }
    i = Math.floor(h * 6);
    f = h * 6 - i;
    p = v * (1 - s);
    q = v * (1 - f * s);
    t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    return {
        r: Math.round(r * 255),
        g: Math.round(g * 255),
        b: Math.round(b * 255)
    };
}

function RGBtoHSV(r, g, b) {
    if (arguments.length === 1) {
        g = r.g, b = r.b, r = r.r;
    }
    var max = Math.max(r, g, b), min = Math.min(r, g, b),
        d = max - min,
        h,
        s = (max === 0 ? 0 : d / max),
        v = max / 255;

    switch (max) {
        case min: h = 0; break;
        case r: h = (g - b) + d * (g < b ? 6 : 0); h /= 6 * d; break;
        case g: h = (b - r) + d * 2; h /= 6 * d; break;
        case b: h = (r - g) + d * 4; h /= 6 * d; break;
    }

    return {
        h: h,
        s: s,
        v: v
    };
}

function showCurrentTime() {
    var lbl = document.getElementById("currentTime");
    var millisecondsToWait = 500;
    var interval = setInterval(function () {
        var now = new Date();
        lbl.innerHTML = now.getFullYear() + '-' + (now.getMonth() + 1) + '-' + now.getDate() + " " + now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds();
    }, millisecondsToWait);
}

function updateColorBox(hsvColor) {
    var color = "hsla(" + hsvColor.h * 360 + "," + hsvColor.s * 100 + "%," + "50%," + hsvColor.v + ")";
    $('#colorBox').css('background-color', color);
}

$(document).on('pagebeforecreate', '#index', function () {
    var interval = setInterval(function () {
        $.mobile.loading('show');
        clearInterval(interval);
    }, 1);
});

$(document).on('pageshow', '#index', function () {
    var interval = setInterval(function () {
        $.mobile.loading('hide');
        clearInterval(interval);
    }, 1);
});

$(document).ready(function () {
    console.log("page ready");

    $.ajax({ url: "/backlight", type: "GET", dataType: "json" })
        .success(function (result) {
            console.log(result);
            backlight.fromJson(result);

            var hsv = RGBtoHSV(backlight.r, backlight.g, backlight.b);
            hsvColor.h = hsv.h;
            hsvColor.s = hsv.s;
            hsvColor.v = hsv.v;
            $(function () {
                $('input:radio[name="backlightType"]').filter('[value="' + backlight.state + '"]').attr("checked", true).checkboxradio("refresh");
                $("input[name='hueSlider']").val(hsvColor.h * 360).slider('refresh');
                $("input[name='saturationSlider']").val(hsvColor.s * 100).slider('refresh');
                $("input[name='valueSlider']").val(hsvColor.v * 100).slider('refresh');
            });
        });

    $("input[name='backlightType']").on('change', function () {
        backlight.state = $(this).val();
    });

    $("input[name='hueSlider']").on('change', function () {
        hsvColor.h = $(this).val() / 360.0;
        updateColorBox(hsvColor);
    });

    $("input[name='saturationSlider']").on('change', function () {
        hsvColor.s = $(this).val() / 100.0;
        updateColorBox(hsvColor);
    });

    $("input[name='valueSlider']").on('change', function () {
        hsvColor.v = $(this).val() / 100.0;
        updateColorBox(hsvColor);
    });

    $("button[name='setBacklightButton'").on('click', function () {
        var rgbColor = hsvColor.toRGB();
        backlight.setColor(rgbColor.r, rgbColor.g, rgbColor.b);
        var jsonObj = backlight.toJson();
        $.ajax({ url: "/backlight", type: "POST", dataType: "json", data: jsonObj })
            .success(function (result) {
                console.log(result);
            });
    });

    $("button[name='setTimeButton'").on('click', function () {
        var now = new Date();
        var jsonObj = {
            "year": now.getFullYear(),
            "month": (now.getMonth() + 1),
            "day": now.getDate(),
            "hour": now.getHours(),
            "minute": now.getMinutes(),
            "second": now.getSeconds()
        }
        $.ajax({ url: "/clock/time", type: "POST", dataType: "json", data: jsonObj })
            .success(function (result) {
                console.log(result);
            });
    });

    showCurrentTime();
});