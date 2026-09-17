# WLXPipeline

Media pipeline factory DLL (`GetPipelineCreateFunctions`).

## Contract Coverage

`GetPipelineCreateFunctions` returns `S_OK` and a real 6-pointer function
table (`uVersion`, `uStructSize`, `pfnCreate`, `pfnDestroy`, `pfnProcess`,
`pfnGetInfo`). Covered by `tests/mmr-python/wmmr/contracts/wlxpipeline.py`.
Reference `build_clean` returns `E_NOTIMPL`; contracts pin our real table.