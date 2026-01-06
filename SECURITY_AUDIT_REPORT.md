# FiveM Server Security Audit Report

**Date:** 2026-01-06
**Scope:** Full codebase security audit
**Auditor:** Automated Security Analysis

---

## Executive Summary

This security audit identified **29 vulnerabilities** across the FiveM game server codebase. The findings range from critical remote code execution risks to informational security improvements.

| Severity | Count |
|----------|-------|
| **Critical** | 5 |
| **High** | 9 |
| **Medium** | 10 |
| **Low** | 5 |

**Key Concerns:**
1. Integer overflow vulnerabilities in network packet handling (RCE risk)
2. Weak cryptographic algorithms (SHA-1) in ticket verification
3. Hardcoded RSA private key for Rockstar services
4. Race conditions in authentication flow
5. Unbounded memory allocations enabling DoS attacks

---

## Critical Vulnerabilities

### CVE-FIVEM-001: Integer Multiplication Overflow in SerializableProperty

**File:** `code/components/net-base/include/SerializableProperty.h:238`
**CVSS Score:** 9.8 (Critical)
**Type:** CWE-190 Integer Overflow

**Description:**
When deserializing vector data from network packets, the multiplication `sizeof(typename Type::value_type) * size` can overflow when `size` comes from untrusted network data.

**Vulnerable Code:**
```cpp
m_value.resize(size);
if (!stream.Field(reinterpret_cast<Type&>(*m_value.data()), sizeof(typename Type::value_type) * size))
```

**Impact:** Remote code execution via heap buffer overflow

**Recommendation:**
```cpp
if (size > SIZE_MAX / sizeof(typename Type::value_type)) {
    return false; // Overflow would occur
}
```

---

### CVE-FIVEM-002: Unbounded Memory Allocation via Network Packets

**Files:**
- `code/components/net-packet/include/PackedClonesPacket.h:12`
- `code/components/net-packet/include/RpcNative.h:12`
- `code/components/net-packet/include/PackedAcksPacket.h:12`

**CVSS Score:** 7.5 (High)
**Type:** CWE-400 Uncontrolled Resource Consumption

**Description:**
Multiple packet types use `ConstrainedStreamTail<1, UINT32_MAX>` allowing allocations up to 4GB from network data.

**Vulnerable Code:**
```cpp
SerializableProperty<Span<uint8_t>, storage_type::ConstrainedStreamTail<1, UINT32_MAX>> data;
```

**Impact:** Denial of Service via memory exhaustion

**Recommendation:** Limit maximum allocation size to reasonable bounds (e.g., 16MB).

---

### CVE-FIVEM-003: Weak Cryptographic Hash (SHA-1) in Ticket Verification

**File:** `code/components/citizen-server-impl/src/InitConnectMethod.cpp:280-290, 339-346`
**CVSS Score:** 7.4 (High)
**Type:** CWE-328 Use of Weak Hash

**Description:**
The ticket verification system uses SHA-1, which is cryptographically broken and vulnerable to collision attacks.

**Vulnerable Code:**
```cpp
Botan::SHA_160 hashFunction;
auto signer = std::make_unique<Botan::PK_Verifier>(pk, "EMSA_PKCS1(SHA-1)");
```

**Impact:** Potential ticket forgery via SHA-1 collision attacks

**Recommendation:** Migrate to SHA-256 or SHA-512 immediately.

---

### CVE-FIVEM-004: Hardcoded RSA Private Key

**File:** `data/client/citizen/ros/ros.key`
**CVSS Score:** 9.1 (Critical)
**Type:** CWE-798 Hardcoded Credentials

**Description:**
A 2048-bit RSA private key and self-signed certificate for `*.ros.rockstargames.com` are committed to the repository.

**Impact:**
- Man-in-the-middle attacks on Rockstar services
- SSL/TLS bypass for ROS domains
- Trust chain compromise

**Recommendation:**
1. Remove key from repository
2. Clean git history using `git-filter-repo`
3. Rotate the key
4. Add `*.key` to `.gitignore`

---

### CVE-FIVEM-005: Integer Overflow in ResourceStreamComponent

**File:** `code/components/citizen-server-impl/src/ResourceStreamComponent.cpp:121-126, 501-506`
**CVSS Score:** 8.1 (High)
**Type:** CWE-190 Integer Overflow

**Description:**
The multiplication `entries.size() * sizeof(Entry)` can overflow when processing resource cache files.

**Vulnerable Code:**
```cpp
size_t numEntries = stream->GetLength() / sizeof(Entry);
std::vector<Entry> entries(numEntries);
stream->Read(&entries[0], entries.size() * sizeof(Entry));
```

**Impact:** Heap buffer overflow via malicious cache files

**Recommendation:** Add overflow check before multiplication.

---

## High Severity Vulnerabilities

### FIVEM-006: Race Condition in Public Key Caching

**File:** `code/components/citizen-server-impl/src/InitConnectMethod.cpp:82-169`
**Type:** CWE-362 Race Condition

**Description:**
TOCTOU vulnerability between checking key validity and using it during authentication.

**Recommendation:** Hold the lock during entire verification process.

---

### FIVEM-007: Connection Token Reuse Vulnerability

**File:** `code/components/citizen-server-impl/src/GameServerNet.ENet.cpp:278-316`
**Type:** CWE-384 Session Fixation

**Description:**
Connection tokens can be reused up to 3 times from the same IP, with no time-based expiration.

**Recommendation:** Implement single-use tokens with timestamp expiration.

---

### FIVEM-008: Integer Overflow in ByteReader Field Operations

**File:** `code/components/net-base/include/ByteReader.h:43, 97, 104`
**Type:** CWE-190 Integer Overflow

**Description:**
`m_offset + length` and `size * sizeof(T)` can overflow, bypassing bounds checks.

**Vulnerable Code:**
```cpp
bool CanRead(const size_t length) const {
    return m_offset + length <= m_capacity;  // Can overflow!
}
```

**Recommendation:**
```cpp
bool CanRead(const size_t length) const {
    if (length > m_capacity || m_offset > m_capacity - length)
        return false;
    return true;
}
```

---

### FIVEM-009: Integer Overflow in ByteWriter Operations

**File:** `code/components/net-base/include/ByteWriter.h:38, 92`
**Type:** CWE-190 Integer Overflow

**Description:** Same overflow pattern as ByteReader affecting write operations.

---

### FIVEM-010: NetBuffer Offset Overflow

**File:** `code/components/net-base/src/NetBuffer.cpp:118`
**Type:** CWE-190 Integer Overflow

**Description:**
```cpp
m_bytes->resize(m_curOff + length);  // Addition can overflow
```

---

### FIVEM-011: Missing Authorization Check on getEndpoints

**File:** `code/components/citizen-server-impl/src/InitConnectMethod.cpp:469-524`
**Type:** CWE-862 Missing Authorization

**Description:**
The `getEndpoints` handler doesn't verify client has passed validation, unlike other handlers.

**Recommendation:** Add `passedValidation` check.

---

### FIVEM-012: Ticket Replay Attack Window

**File:** `code/components/citizen-server-impl/src/InitConnectMethod.cpp:255-270`
**Type:** CWE-294 Authentication Bypass

**Description:**
Every 30 minutes, the entire ticket replay list is cleared, allowing ticket reuse.

```cpp
g_ticketList.clear();  // Creates replay window
g_nextTicketGc = msec() + std::chrono::minutes(30);
```

**Recommendation:** Use sliding window cleanup instead of clearing all tickets.

---

### FIVEM-013: Clone Processing Vector Allocation Overflow

**File:** `code/components/citizen-server-impl/src/state/ServerGameState.cpp:3364-3380`
**Type:** CWE-190 Integer Overflow

**Description:**
`bitBytes.size() * 8` multiplication used for ReadBits without overflow protection.

---

### FIVEM-014: Incomplete Client Reconnection Validation

**File:** `code/components/citizen-server-impl/src/InitConnectMethod.cpp:757-763`
**Type:** CWE-287 Improper Authentication

**Description:**
Old client is dropped without verifying the new connection owns the GUID, enabling DoS attacks.

**Recommendation:** Add challenge to prove ownership of GUID.

---

## Medium Severity Vulnerabilities

### FIVEM-015: Missing Null Termination in VoIP Server

**File:** `code/components/voip-server-mumble/src/sharedmemory.cpp:73-81`
**Type:** CWE-170 Improper Null Termination

**Description:**
`strncpy` calls don't guarantee null termination for maximum-length strings.

```cpp
strncpy(shmptr->client[cc].username, client_itr->username, 120);
// Missing: shmptr->client[cc].username[119] = '\0';
```

---

### FIVEM-016: Off-by-One Array Bounds Check

**File:** `code/components/citizen-server-impl/src/state/ServerGameState.cpp:3463`
**Type:** CWE-193 Off-by-one Error

**Description:**
Check allows `slotId == MAX_CLIENTS` when arrays are indexed 0 to MAX_CLIENTS-1.

```cpp
if (slotId < 0 || slotId > MAX_CLIENTS)  // Should be >= MAX_CLIENTS
```

---

### FIVEM-017: RCON Password Timing Attack

**File:** `code/components/citizen-server-impl/include/outofbandhandlers/RconOutOfBand.h:47-51`
**Type:** CWE-208 Observable Timing Discrepancy

**Description:**
Standard string comparison allows timing attacks to discover password.

**Recommendation:** Use constant-time comparison function.

---

### FIVEM-018: External Command Execution via Tebex

**File:** `code/components/citizen-server-impl/src/ServerExtCommerce.cpp:89-170`
**Type:** CWE-78 OS Command Injection

**Description:**
Commands from Tebex API are executed on server console. Compromised API or MITM could inject commands.

**Recommendation:** Implement command whitelist for Tebex-sourced commands.

---

### FIVEM-019: Session Fixation via ConnectionToken

**File:** `code/components/citizen-server-impl/src/InitConnectMethod.cpp:700, 772`
**Type:** CWE-384 Session Fixation

**Description:**
Token can be intercepted in HTTP response with no mutual authentication.

**Recommendation:** Use TLS for all HTTP connections, implement challenge-response.

---

### FIVEM-020: X-Real-Ip Header Trust Without Validation

**File:** `code/components/citizen-server-impl/src/EndPointIdentityProvider.cpp:67-78`
**Type:** CWE-290 Authentication Bypass by Spoofing

**Description:**
`X-Real-Ip` header accepted without validating request comes from trusted proxy.

---

### FIVEM-021: BitReader/BitWriter Bit Offset Overflow

**Files:**
- `code/components/net-base/include/BitReader.h:48`
- `code/components/net-base/include/BitWriter.h:50`

**Type:** CWE-190 Integer Overflow

**Description:** Potential overflow in `m_offset + bitSize` calculation.

---

### FIVEM-022: WorldGrid Pointer Arithmetic Overflow

**File:** `code/components/citizen-server-impl/src/state/ServerGameState.cpp:2227`
**Type:** CWE-190 Integer Overflow

**Description:**
Pointer subtraction followed by multiplication could produce incorrect base values.

---

### FIVEM-023: SerializableVector Unbounded Resize

**File:** `code/components/net-base/include/SerializableVector.h:88`
**Type:** CWE-400 Uncontrolled Resource Consumption

**Description:**
Vector resized based on network data without tight bounds in many uses.

---

### FIVEM-024: License Identity Provider Trusts Ticket Data

**File:** `code/components/citizen-server-impl/src/LicenseIdentityProvider.cpp:36-75`
**Type:** CWE-20 Improper Input Validation

**Description:**
Identifiers from ticket JSON added without format/count/length validation.

---

## Low Severity Vulnerabilities

### FIVEM-025: Use of Unsafe strcpy

**File:** `code/components/net-base/src/NetAddress.cpp:176`
**Type:** CWE-120 Buffer Overflow

**Description:** Uses `strcpy()` with constant source (low risk but bad practice).

---

### FIVEM-026: Predictable Temporary NetID Assignment

**File:** `code/components/citizen-server-impl/src/InitConnectMethod.cpp:767-774`
**Type:** CWE-330 Insufficient Randomness

**Description:** Temporary NetIDs are sequential, enabling client enumeration.

---

### FIVEM-027: Client Command Execution Rate Limits

**File:** `code/components/citizen-server-impl/src/packethandlers/ServerCommandPacketHandler.cpp:61-140`
**Type:** CWE-770 Resource Exhaustion

**Description:** While rate limited, consider additional logging of failed privilege checks.

---

### FIVEM-028: SDK/IPC Command Execution (Dev Only)

**File:** `code/components/citizen-server-fxdk/src/SdkIpc.cpp:401-405`
**Type:** CWE-78 Command Injection

**Description:** IPC commands executed with system.console privileges (FXDK mode only).

---

### FIVEM-029: Default Anonymous Token

**File:** `code/components/citizen-server-monitor/src/MonitorNucleus.cpp:45`
**Type:** CWE-1188 Insecure Default Initialization

**Description:** Default nucleus token is "anonymous" (overridden at runtime).

---

## Positive Security Practices

The codebase demonstrates several good security practices:

1. **Lua Sandboxing:** `os.execute()` always returns EACCES; `io.popen()` only allows dir/ls
2. **Format Strings:** All trace()/printf() calls use literal format strings
3. **Rate Limiting:** Multiple endpoints implement proper rate limiting
4. **Principal-Based ACL:** Granular permission system with `seCheckPrivilege()`
5. **Disabled Dangerous Functions:** `dofile()` and `loadfile()` set to nil in Lua runtime
6. **No SQL Usage:** File-based storage avoids SQL injection entirely
7. **Modern C++:** Use of std::string, smart pointers, and RAII patterns

---

## Recommendations Summary

### Immediate (Critical - Within 1 Week)
1. Add overflow checks to all size calculations in network parsing
2. Replace SHA-1 with SHA-256/SHA-512 in ticket verification
3. Remove RSA private key from repository and clean git history
4. Add maximum bounds to `ConstrainedStreamTail` allocations

### Short-term (High - Within 1 Month)
5. Fix all integer overflow vulnerabilities in ByteReader/ByteWriter
6. Implement single-use connection tokens with expiration
7. Fix race condition in public key caching
8. Add validation check to `getEndpoints` handler
9. Use sliding window for ticket replay prevention
10. Implement constant-time RCON password comparison

### Medium-term (Medium - Within 3 Months)
11. Add null termination after all strncpy calls
12. Fix off-by-one bounds check in ServerGameState
13. Validate X-Real-Ip header comes from trusted proxy
14. Add command whitelist for Tebex integration
15. Implement TLS for HTTP endpoints

### Long-term (Improvements)
16. Implement comprehensive audit logging
17. Add intrusion detection capabilities
18. Consider fuzzing network packet parsing code
19. Add pre-commit hooks to prevent secret commits

---

## Appendix: Files Requiring Attention

| File | Issues |
|------|--------|
| `InitConnectMethod.cpp` | CVE-003, FIVEM-006, 007, 011, 012, 014, 019, 026 |
| `SerializableProperty.h` | CVE-001 |
| `ByteReader.h` / `ByteWriter.h` | FIVEM-008, 009, 021 |
| `ServerGameState.cpp` | FIVEM-013, 016, 022 |
| `ResourceStreamComponent.cpp` | CVE-005 |
| `NetBuffer.cpp` | FIVEM-010 |
| `sharedmemory.cpp` | FIVEM-015 |
| `RconOutOfBand.h` | FIVEM-017 |
| `ServerExtCommerce.cpp` | FIVEM-018 |
| `ros.key` | CVE-004 |

---

**Report Complete**
