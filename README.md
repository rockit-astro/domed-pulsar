## Pulsar Dome daemon

`pulsar_domed` communicates with the Pulsar dome controller. Control is exposed via Pyro.

`dome` is a commandline utility that interfaces with the dome daemon.

`python3-rockit-dome-pulsar` is a python module with the common dome code.

### Configuration

Configuration is read from json files that are installed by default to `/etc/domed`.
A configuration file is specified when launching the dome server, and the `dome` frontend will search this location when launched.

```python
{
  "daemon": "warwick_heliostat_dome", # Run the server as this daemon. Daemon types are registered in `rockit.common.daemons`.
  "log_name": "domed@warwick_heliostat", # The name to use when writing messages to the observatory log.
  "control_machines": ["WarwickHeliostatDome", "WarwickTCS"], # Machine names that are allowed to control (rather than just query) state. Machine names are registered in `rockit.common.IP`.
  "serial_port": "/dev/dome", # Serial FIFO for communicating with the dome motors.
  "serial_baud": 115200, # Serial baud rate (always 115200).
  "serial_timeout": 3, # Serial communication timeout.
  "idle_loop_delay": 10, # Status refresh rate in seconds when the dome is not moving.
  "moving_loop_delay": 1, # Status refresh rate in seconds when the dome azimuth or shutter is moving.
  "azimuth_move_timeout": 180, # Maximum movement time between any two azimuth positions (including homing).
  "shutter_move_timeout": 180 # Maximum movement time to fully open or clsoe the shutter.
}
```

### Testing Locally

The dome server and client can be run directly from a git clone:
```
./pulsar_domed warwick.json
DOMED_CONFIG_PATH=./warwick.json ./dome status
```
