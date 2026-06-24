$ErrorActionPreference = "Stop"

Write-Host "========================================"
Write-Host " EmergiCare Backend Connection Test"
Write-Host "========================================"

$BaseUrl = "http://localhost:3001/api"

# 1. Login to get JWT
Write-Host "Logging in as Admin..."
$LoginPayload = @{
    username = "admin"
    password = "Admin@123"
} | ConvertTo-Json

try {
    $LoginResponse = Invoke-RestMethod -Uri "$BaseUrl/login" -Method Post -Body $LoginPayload -ContentType "application/json"
    $Token = $LoginResponse.token
    Write-Host "[OK] Logged in successfully. Token received."
} catch {
    Write-Host "[X] FAILED: Could not login."
    Write-Host $_.Exception.Message
    exit
}

$Headers = @{
    Authorization = "Bearer $Token"
}

# 2. Test Users Endpoint
Write-Host "========================================"
Write-Host "Testing DB Connection via GET /api/users..."
try {
    $Users = Invoke-RestMethod -Uri "$BaseUrl/users" -Method Get -Headers $Headers
    Write-Host "[OK] Connection to PostgreSQL Successful!"
    Write-Host "Returned $($Users.Count) users."
} catch {
    Write-Host "[X] FAILED: Could not reach backend or DB error."
    Write-Host $_.Exception.Message
}

# 3. Test Emergency Queue Endpoint
Write-Host "========================================"
Write-Host "Testing Emergency Queue Endpoint..."
try {
    $Queue = Invoke-RestMethod -Uri "$BaseUrl/emergency-queue" -Method Get -Headers $Headers
    Write-Host "[OK] Emergency Queue Active."
} catch {
    Write-Host "[X] FAILED: Emergency Queue Endpoint."
    Write-Host $_.Exception.Message
}
