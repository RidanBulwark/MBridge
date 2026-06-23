

# Dependency Matrix:

|Tier | Folder name | What lives here | Can #include from... | BANNED from #includeing| \n
|---|---|---|---|---|---|
3 | `app/` | High-level thread logic | `services/, drivers/, sys/`| None (Top of the food chain)|
2| `services/` |Middleware / Gatekeepers|`drivers/, sys/`| `app/`|
1| `drivers/` | Silicon manipulators | `sys/`| `services/, app/`|
0| `sys/`| Pure logic & OS primitives| `<stdint.h>, <stdbool.h>`|`drivers/, services/, app/`