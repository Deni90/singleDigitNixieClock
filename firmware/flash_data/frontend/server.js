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
    constructor(tzZone, tzOffset) {
        this.tzZone = tzZone;
        this.tzOffset = tzOffset
    }
    toJson() {
        return {
            "tz_zone": this.tzZone,
            "tz_offset": this.tzOffset
        };
    }
    static Builder = class {
        fromJson(message) {
            return new TimeInfo(message.tz_zone, message.tz_offset);
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
    constructor(hostname, ssid, password) {
        this.hostname = hostname
        this.ssid = ssid;
        this.password = password;
    }
    toJson() {
        return {
            "hostname": this.hostname,
            "SSID": this.ssid,
            "password": this.password,
        };
    }
    static Builder = class {
        fromJson(message) {
            return new WifiInfo(message.hostname, message.SSID, atob(message.password));
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

function setEqualTabButtonWidth(id) {
    var tabcontent = document.getElementById(id).children;
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.width = 100.0 / tabcontent.length + "%";
    }
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
    getSleepInfo();
    getLedInfo();
    getWifiInfo();
    updateColorBox();
    setEqualTabButtonWidth("mainTab");
    openTab("buttonClock", "tabClock");
});

document.addEventListener("DOMContentLoaded", () => {
    const timeZoneDropdown = document.getElementById("selectTimeZone");
    fetch("zones.json")
        .then(response => response.json())
        .then(data => {
            // data is an object { "Africa/Abidjan": "GMT0", ... }
            for (const [zone, offset] of Object.entries(data)) {
                const option = document.createElement("option");
                option.value = offset;
                option.textContent = zone;
                timeZoneDropdown.appendChild(option);
            }
            getTimeInfo();
        })
        .catch(error => {
            console.error("Error loading timezone JSON:", error);
        });
});

function getTimeInfo() {
    fetch('/api/v1/clock/time_info')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            timeInfo = new TimeInfo.Builder().fromJson(data);
            const timeZoneDropdown = document.getElementById('selectTimeZone');
            for (let i = 0; i < timeZoneDropdown.options.length; i++) {
                if (timeZoneDropdown.options[i].textContent === timeInfo.tzZone) {
                    timeZoneDropdown.selectedIndex = i;
                }
            }
        })
        .catch(error => {
            console.error('There was a problem with the getting time info:', error);
        });
}

function setTimeInfo() {
    const timeZoneDropdown = document.getElementById('selectTimeZone');
    const selectedTimeZone = timeZoneDropdown.options[timeZoneDropdown.selectedIndex];
    timeInfo.tzOZone = selectedTimeZone.text;
    timeInfo.tzOffset = selectedTimeZone.value;
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(timeInfo.toJson())
    };
    fetch("/api/v1/clock/time_info", requestOptions)
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
        });
}

function getSleepInfo() {
    fetch('/api/v1/clock/sleep_info')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            sleepInfo = new SleepInfo.Builder().fromJson(data);
            document.getElementById("inputSleepBeforeHour").value = Math.floor(sleepInfo.sleepBefore / 60);
            document.getElementById("sliderSleepBeforeHour").value = Math.floor(sleepInfo.sleepBefore / 60);
            document.getElementById("inputSleepBeforeMinute").value = sleepInfo.sleepBefore % 60;
            document.getElementById("sliderSleepBeforeMinute").value = sleepInfo.sleepBefore % 60;

            document.getElementById("inputSleepAfterHour").value = Math.floor(sleepInfo.sleepAfter / 60);
            document.getElementById("sliderSleepAfterHour").value = Math.floor(sleepInfo.sleepAfter / 60);
            document.getElementById("inputSleepAfterMinute").value = sleepInfo.sleepAfter % 60;
            document.getElementById("sliderSleepAfterMinute").value = sleepInfo.sleepAfter % 60;
        })
        .catch(error => {
            console.error('There was a problem with the getting sleep info:', error);
        });
}

function setSleepInfo() {
    sleepInfo.sleepBefore = document.getElementById("inputSleepBeforeHour").value * 60 + parseInt(document.getElementById("inputSleepBeforeMinute").value);
    sleepInfo.sleepAfter = document.getElementById("inputSleepAfterHour").value * 60 + parseInt(document.getElementById("inputSleepAfterMinute").value);
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(sleepInfo.toJson())
    };
    fetch("/api/v1/clock/sleep_info", requestOptions)
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
        });
}

function getLedInfo() {
    fetch('/api/v1/led/led_info')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            ledInfo = new LedInfo.Builder().fromJson(data);
            selectRadioChoiceByName("backlightTypeChoice", ledInfo.state);
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
    fetch("/api/v1/del/led_info", requestOptions)
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
        });
}

function getWifiInfo() {
    fetch('/api/v1/wifi/wifi_info')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            wifiInfo = new WifiInfo.Builder().fromJson(data);
            document.getElementById("hostname").value = wifiInfo.hostname;
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
    if (document.getElementById("hostname").value == "") {
        console.log("Error: Hostname cannot be empty");
        logLabel.innerHTML = "Error: Hostname cannot be empty";
        logLabel.style.color = errorColor;
        return;
    }
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
    wifiInfo.hostname = document.getElementById("hostname").value;
    wifiInfo.ssid = document.getElementById("ssid").value;
    wifiInfo.password = btoa(document.getElementById("password").value);
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(wifiInfo.toJson())
    };
    fetch("/api/v1/wifi/wifi_info", requestOptions)
        .then(response => {
            if (response.ok) {
                logLabel.innerHTML = "Restarting...";
                logLabel.style.color = "black";
            } else {
                throw new Error('Network response was not ok');
            }
        });
}

function selectRadioChoiceByName(name, value) {
    let choices = document.getElementsByName(name);
    for (i = 0; i < choices.length; i++) {
        if (choices[i].getAttribute("value") == value) {
            choices[i].className += " active";
            break;
        }
    }
}

function selectRadioChoiceByEvent(evt) {
    var i, choices;
    choices = document.getElementsByName(evt.currentTarget.getAttribute("name"));
    for (i = 0; i < choices.length; i++) {

        choices[i].className = choices[i].className.replace(" active", "");
    }
    evt.currentTarget.className += " active";
    return evt.currentTarget.getAttribute("value");
}

function selectBacklightChoice(evt) {
    ledInfo.state = selectRadioChoiceByEvent(evt);
}

function selectDstChoice(evt) {
    timeInfo.isDst = selectRadioChoiceByEvent(evt);
}

function updateSleepBefore() {
    document.getElementById("inputSleepBeforeHour").value = document.getElementById("sliderSleepBeforeHour").value
    document.getElementById("inputSleepBeforeMinute").value = document.getElementById("sliderSleepBeforeMinute").value
}

function updateSleepAfter() {
    document.getElementById("inputSleepAfterHour").value = document.getElementById("sliderSleepAfterHour").value
    document.getElementById("inputSleepAfterMinute").value = document.getElementById("sliderSleepAfterMinute").value
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