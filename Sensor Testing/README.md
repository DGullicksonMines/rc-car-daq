# Sensor Testing

This directory should contain a subdirectory for each sensor to be tested.

Sensors will be tested either using **Python** or **C** on the **Raspberry Pi** or using **Arduino C** on an **Arduino**.

---

For help using **Python** to interface with pins on the **Raspberry Pi**, please see https://www.raspberrypi.com/documentation/computers/os.html.

The only interfacing modules that should be needed are:

- `busio` - For **I2C** communication.  
Documentation can be found here: https://docs.circuitpython.org/en/latest/shared-bindings/busio/.

- `smbus` - Alternative for **I2C** communication.  
Documentation can be found here: https://smbus2.readthedocs.io/en/latest/index.html.  
*NOTE:* `smbus` and `smbus2` are equivalent libraries, differing only in their implementations. `smbus` should be pre-installed on **Pi OS** but, if not, `smbus2` should be preferred.

- `gpiozero` - FOR **GPIO** use.  
Documentation can be found here: https://gpiozero.readthedocs.io/en/stable/.  

If any modules need to be installed, installation should be done in a **Python Virtual Environment** constructed inside this directory named `sensor_testing_env`.

- To create a **Virtual Environment**, run `python -m venv sensor_testing_env` from a command line positioned inside this directory.

- To activate a **Virtual Environment**, run `source sensor_testing_env/bin/activate` from a command line positioned inside this directory.  
Alternatively, run `../source sensor_testing_env/bin/activate` from a command line positioned inside a direct subdirectory.  
*NOTE:* The **Virtual Environment** *must* be reactivated for each new shell that is created.  
If this is hassle, a simple runner script should be created.  
Alternatively, **VS Codium**, if used with the **Python** extension, can be configured to take care of **Virtual Environments** automatically.

- Once a **Virtual Environment** has been activated, `pip install` may then be used to install any needed **Python** modules.

