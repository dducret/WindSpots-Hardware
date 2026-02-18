<?php
// Improved WPA configuration generator.
// This script builds the wpa_supplicant configuration by reading SSID and passphrase info from a source file.

$wpaFile = '/etc/wpa_supplicant/wpa_supplicant.conf';
$wpaSourceFile = '/opt/windspots/etc/wpa';

// Base WPA configuration lines
$wpaConfigLines = [
    "country=CH",
    "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev",
    "update_config=1"
];

if (file_exists($wpaSourceFile)) {
    // Read source file without empty lines.
    $fileArray = file($wpaSourceFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    foreach ($fileArray as $line) {
        // Expecting lines in "SSID;passphrase" format.
        $parts = explode(';', trim($line));
        if (count($parts) >= 2) {
            // Sanitize the inputs for shell command.
            $ssid = escapeshellarg(trim($parts[0]));
            $passphrase = escapeshellarg(trim($parts[1]));
            // Generate WPA PSK using the system command.
            $wpaPSK = shell_exec("wpa_passphrase $ssid $passphrase");
            if ($wpaPSK !== null) {
                $wpaConfigLines[] = trim($wpaPSK);
            }
        }
    }
} else {
    error_log("WPA source file not found: $wpaSourceFile");
}

$configContent = implode(PHP_EOL, $wpaConfigLines) . PHP_EOL;

// Write the configuration directly to the file.
if (file_put_contents($wpaFile, $configContent) === false) {
    error_log("Failed to write WPA configuration to $wpaFile");
    echo "Error updating WPA configuration.";
} else {
    echo "WPA configuration updated successfully.";
}
?>
