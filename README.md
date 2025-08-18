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
  "azimuth_serial_port": "/dev/dome", # Serial FIFO for communicating with the azimuth controller.
  "azimuth_serial_baud": 115200, # Serial baud rate (always 115200).
  "azimuth_serial_timeout": 1, # Serial communication timeout.
  "park_azimuth": 140, # The angle relative to north of the dome slit at the standard park position.
  "home_azimuth": 140, # The angle relative to north of the dome slit at the home position.
  "azimuth_loop_delay": 5, # Status refresh rate in seconds when the dome is not moving.
  "azimuth_moving_loop_delay": 0.5, # Status refresh rate in seconds when the dome azimuth is moving.
  "azimuth_move_timeout": 180, # Maximum movement time between any two azimuth positions (including homing).
  "shutter_move_timeout": 70, # Maximum movement time to fully open or close the shutter.
  "shutter_serial_port": "/dev/ttyUSB0", # Serial FIFO for communicating with the shutter controller.
  "shutter_serial_baud": 4800, # Serial baud rate (always 4800).
  "shutter_serial_timeout": 3, # Serial communication timeout.
  "latitude": 52.376861, # Site latitude in degrees.
  "longitude": -1.583861, # Site longitude in degrees.
  "altitude": 94, # Site altitude in metres.
  "dome_radius_cm": 134,
  "telescope_offset_x_cm": -20,
  "tracking_max_separation": 2 # The maximum allowed difference between the telescope and dome azimuth when tracking a target.
}
```

### Testing Locally

The dome server and client can be run directly from a git clone:
```
./pulsar_domed warwick.json
DOMED_CONFIG_PATH=./warwick.json ./dome status
```
