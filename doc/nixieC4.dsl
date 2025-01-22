workspace {

    model {
        user = person "User"

        nixieClock = softwareSystem "Single Digit Nixie Clock" "Shows current time on a single nixie tube every minute, while the backlight is updated every second" {
            configStore = container "Config Store" "Read/Write configuration files from file system"
            fileSystem = container "File System" "LittleFS" {
                tags "Database"
            }
            webApp = container "Web Application" "Nixie clock's control panel" {
                tags "WebBrowser"
            }
            webServer = container "Web Server" "AsyncWebServer that exposes REST API for getting/setting configuration parameters"
            timeManager = container "Time Manager" "Wrapper around the NTP client library"
            ledController = container "Led Controller" "Module used to control the RGB LED used as backlight. States can be off, on, fade and pulse"
            nixieTube = container "Nixie Tube" "BCD used to control the state of the nixie tube"
            rtc = container "Real Time Clock" "RtcDS3231 library"
            wifi = container "Wifi" "Setup for either AP or client modes"
            main = container "Nixie Clock" "TODO"
        }
        ntpServer = softwareSystem "NTP server" "Provides precise current time"

        // system context relationships
        user -> nixieClock "Checks current time on"
        user -> nixieClock "Configures"
        nixieClock -> ntpServer "Synchronizes time with"


        // software system relationships
        main -> configStore "Gets/Sets configuration parameters"
        main -> nixieTube "Shows current time on"
        main -> ledController "Controls LED state"
        main -> rtc "Gets current time from"
        main -> timeManager "Periodically synchronizes time"

        user -> webApp "Configures clock with"
        user -> nixieTube "Can see the current time every minute"
        user -> ledController "Enjoys the beauty of backlight :)"


        configStore -> fileSystem "Reads/Writes JSON files"

        timeManager -> ntpServer "Gets current time from"
        timeManager -> rtc "Updates time to"
        timeManager -> configStore "Gets TimeInfo configuration from"

        ledController -> configstore "Gets LedInfo configuration from"

        wifi -> configStore "Gets WifiInfo configuration from"

        webServer -> webApp "Hosts"
        webServer -> main "Calls callback functions via ClockInterface"

    }

    views {
        systemContext nixieClock "singleDigitNixieClockContextView" {
            include *
            autolayout
        }
        container nixieClock "singleDigitNixieClockSoftwareSystemView" {
            include *
            # autolayout
        }
        styles {
            element "Database" {
                shape Cylinder
            }
            element "WebBrowser" {
                shape WebBrowser
            }
        }
        theme default
    }

}