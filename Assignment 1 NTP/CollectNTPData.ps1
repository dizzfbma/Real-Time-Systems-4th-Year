# Set project directory and output file
$projectFolder = "C:\Users\spinn\OneDrive\Documents\Real-Time Systems\Assignment 1 NTP"
$logFile = "$projectFolder\ntp_output.txt"

# Ensure the project folder exists
if (!(Test-Path $projectFolder)) {
    New-Item -ItemType Directory -Path $projectFolder | Out-Null
}

# Full path to Meinberg NTP ntpq.exe
$ntpqCommand = "C:\Program Files (x86)\NTP\bin\ntpq.exe"

# Function to restart NTP service
function Restart-NTP {
    Write-Host "Restarting Meinberg NTP service..."
    Stop-Service -Name ntp -Force
    Start-Service -Name ntp
    Start-Sleep -Seconds 5  # Wait for the service to start
}

# Restart the service before logging data
Restart-NTP

# Run for 8 hours (24 loops of 20 min intervals)
for ($i = 0; $i -lt 24; $i++) {
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "`n[$timestamp] Logging NTP data..."

    # Append timestamp to log file
    "`n[$timestamp] NTP Query Result:" | Out-File -FilePath $logFile -Append -Encoding UTF8

    # Run ntpq -p and capture output
    $ntpStatus = & $ntpqCommand -p 2>&1
    $ntpLines = $ntpStatus -split "`n"

    # Debugging: Log raw output to check format issues
    "Raw NTP Output:" | Out-File -FilePath $logFile -Append -Encoding UTF8
    $ntpLines | Out-File -FilePath $logFile -Append -Encoding UTF8

    # Extract values for each NTP server
    $loggedData = $false

    foreach ($line in $ntpLines) {
        # Match NTP server data, allowing jitter values of 0.000
        if ($line -match "^\s*([\*\+\-]?\S+)\s+([\S]+)\s+(\d+)\s+\S+\s+\S+\s+\S+\s+([\d\.]+)\s+([\d\.\-]+)\s+([\d\.]+)") {
            $server = $matches[1] -replace "^[\*\+\-]", ""  # Remove *, +, -
            $refid = $matches[2]  # Can be an IP or a text ref (e.g., `.PPS.`)
            $stratum = $matches[3]
            $delay = $matches[4]
            $offset = $matches[5]
            $jitter = if ($matches[6] -eq "0.000") { "No Jitter Reported" } else { $matches[6] }

            # Log structured data for each server
            "Server: $server | RefID: $refid | Stratum: $stratum | Delay: $delay ms | Offset: $offset ms | Jitter: $jitter ms" | Out-File -FilePath $logFile -Append -Encoding UTF8
            $loggedData = $true
        }
    }

    if (-not $loggedData) {
        Write-Host "Warning: No valid NTP data found! Logging raw output for debugging..." -ForegroundColor Yellow
    }

    Write-Host "NTP Data logged in $logFile"

    Write-Host "Waiting 20 minutes before next log..."
    Start-Sleep -Seconds 1200
}

Write-Host "NTP logging complete. Output is saved in $logFile" -ForegroundColor Cyan
