# setup.ps1

# Check if the script is running with administrative privileges
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    # Relaunch the script with administrative privileges
    Start-Process powershell -Verb RunAs -ArgumentList ("-NoProfile -ExecutionPolicy Bypass -File `"$($MyInvocation.MyCommand.Path)`"")
    exit
}

$projectName = "C_CPP_BASE"
# Define the path to the project folder
$projectFolderPath = "$PSScriptRoot"

# Check if project folder path is already in the system's PATH environment variable
$currentPath = [System.Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::Machine)
if ($currentPath -split ";" -notcontains $projectFolderPath) {
    # Add the project folder to the system's PATH environment variable
    $newPath = $currentPath + ";" + $projectFolderPath
    [System.Environment]::SetEnvironmentVariable("PATH", $newPath, [System.EnvironmentVariableTarget]::Machine)
    Write-Host "$projectName folder added to the system PATH."
} else {
    Write-Host "$projectName folder is already present in the system PATH."
}
