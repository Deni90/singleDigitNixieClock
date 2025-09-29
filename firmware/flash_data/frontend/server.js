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
    constructor(tzZone, tzOffset, timeFormat) {
        this.tzZone = tzZone;
        this.tzOffset = tzOffset
        this.timeFormat = timeFormat;
    }
    toJson() {
        return {
            "tz_zone": this.tzZone,
            "tz_offset": this.tzOffset,
            "time_format": this.timeFormat
        };
    }
    static Builder = class {
        fromJson(message) {
            return new TimeInfo(message.tz_zone, message.tz_offset, message.time_format);
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
    constructor(hostname, ssid, authType, password) {
        this.hostname = hostname
        this.ssid = ssid;
        this.authType = authType;
        this.password = password;
    }
    toJson() {
        return {
            "hostname": this.hostname,
            "SSID": this.ssid,
            "auth_type": this.authType,
            "password": this.password,
        };
    }
    static Builder = class {
        fromJson(message) {
            return new WifiInfo(message.hostname, message.SSID, message.auth_type, atob(message.password));
        }
    }
}

function HSVtoRGB(h, s, v) {
    let r, g, b, i, f, p, q, t;
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
    let max = Math.max(r, g, b), min = Math.min(r, g, b),
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
    let tabcontent = document.getElementById(id).children;
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.width = 100.0 / tabcontent.length + "%";
    }
}

function openTab(id, tabName) {
    let i, tabcontent, tablinks;
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

function openMainTab(id, tabName) {
    if (tabName == "tabClock") {
        getSleepInfo();
        getTimeZonesAndTimeInfo();
    } else if (tabName == "tabBacklight") {
        getLedInfo();
        updateColorBox();
    } else if (tabName == "tabWifi") {
        getWifiInfo();
    }
    openTab(id, tabName)
}

window.addEventListener('load', function () {
    setEqualTabButtonWidth("mainTab");
    openMainTab("buttonClock", "tabClock");
});

function getTimeZonesAndTimeInfo() {
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
}

function getTimeInfo() {
    fetch('/api/v1/clock/time_info')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            let timeInfo = new TimeInfo.Builder().fromJson(data);
            const timeZoneDropdown = document.getElementById('selectTimeZone');
            for (let i = 0; i < timeZoneDropdown.options.length; i++) {
                if (timeZoneDropdown.options[i].textContent === timeInfo.tzZone) {
                    timeZoneDropdown.selectedIndex = i;
                }
            }
            selectRadioChoiceByName("timeFormatChoice", timeInfo.timeFormat);
        })
        .catch(error => {
            console.error('There was a problem with the getting time info:', error);
        });
}

function setTimeInfo() {
    const timeZoneDropdown = document.getElementById('selectTimeZone');
    const selectedTimeZone = timeZoneDropdown.options[timeZoneDropdown.selectedIndex];
    let timeInfo = new TimeInfo(selectedTimeZone.text, selectedTimeZone.value, getSelectedRadioChoice("timeFormatChoice"));
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
            let sleepInfo = new SleepInfo.Builder().fromJson(data);
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
    let sleepInfo = new SleepInfo(
        document.getElementById("inputSleepBeforeHour").value * 60 + parseInt(document.getElementById("inputSleepBeforeMinute").value),
        document.getElementById("inputSleepAfterHour").value * 60 + parseInt(document.getElementById("inputSleepAfterMinute").value));
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
            let ledInfo = new LedInfo.Builder().fromJson(data);
            selectRadioChoiceByName("backlightTypeChoice", ledInfo.state);
            let hsv = RGBtoHSV(ledInfo.r, ledInfo.g, ledInfo.b);
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
    let h = (360 - document.getElementById("sliderHue").value) / 360;
    let s = document.getElementById("sliderSaturation").value / 100;
    let v = document.getElementById("sliderValue").value / 100;
    let rgb = HSVtoRGB(h, s, v);
    let ledInfo = new LedInfo(rgb.r, rgb.b, rgb.g, getSelectedRadioChoice("backlightTypeChoice"));
    const requestOptions = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(ledInfo.toJson())
    };
    fetch("/api/v1/led/led_info", requestOptions)
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
            let wifiInfo = new WifiInfo.Builder().fromJson(data);
            document.getElementById("hostname").value = wifiInfo.hostname;
            document.getElementById("ssid").value = wifiInfo.ssid;
            selectRadioChoiceByName("authTypeChoice", wifiInfo.authType);
            if (wifiInfo.authType == "open") {
                wifiInfo.password = "";
                document.getElementById("password").disabled = true;
                document.getElementById("passwordVerify").disabled = true;
            } else {
                document.getElementById("password").disabled = false;
                document.getElementById("passwordVerify").disabled = false;
            }
            document.getElementById("password").value = wifiInfo.password;
            document.getElementById("passwordVerify").value = wifiInfo.password;
        })
        .catch(error => {
            console.error('There was a problem with the getting wifi info:', error);
        });
}

function setWifiInfo() {
    let logLabel = document.getElementById("wifiLog");
    let errorColor = "#ff6457"
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
    if (getSelectedRadioChoice("authTypeChoice") != "open") {
        if (document.getElementById("password").value == "") {
            console.log("Wifi password cannot be empty");
            logLabel.innerHTML = "Error: Wifi password cannot be empty";
            logLabel.style.color = errorColor;
            return;
        }
        if (document.getElementById("password").value.length < 8) {
            console.log("Use at least 8 characters for password");
            logLabel.innerHTML = "Error: Use at least 8 characters for password";
            logLabel.style.color = errorColor;
            return;
        }
        if (document.getElementById("password").value != document.getElementById("passwordVerify").value) {
            console.log("Wifi passwords are not matching");
            logLabel.innerHTML = "Error: Passwords are not matching";
            logLabel.style.color = errorColor;
            return;
        }
    }
    logLabel.innerHTML = "";
    let wifiInfo = new WifiInfo(
        document.getElementById("hostname").value,
        document.getElementById("ssid").value,
        getSelectedRadioChoice("authTypeChoice"),
        btoa(document.getElementById("password").value));
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
        } else {
            // clear if any choice was active previously
            choices[i].className = choices[i].className.replace(" active", "");
        }
    }
}

function getSelectedRadioChoice(name) {
    let choices = document.getElementsByName(name);
    for (i = 0; i < choices.length; i++) {
        if (choices[i].className.includes(" active")) {
            return choices[i].getAttribute("value")
        }
    }
    return "";
}

function selectRadioChoiceByEvent(evt) {
    let i, choices;
    choices = document.getElementsByName(evt.currentTarget.getAttribute("name"));
    for (i = 0; i < choices.length; i++) {
        choices[i].className = choices[i].className.replace(" active", "");
    }
    evt.currentTarget.className += " active";
    return evt.currentTarget.getAttribute("value");
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
    let hsv = new HSV(0, 0, 0);
    hsv.h = document.getElementById("sliderHue").value;
    hsv.s = document.getElementById("sliderSaturation").value;
    hsv.v = document.getElementById("sliderValue").value;
    let color = "hsla(" + hsv.h + "," + hsv.s + "%," + "50%," + hsv.v + "%)";
    document.getElementById("colorBox").style.backgroundColor = color;
}

function handleAtuhTypeChoice(evt) {
    if (selectRadioChoiceByEvent(evt) == "open") {
        document.getElementById("password").disabled = true;
        document.getElementById("password").value = "";
        document.getElementById("passwordVerify").disabled = true;
        document.getElementById("passwordVerify").value = "";
    } else {
        document.getElementById("password").disabled = false;
        document.getElementById("passwordVerify").disabled = false;
    }
}