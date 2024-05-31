# Start time
$startTime = Get-Date
Write-Output "Build started at: $startTime"

# Run the build process
Write-Output "Building project..."
& 'C:\Program Files\JetBrains\CLion 2023.3.4\bin\cmake\win\x64\bin\cmake.exe' --build C:\Users\hmich\Documents\GitHub\zero_zero_beta\cmake-build-debug --target all -j 10

# End time
$endTime = Get-Date
Write-Output "Build finished at: $endTime"

# Calculate the duration
$duration = $endTime - $startTime
Write-Output "Total build time: $($duration.Hours) hours, $($duration.Minutes) minutes, $($duration.Seconds) seconds"
