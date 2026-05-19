<?php
// Build wpa_supplicant.conf from /opt/windspots/etc/wpa.

$wpaFile = '/etc/wpa_supplicant/wpa_supplicant.conf';
$wpaSourceFile = '/opt/windspots/etc/wpa';
$wpaPassphraseBin = '/usr/bin/wpa_passphrase';
$errors = [];

function runWpaPassphrase($ssid, $passphrase, $wpaPassphraseBin) {
    $descriptorSpec = [
        0 => ['pipe', 'r'],
        1 => ['pipe', 'w'],
        2 => ['pipe', 'w'],
    ];

    $process = proc_open([$wpaPassphraseBin, $ssid, $passphrase], $descriptorSpec, $pipes);
    if (!is_resource($process)) {
        return false;
    }

    fclose($pipes[0]);
    $stdout = stream_get_contents($pipes[1]);
    $stderr = stream_get_contents($pipes[2]);
    fclose($pipes[1]);
    fclose($pipes[2]);

    $exitCode = proc_close($process);
    if ($exitCode !== 0) {
        error_log("wpa_passphrase failed for SSID '$ssid': " . trim((string) $stderr));
        return false;
    }

    return trim((string) $stdout);
}

// Base WPA configuration lines
$wpaConfigLines = [
    "country=CH",
    "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev",
    "update_config=1"
];

if (file_exists($wpaSourceFile)) {
    if (!is_executable($wpaPassphraseBin)) {
        $errors[] = "Missing executable: $wpaPassphraseBin";
    } else {
        // Read source file without empty lines.
        $fileArray = file($wpaSourceFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach ($fileArray as $line) {
            // Expecting lines in "SSID;passphrase" format.
            $parts = explode(';', trim($line), 2);
            if (count($parts) !== 2) {
                $errors[] = "Invalid WPA source line format.";
                continue;
            }

            $ssid = trim($parts[0]);
            $passphrase = trim($parts[1]);
            $passphraseLength = strlen($passphrase);

            if ($ssid === '' || strlen($ssid) > 32) {
                $errors[] = "Invalid SSID length.";
                continue;
            }

            if ($passphraseLength < 8 || $passphraseLength > 63) {
                $errors[] = "Invalid WPA passphrase length for SSID '$ssid'.";
                continue;
            }

            $wpaPSK = runWpaPassphrase($ssid, $passphrase, $wpaPassphraseBin);
            if (is_string($wpaPSK) && $wpaPSK !== '') {
                $wpaConfigLines[] = $wpaPSK;
            } else {
                $errors[] = "Unable to generate WPA block for SSID '$ssid'.";
            }
        }
    }
} else {
    $errors[] = "WPA source file not found: $wpaSourceFile";
}

if ($errors !== []) {
    foreach ($errors as $error) {
        error_log($error);
    }
    echo "Error updating WPA configuration.";
    exit(1);
}

$configContent = implode(PHP_EOL, $wpaConfigLines) . PHP_EOL;
$tmpFile = $wpaFile . '.tmp';

if (file_put_contents($tmpFile, $configContent, LOCK_EX) === false) {
    error_log("Failed to write temporary WPA configuration to $tmpFile");
    echo "Error updating WPA configuration.";
    exit(1);
}

chmod($tmpFile, 0660);
@chown($tmpFile, 'root');
@chgrp($tmpFile, 'www-data');

if (!rename($tmpFile, $wpaFile)) {
    @unlink($tmpFile);
    error_log("Failed to replace WPA configuration at $wpaFile");
    echo "Error updating WPA configuration.";
    exit(1);
}

echo "WPA configuration updated successfully.";
?>
