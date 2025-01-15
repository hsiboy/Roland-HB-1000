# Roland D50 MIDI Implementation Guide

## Address Map Overview

### Base Addresses (Top Address)
| Address     | Description    | Range       |
|------------|---------------|-------------|
| [00-00-00] | Upper Partial 1 | (0 - 53)    |
| [00-00-40] | Upper Partial 2 | (64 - 117)  |
| [00-01-00] | Upper Common   | (128 - 175) |
| [00-01-40] | Lower Partial 1 | (192 - 245) |
| [00-02-00] | Lower Partial 2 | (256 - 309) |
| [00-02-40] | Lower Common   | (320 - 367) |
| [00-03-00] | Patch          | (384 - 420) |

## Parameter Address Calculation
Parameter address = Base address + Offset

## Parameter Sections

### 1. Partial Parameters (WG Section)
| Offset | Function              | Value Range | Description                  |
|--------|----------------------|-------------|------------------------------|
| 0      | WG Pitch Coarse      | 0 - 72      | C1 - C7                     |
| 1      | WG Pitch Fine        | 0 - 100     | -50 - +50                   |
| 2      | WG Pitch Keyfollow   | 0 - 16      | -1, -1/2, -4/1, 0, 1/8, ... |
| 3      | WG Mod LFO Mode      | 0 - 3       | OFF, (+), (-), A&L          |
| 4      | WG Mod P-ENV Mode    | 0 - 2       | OFF, (+), (-)               |
| 5      | WG Mod Bender Mode   | 0 - 2       | OFF, KF, NORMAL             |
| 6      | WG Waveform          | 0 - 1       | SQU, SAW                    |
| 7      | WG PCM Wave No.      | 0 - 99      | 1 - 100                     |
| 8      | WG Pulse Width       | 0 - 100     | 0 - 100                     |
| 9      | WG PW Velocity Range | 0 - 14      | -7 - +7                     |

### 2. Partial Parameters (TVF Section)
| Offset | Function              | Value Range | Description                  |
|--------|----------------------|-------------|------------------------------|
| 13     | TVF Cutoff Frequency | 0 - 100     | 0 - 100                     |
| 14     | TVF Resonance        | 0 - 30      | 0 - 30                      |
| 15     | TVF Keyfollow        | 0 - 14      | -1, -1/2, -4/1, 0, ...      |
| 16     | TVF Bias Point/Dir   | 0 - 127     | <A1 - <C7, >A1 - >C7        |
| 17     | TVF Bias Level       | 0 - 14      | -7 - +7                     |

### 3. Partial Parameters (TVA Section)
| Offset | Function              | Value Range | Description                  |
|--------|----------------------|-------------|------------------------------|
| 35     | TVA Level            | 0 - 100     | 0 - 100                     |
| 36     | TVA Velocity Range   | 0 - 100     | -50 - +50                   |
| 37     | TVA Bias Point Dir   | 0 - 127     | <A1 - <C7, >A1 - >C7        |
| 38     | TVA Bias Level       | 0 - 12      | -12 - 0                     |

### 4. Common Parameters
| Offset | Function              | Value Range | Description                  |
|--------|----------------------|-------------|------------------------------|
| 10     | Structure No.        | 0 - 6       | 1 - 7                       |
| 25     | LFO-1 Waveform       | 0 - 3       | TRI, SAW, SQU, RND          |
| 26     | LFO-1 Rate           | 0 - 100     | 0 - 100                     |
| 37     | Low EQ Frequency     | 0 - 15      | 63, 75, 88, 105, ...Hz      |
| 39     | High EQ Frequency    | 0 - 21      | Multiple specific frequencies|
| 46     | Partial Mute         | 0 - 3       | See Mute Table              |

#### Partial Mute Table
| Value | Partial 1 | Partial 2 |
|-------|-----------|-----------|
| 0     | Muting    | Muting    |
| 1     | Sounding  | Muting    |
| 2     | Muting    | Sounding  |
| 3     | Sounding  | Sounding  |

### 5. Patch Parameters
| Offset | Function              | Value Range | Description                  |
|--------|----------------------|-------------|------------------------------|
| 20     | Portamento Mode      | 0 - 2       | U, L, UL                     |
| 21     | Hold Mode            | 0 - 2       | U, L, UL                     |
| 22     | Upper Tone Key Shift | 0 - 48      | -24 - +24                    |
| 34     | Chase Mode           | 0 - 2       | UL, ULL, ULU                 |

## MIDI System Exclusive Implementation

### Message Types

#### 1. Request (RQ1) - One Way Command
```
F0 41 0n 14 11 aaaa bbbb cccc dddd eeee ffff gg F7
   |  |  |  |  |  |_________________| |_____| |
   |  |  |  |  |    Address bytes     Size    EOX
   |  |  |  |  Command-ID (RQ1)
   |  |  |  Model-ID (D-50)
   |  |  Device-ID (MIDI Channel)
   Roland-ID
```

#### 2. Data Set (DT1) - Two Way Command
```
F0 41 0n 14 12 aaaa bbbb cccc dd ... ee F7
   |  |  |  |  |  |__________| |___|  |
   |  |  |  |  |    Address    Data   EOX
   |  |  |  |  Command-ID (DT1)
   |  |  |  Model-ID (D-50)
   |  |  Device-ID (MIDI Channel)
   Roland-ID
```

### Checksum Calculation
- The checksum is calculated as the sum of all bytes between Command-ID and EOX (excluding both)
- Result must be 00H (7 bits)
- Command-ID and EOX are not included in calculation

### Special Notes
1. PG-1000 Parameter Request:
   - Address: [00-00-00]
   - Size: [00-03-25] (421 bytes)
   - Only transmitted when Parameter Request button is pressed

2. Device-ID Calculation:
   - Device-ID = MIDI Channel - 1
   - Example: Channel 1 = Device-ID 0

3. Patch Write:
   - Address: [00-20-00]
   - Transmitted when Manual Button is pressed twice while holding Partial Mute
   - Sends two 00H bytes

## Implementation Notes
1. All addresses consist of three bytes
2. Parameter values are 7-bit (0-127)
3. Some parameters have specific value mappings (like LFO waveforms)
