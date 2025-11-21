#!/system/bin/sh
# Kipepeo AI Magisk Module Installation Script

MODPATH=${0%/*}

ui_print "**********************************"
ui_print " Kipepeo AI System Hooks         "
ui_print " Version: 1.0.0                  "
ui_print "**********************************"
ui_print ""
ui_print "Installing system-level PLT hooks..."
ui_print ""

# Set permissions
ui_print "Setting permissions..."
set_perm_recursive $MODPATH 0 0 0755 0644
set_perm_recursive $MODPATH/system/lib64 0 0 0755 0644

# Check architecture
ABI=$(getprop ro.product.cpu.abi)
ui_print "Device ABI: $ABI"

if [ "$ABI" != "arm64-v8a" ] && [ "$ABI" != "armeabi-v7a" ]; then
    ui_print ""
    ui_print "[ERROR] Unsupported architecture: $ABI"
    ui_print "Kipepeo requires arm64-v8a or armeabi-v7a"
    exit 1
fi

ui_print ""
ui_print "Installation complete!"
ui_print ""
ui_print "Next steps:"
ui_print "1. Reboot your device"
ui_print "2. Install Kipepeo APK"
ui_print "3. Activate engine in the app"
ui_print ""
