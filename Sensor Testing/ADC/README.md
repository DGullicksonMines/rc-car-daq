# ADC Testing

This directory contains **Python** and **C** code for testing an **MCP3424**. The **MCP3424** is a **differential 4-channel ADC** capable of **I2C** communication.

**C** code may be built using **ninja** or **make** and will produce outputs in `out/`.

`read_adc.py` is the top-level program and will allow the user to probe a voltage from any desired channel of the **ADC**.

`read_adc.c` is the **C** equivalent and is built to `out/read_adc`.

`mcp3424.py` contains helpful code for interfacing with the **MCP3424** over **I2C**.

`mcp3424.c` and `mcp3424.h` are the **C** equivalents and are built to `out/mcp3424.o`.