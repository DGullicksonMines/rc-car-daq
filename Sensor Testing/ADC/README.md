# ADC Testing

This directory contains **Python** code for testing an **MCP3424**. The **MCP3424** is a **differential 4-channel ADC** capable of **I2C** communication.

`read_adc.py` is the top-level program and will allow the user to probe a voltage from any desired channel of the **ADC**.

`mcp3424.py` contains helpful code for interfacing with the **MCP3424** over **I2C**.