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

class TimeInfo {
    constructor(offset) {
        this.offset = offset;
    }
    toJson() {
        return {
            "offset": this.offset
        };
    }
    static Builder = class {
        fromJson(message) {
            return new TimeInfo(message.offset);
        }
    }
}

class SleepInfo {
    constructor(sleepBefore, sleepAfter) {
        this.sleepBefore = sleepBefore;
        this.sleepAfter = sleepAfter;
    }
    toJson() {
        return {
            "sleep_before": this.sleepBefore,
            "sleep_after": this.sleepAfter,
        };
    }
    static Builder = class {
        fromJson(message) {
            return new SleepInfo(message.sleep_before, message.sleep_after);
        }
    }
}

class LedInfo {
    constructor(r, g, b, state) {
        this.r = r;
        this.g = g;
        this.b = b;
        this.state = state;
    }
    setColor(r, g, b) {
        this.r = r;
        this.g = g;
        this.b = b;
    }
    setState(state) {
        this.state = state;
    }
    toJson() {
        return {
            "R": this.r,
            "G": this.g,
            "B": this.b,
            "state": this.state
        }
    }
    static Builder = class {
        fromJson(message) {
            return new LedInfo(message.R, message.G, message.B, message.state);
        }
    }
}

class WifiInfo {
    constructor(ssid, password) {
        this.ssid = ssid;
        this.password = password;
    }
    toJson() {
        return {
            "SSID": this.ssid,
            "password": this.password,
        };
    }
    static Builder = class {
        fromJson(message) {
            return new WifiInfo(message.SSID, message.password);
        }
    }
}

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

function openTab(id, tabName) {
    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    document.getElementById(tabName).style.display = "block";
    document.getElementById(id).className += " active";
}

var timeInfo = null;
var sleepInfo = null;
var ledInfo = null;
var wifiInfo = null;

window.addEventListener('load', function () {
    getTimeInfo();
    getSleepInfo();
    getLedInfo();
    getWifiInfo();
    openTab("buttonClock", "tabClock");
    updateColorBox();
});

function getTimeInfo() {
    fetch('/clock/time_info')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            timeInfo = new TimeInfo.Builder().fromJson(data);
            document.getElementById("selectTimeZone").value = timeInfo.offset;
        })
        .catch(error => {
            console.error('There was a problem with the getting time info:', error);
        });
}

function setTimeInfo() {
    timeInfo.offset = document.getElementById("selectTimeZone").value;
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(timeInfo.toJson())
    };
    fetch("/clock/time_info", requestOptions)
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
        });
}

function getSleepInfo() {
    fetch('/clock/sleep_info')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            sleepInfo = new SleepInfo.Builder().fromJson(data);
            document.getElementById("sleepBeforeInput").value = sleepInfo.sleepBefore;
            document.getElementById("sliderSleepBefore").value = sleepInfo.sleepBefore;
            document.getElementById("sleepAfterInput").value = sleepInfo.sleepAfter;
            document.getElementById("sliderSleepAfter").value = sleepInfo.sleepAfter;
        })
        .catch(error => {
            console.error('There was a problem with the getting sleep info:', error);
        });
}

function setSleepInfo() {
    sleepInfo.sleepBefore = document.getElementById("sleepBeforeInput").value;
    sleepInfo.sleepAfter = document.getElementById("sleepAfterInput").value
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(sleepInfo.toJson())
    };
    fetch("/clock/sleep_info", requestOptions)
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
        });
}

function getLedInfo() {
    fetch('/backlight')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            ledInfo = new LedInfo.Builder().fromJson(data);
            choices = document.getElementsByClassName("choice");
            choices[ledInfo.state].className += " active";
            var hsv = RGBtoHSV(ledInfo.r, ledInfo.g, ledInfo.b);
            document.getElementById("sliderHue").value = hsv.h * 360;
            document.getElementById("sliderSaturation").value = hsv.s * 100;
            document.getElementById("sliderValue").value = hsv.v * 100;
            updateColorBox();
        })
        .catch(error => {
            console.error('There was a problem with the getting led info:', error);
        });
}

function setLedInfo() {
    var h = (360 - document.getElementById("sliderHue").value) / 360;
    var s = document.getElementById("sliderSaturation").value / 100;
    var v = document.getElementById("sliderValue").value / 100;
    var rgb = HSVtoRGB(h, s, v);
    ledInfo.setColor(rgb.r, rgb.b, rgb.g);
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(ledInfo.toJson())
    };
    fetch("/backlight", requestOptions)
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
        });
}

function getWifiInfo() {
    fetch('/wifi')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            wifiInfo = new WifiInfo.Builder().fromJson(data);
            document.getElementById("ssid").value = wifiInfo.ssid;
            document.getElementById("password").value = wifiInfo.password;
            document.getElementById("passwordVerify").value = wifiInfo.password;
        })
        .catch(error => {
            console.error('There was a problem with the getting wifi info:', error);
        });
}

function setWifiInfo() {
    var logLabel = document.getElementById("wifiLog");
    var errorColor = "#ff6457"
    if (document.getElementById("ssid").value == "") {
        console.log("Error: Wifi SSID cannot be empty");
        logLabel.innerHTML = "Error: Wifi SSID cannot be empty";
        logLabel.style.color = errorColor;
        return;
    }
    if (document.getElementById("password").value == "") {
        console.log("Wifi password cannot be empty");
        logLabel.innerHTML = "Error: Wifi password cannot be empty";
        logLabel.style.color = errorColor;
        return;
    }
    if (document.getElementById("password").value != document.getElementById("passwordVerify").value) {
        console.log("Wifi passwords are not matching");
        logLabel.innerHTML = "Error: Passwords are not matching";
        logLabel.style.color = errorColor;
        return;
    }
    logLabel.innerHTML = "";
    wifiInfo.ssid = document.getElementById("ssid").value;
    wifiInfo.password = btoa(document.getElementById("password").value);
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(wifiInfo.toJson())
    };
    fetch("/wifi", requestOptions)
        .then(response => {
            if (response.ok) {
                logLabel.innerHTML = "Restarting...";
                logLabel.style.color = "black";
            } else {
                throw new Error('Network response was not ok');
            }
        });
}

function selectBacklightChoice(evt, choiceName) {
    var i, choiceContent, choices;
    choices = document.getElementsByClassName("choice");
    for (i = 0; i < choices.length; i++) {
        choices[i].className = choices[i].className.replace(" active", "");
    }
    evt.currentTarget.className += " active";
    ledInfo.state = document.getElementById(choiceName).value;
}

function updateSleepBefore() {
    document.getElementById("sleepBeforeInput").value = document.getElementById("sliderSleepBefore").value
}

function updateSleepAfter() {
    document.getElementById("sleepAfterInput").value = document.getElementById("sliderSleepAfter").value
}

function setClockConfig() {
    setTimeInfo();
    setSleepInfo();
}

function updateColorBox() {
    var hsv = new HSV(0, 0, 0);
    hsv.h = document.getElementById("sliderHue").value;
    hsv.s = document.getElementById("sliderSaturation").value;
    hsv.v = document.getElementById("sliderValue").value;
    var color = "hsla(" + hsv.h + "," + hsv.s + "%," + "50%," + hsv.v + "%)";
    document.getElementById("colorBox").style.backgroundColor = color;
}