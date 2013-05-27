#!/bin/sh

defaults -currentHost write com.apple.screensaver idleTime 0

osascript -e 'tell application "System Events"
tell security preferences
set properties to {require password to wake:false} # true = on, false = off
end tell
end tell'

