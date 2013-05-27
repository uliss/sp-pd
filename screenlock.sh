#!/bin/sh

defaults -currentHost write com.apple.screensaver idleTime 300
osascript -e 'tell application "System Events"
tell security preferences
set properties to {require password to wake:true} # true = on, false = off
end tell
end tell'

