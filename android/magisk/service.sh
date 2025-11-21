#!/system/bin/sh
# Kipepeo AI Service Script - runs on boot

MODDIR=${0%/*}

# Wait for boot to complete
until [ "$(getprop sys.boot_completed)" == 1 ]; do
    sleep 1
done

# Log startup
log -t Kipepeo "Kipepeo system hooks service starting..."

# Set SELinux permissive for Kipepeo library (required for PLT hooking)
# Note: This may trigger SafetyNet - use with caution
KIPEPEO_LIB="/system/lib64/libkipepeo.so"
if [ -f "$KIPEPEO_LIB" ]; then
    chcon u:object_r:system_lib_file:s0 $KIPEPEO_LIB
    log -t Kipepeo "Set SELinux context for libkipepeo.so"
else
    log -t Kipepeo "[WARN] libkipepeo.so not found in system/lib64"
fi

# Inject library into zygote for system-wide hooks (advanced)
# This is commented out by default - uncomment if you want aggressive hooking
# WARNING: May cause bootloops on some devices
# setprop persist.kipepeo.inject 1

log -t Kipepeo "Kipepeo system hooks service started successfully"
