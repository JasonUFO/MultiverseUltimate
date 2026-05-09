#!/bin/bash
# fix_instrument_classification.sh
#
# Enforces correct instrument classification for a SYNTHESIZER plugin.
# Projucer --resave overwrites these values to "effect" defaults every time.
# This is a known Projucer bug — pluginIsSynth in .jucer does not propagate
# to Vst3Category, AUMainType, VSTCategory, or Info-AU.plist type field.
#
# Run this script:
#   1. After every Projucer --resave
#   2. It also runs as an Xcode build phase (pre-compile) for automatic enforcement
#
# Required values for a software INSTRUMENT (synth) plugin:
#   JucePluginDefines.h:
#     JucePlugin_IsSynth          = 1       (not 0)
#     JucePlugin_WantsMidiInput   = 1       (not 0)
#     JucePlugin_VSTCategory      = kPlugCategSynth   (not kPlugCategEffect)
#     JucePlugin_Vst3Category     = "Instrument"       (not "Fx")
#     JucePlugin_AUMainType       = 'aumu'              (not 'aufx')
#   Info-AU.plist:
#     type = aumu  (not aufx)

# Resolve project root
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DEFINES_FILE="$PROJECT_ROOT/JuceLibraryCode/JucePluginDefines.h"
AU_PLIST="$PROJECT_ROOT/Builds/MacOSX/Info-AU.plist"

# Also support Xcode build phase (SRCROOT is set by Xcode)
if [ -n "$SRCROOT" ] && [ ! -f "$DEFINES_FILE" ]; then
    DEFINES_FILE="$SRCROOT/../../JuceLibraryCode/JucePluginDefines.h"
    AU_PLIST="$SRCROOT/Info-AU.plist"
fi

CHANGED=0

# --- Fix JucePluginDefines.h ---
if [ -f "$DEFINES_FILE" ]; then
    # Remove carriage returns first (Projucer may add them)
    if grep -q $'\r' "$DEFINES_FILE" 2>/dev/null; then
        sed -i '' $'s/\r$//' "$DEFINES_FILE"
    fi

    # Patch a #define line by name
    patch_define() {
        local name="$1"
        local expected="$2"
        local current
        current=$(perl -ne "if (/#define\s+${name}\s+(.*)/) { print \$1 }" "$DEFINES_FILE")

        if [ -z "$current" ]; then
            echo "fix_instrument_classification: WARNING: ${name} not found in JucePluginDefines.h"
            return
        fi

        if [ "$current" != "$expected" ]; then
            echo "fix_instrument_classification: Patching ${name}: '${current}' -> '${expected}'"
            perl -pi -e "s/#define\s+${name}\s+\S+/#define  ${name}                ${expected}/" "$DEFINES_FILE"
            CHANGED=1
        fi
    }

    patch_define "JucePlugin_IsSynth"           "1"
    patch_define "JucePlugin_WantsMidiInput"    "1"
    patch_define "JucePlugin_VSTCategory"       "kPlugCategSynth"
    patch_define "JucePlugin_Vst3Category"      '"Instrument"'
    patch_define "JucePlugin_AUMainType"        "'aumu'"
else
    echo "fix_instrument_classification: WARNING: $DEFINES_FILE not found"
fi

# --- Fix Info-AU.plist (Projucer writes 'aufx' for type) ---
if [ -f "$AU_PLIST" ]; then
    AU_TYPE=$(perl -ne 'if (/<key>type<\/key>\s*<string>(.*?)<\/string>/) { print $1 }' "$AU_PLIST")
    if [ "$AU_TYPE" != "aumu" ]; then
        echo "fix_instrument_classification: Patching Info-AU.plist type: '${AU_TYPE}' -> 'aumu'"
        perl -pi -e 's|(<key>type</key>\s*<string>)aufx(</string>)|$1aumu$2|' "$AU_PLIST"
        CHANGED=1
    fi
else
    echo "fix_instrument_classification: WARNING: $AU_PLIST not found"
fi

if [ "$CHANGED" -eq 1 ]; then
    echo "fix_instrument_classification: Corrected to Instrument/Synth (was effect defaults)"
else
    echo "fix_instrument_classification: OK — all values already correct"
fi