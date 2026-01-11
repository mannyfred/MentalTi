# MentalTi
Mentally ill Microsoft-Windows-Threat-Intelligence parser that somehow works for the most part

### Features:
- Select only the events you are interested in
- Select only specific members you want from an event (with a shitty UI)
- Easily usable JSON output
- Almost _full_ event definitions (check `EtwTi.hpp` for more info)
- Symbol resolving for addresses in 64bit KnownDlls (setup `_NT_SYMBOL_PATH` for better results)
- Stacktraces for 64bit processes

### Compiling:
- WDK for KMentalTi
- C++20 for MentalTi
- In `Main.cpp` define the statistics interval (in milliseconds)

### Cloning:
```
git clone --recurse-submodules https://github.com/mannyfred/MentalTi
```

### Running:
Since PPL is needed for EtwTi, you need to enable testsigning mode and reboot:
```
bcdedit /set testsigning on
```
After that load the driver (name doesn't matter)
```
sc create random type= kernel binpath= C:\Users\someone\Downloads\KMentalTi.sys
sc start random
```

Specify if you want to monitor all processes with `all`, or specify a PID:

`-proc` flags:
- `<pid>` - Log specified events only from a specific process.
- `all` - Log specified events from all processes. Check `ParseUserKeywords` in `Utils.cpp`, depending on the specified events, event logging is modified for each running process and later created process (some event emissions are disabled unless enabled. Some processes have these enabled, some don't)
- `all-og` - Log specified events from all processes. Don't modify flags, keep them as they are for each process.

```
MentalTi.exe -proc all "0x40 | 0x1000" log.json
MentalTi.exe -proc all-og "0x100000 | 0x200000" log.json
MentalTi.exe -proc 6969 "0x4 | 0x4000" log.json
```

---

Third argument defines all of the events you are interested in. Full list of available events is displayed when running the program with incorrect amount of arguments.
For each event, select the members you actually want to retrieve:

![image](https://github.com/user-attachments/assets/790a0271-8f21-4548-ba06-bd283bfb8229)

_SPACEBAR to select/deselect member, Arrow UP/DOWN for movement, Enter to confirm and continue_

---

Fourth argument specifies the log file you wish to write the data to. This can be a relative or full path.
Output is compact, just use `jq` or similar to look at it.

---

Fifth argument and so on configures stacktracing for 64-bit processes. To enable stacktracing, use the `-trace` flag with desired Event IDs:
```
MentalTi.exe -proc all-og "0x4 | 0x40 | 0x400 | 0x1000" log.json -trace 1,2,3
```
_Make sure Event IDs are comma separated and without spaces_

Besides `*_KERNEL` events, stacktraces for the following Event IDs also aren't supported as the traces contain only kernel-mode addresses:
- 4 - `QUEUEUSERAPC_REMOTE`
- 5 - `SETTHREADCONTEXT_REMOTE`

> [!NOTE]
> Stacktracing might cause you to lose events. Increasing `EVENT_TRACE_PROPERTIES::BufferSize` can help

Stacktraces contain offsets from the main process' image alongside 64-bit KnownDll addresses, kernel addresses are filtered out:
```json
{
  "Data": {
    "AllocationType": 8192,
    "BaseAddress": [
      "0x25c10fd0000"
    ]
  },
  "Metadata": {
    "EventId": 6,
    "Exe": "powershell.exe",
    "ProcessId": 5460,
    "Stack": [
      "ntdll!NtAllocateVirtualMemory+0x14",
      "ntdll!RtlCreateHeap+0x482",
      "ntdll!RtlCreateHeap+0x22",
      "kernelbase!HeapCreate+0x4b",
      "0x7ffe21bb04fe",
      "0x7ffe21bb0485",
      "0x7ffe21bb0884",
      "0x7ffe21be7706",
      "0x7ffe21b5c930",
      "0x7ffe21bedd7d",
      "0x7ffe21a8f829",
      "0x7ffe21e3e173",
      "0x7ffe21e3e0e0",
      "powershell.exe+0x46d8",
      "powershell.exe+0x1dcc",
      "powershell.exe+0x51e4",
      "kernel32!BaseThreadInitThunk+0x17",
      "ntdll!RtlUserThreadStart+0x2c"
    ]
  }
}
```

---

After starting it, console is updated at the defined interval. Events and their log count is displayed:

![image](https://github.com/user-attachments/assets/cd2e009f-8948-436a-b4ba-497a5a90b98d)


### Example output parsing:
#### 1. Detect memory fluctuations
```bash
cat log.json | jq -s '[.[] | select(.Metadata.EventId == 7) | {PID: .Metadata.ProcessId, Base: .Data.BaseAddress, Exe: .Metadata.Exe, Flip: [.Data.ProtectionMask, .Data.LastProtectionMask]}] | group_by(.Base) | map(. as $group | {ProcessId: $group[0].PID, BaseAddress: $group[0].Base, Exe: $group[0].Exe, Flips: (reduce $group[] as $item ([]; if length == 0 or .[-1][0] != $item.Flip[1] then . + [$item.Flip] else . end) | length - 1)} | select(.Flips > 3))'
```
Example output:
```json
[
  {
    "ProcessId": 6928,
    "BaseAddress": [
      "0x2316bac0000"
    ],
    "Exe": "MsMpEng.exe",
    "Flips": 4
  },
  {
    "ProcessId": 25052,
    "BaseAddress": [
      "0x7ff7cad80000"
    ],
    "Exe": "defnotabeacon.exe",
    "Flips": 14
  }
]
```

#### 2. Detect RW -> RX remote protection change
```bash
cat log.json | jq -s '[.[] | select(.Metadata.EventId == 2 and .Data.LastProtectionMask == 4 and .Data.ProtectionMask == 32)]'
```
Example output:
```json
[
  {
    "Data": {
      "BaseAddress": [
        "0x20017950000"
      ],
      "CallingProcessId": 13060,
      "LastProtectionMask": 4,
      "ProtectionMask": 32,
      "RegionSize": [
        "0x10000"
      ],
      "TargetProcessId": 7468
    },
    "Metadata": {
      "EventId": 2,
      "Exe": "random.exe",
      "ProcessId": 13060
    }
  },
  {
    "Data": {
      "BaseAddress": [
        "0x7ffc8f50d000"
      ],
      "CallingProcessId": 7468,
      "LastProtectionMask": 4,
      "ProtectionMask": 32,
      "RegionSize": [
        "0x10"
      ],
      "TargetProcessId": 1084
    },
    "Metadata": {
      "EventId": 2,
      "Exe": "msedge.exe",
      "ProcessId": 7468
    }
  }
]
```

#### 3. Detect APC injection (unbacked memory)
```bash
cat log.json | jq -s '[ .[] | select(.Metadata.EventId == 4) | select(.Data.ApcRoutineVadMmfName == 0) | { CallingPID: .Data.CallingProcessId, CallingTID: .Data.CallingThreadId, TargetPID: .Data.TargetProcessId, TargetTID: .Data.TargetThreadId, ApcRoutine: .Data.ApcRoutine }]'
```
Example output:
```json
[
  {
    "CallingPID": 9412,
    "CallingTID": 12552,
    "TargetPID": 13060,
    "TargetTID": 9388,
    "ApcRoutine": [
      "0x1bdf95e0000"
    ]
  }
]
```

#### 4. Detect Remote Writes to Addresses in 64bit KnownDlls
```bash
cat log.json | jq -s '[ .[] | select(.Metadata.EventId == 14) | select(.Data.BaseAddress | length > 1)]'
```
Example output:
```json
[
  {
    "Data": {
      "BaseAddress": [
        "0x7ff8c59106c0",
        "ntdll!ZwOpenProcessTokenEx"
      ],
      "BytesCopied": [
        "0x10"
      ],
      "CallingProcessCreateTime": "07:48:16.306",
      "CallingProcessId": 33176,
      "CallingProcessProtection": 0,
      "OperationStatus": 0,
      "TargetProcessCreateTime": "07:49:37.279",
      "TargetProcessId": 35948,
      "TargetProcessProtection": 0
    },
    "Metadata": {
      "EventId": 14,
      "Exe": "msedge.exe",
      "ProcessId": 33176
    }
  },
  {
    "Data": {
      "BaseAddress": [
        "0x7ff8c59106a0",
        "ntdll!NtOpenThreadTokenEx"
      ],
      "BytesCopied": [
        "0x10"
      ],
      "CallingProcessCreateTime": "07:48:16.306",
      "CallingProcessId": 33176,
      "CallingProcessProtection": 0,
      "OperationStatus": 0,
      "TargetProcessCreateTime": "07:49:37.279",
      "TargetProcessId": 35948,
      "TargetProcessProtection": 0
    },
    "Metadata": {
      "EventId": 14,
      "Exe": "msedge.exe",
      "ProcessId": 33176
    }
  }
]
```

#### 5. Detect Remote Memory Protection Change via Randomized Indirect Syscall

- Filter for Event ID 2 (`ETWTI_PROTECTVM_REMOTE`)
- Check last stackframe to see if it doesn't contain `ntdll!NtProtectVirtualMemory+0x14`

```bash
cat log.json | jq -s '[ .[] | select(.Metadata.EventId == 2 and .Metadata.Stack[0] != "ntdll!NtProtectVirtualMemory+0x14") ]'
```
Example output:
```json
[
  {
    "Data": {
      "BaseAddress": [
        "0x2c0ceb90000"
      ],
      "FullRegionSize": 4096,
      "LastProtectionMask": 4,
      "ProtectionMask": 64,
      "RegionSize": [
        "0x1000"
      ],
      "TargetAddress": [
        "0x2c0ceb90000"
      ],
      "TargetProcessId": 5708,
      "VaVadAllocationBase": [
        "0x2c0ceb90000"
      ],
      "VaVadAllocationProtect": 4,
      "VaVadCommitSize": [
        "0x1000"
      ],
      "VaVadMmfName": 0,
      "VaVadQueryResult": 0,
      "VaVadRegionSize": [
        "0x1000"
      ],
      "VaVadRegionType": 131072
    },
    "Metadata": {
      "EventId": 2,
      "Exe": "delete.exe",
      "ProcessId": 536,
      "Stack": [
        "ntdll!NtTerminateThread+0x14",
        "delete.exe+0x1638",
        "delete.exe+0x1807",
        "delete.exe+0x1af0",
        "kernel32!BaseThreadInitThunk+0x17",
        "ntdll!RtlUserThreadStart+0x2c"
      ]
    }
  }
]
```

---

### Notes & Issues:

- ALLOCVM/MAPVIEW/PROTECT events get logged only when an executable bit is set
- There might be some issues related to some other events. If you aren't receiving events, that you clearly should be, try to run with `-proc all`


- Single header dependency ([nlohmann/json](https://github.com/nlohmann/json))
- Event ID 4 and 5 (`QUEUE_REMOTE_APC`/`SETTHREAD_CONTEXT_REMOTE`) always has PID as 4 in the event header. Because of this, when you are targeting a specific PID, all of these events will be logged so you don't miss them.
- `IsSandboxedToken` in event 35 (`SYSCALL_USAGE`) not working.
- `PreviousTokenTrustLevel`, `PreviousTokenGroups`, `CurrentTokenTrustLevel` and `CurrentTokenGroups` not included in events 33 and 36 (`IMPERSONATION_UP`/`IMPERSONATION_DOWN`) since they use some X type for those that I can't be bothered to figure out atm.
- Uses buffered io - when trying to take a look at the output while it's running, it might not be valid json, or it might be empty all together. When exiting the program, stuff gets flushed to disk and you get everything.
- Maybe some other issues and edge cases somewhere

