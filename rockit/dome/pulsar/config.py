#
# This file is part of the Robotic Observatory Control Kit (rockit)
#
# rockit is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rockit is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rockit.  If not, see <http://www.gnu.org/licenses/>.

"""Helper function to validate and parse the json config file"""

# pylint: disable=too-many-instance-attributes

import json
from rockit.common import daemons, IP, validation

CONFIG_SCHEMA = {
    'type': 'object',
    'additionalProperties': False,
    'required': [
        'daemon', 'log_name', 'control_machines',
        'azimuth_serial_port', 'azimuth_serial_baud', 'azimuth_serial_timeout',
        'shutter_serial_port', 'shutter_serial_baud', 'shutter_serial_timeout',
        'azimuth_loop_delay', 'azimuth_moving_loop_delay', 'home_azimuth', 'park_azimuth',
        'azimuth_move_timeout', 'shutter_move_timeout',
    ],
    'properties': {
        'daemon': {
            'type': 'string',
            'daemon_name': True
        },
        'log_name': {
            'type': 'string'
        },
        'control_machines': {
            'type': 'array',
            'items': {
                'type': 'string',
                'machine_name': True
            }
        },
        'azimuth_serial_port': {
            'type': 'string'
        },
        'azimuth_serial_baud': {
            'type': 'integer',
            'minimum': 115200,
            'maximum': 115200
        },
        'azimuth_serial_timeout': {
            'type': 'number',
            'minimum': 0
        },
        'azimuth_serial_retries': {
            'type': 'number',
            'minimum': 0
        },
        'shutter_serial_port': {
            'type': 'string'
        },
        'shutter_serial_baud': {
            'type': 'integer',
            'minimum': 4800,
            'maximum': 4800
        },
        'shutter_serial_timeout': {
            'type': 'number',
            'minimum': 0
        },
        'azimuth_loop_delay': {
            'type': 'number',
            'min': 0
        },
        'azimuth_moving_loop_delay': {
            'type': 'number',
            'min': 0
        },
        'home_azimuth': {
            'type': 'number',
            'minimum': 0
        },
        'park_azimuth': {
            'type': 'number',
            'minimum': 0
        },
        'azimuth_move_timeout': {
            'type': 'number',
            'min': 0
        },
        'shutter_move_timeout': {
            'type': 'number',
            'min': 0
        }
    }
}


class Config:
    """Daemon configuration parsed from a json file"""
    def __init__(self, config_filename):
        # Will throw on file not found or invalid json
        with open(config_filename, 'r', encoding='utf-8') as config_file:
            config_json = json.load(config_file)

        # Will throw on schema violations
        validation.validate_config(config_json, CONFIG_SCHEMA, {
            'daemon_name': validation.daemon_name_validator
        })

        self.daemon = getattr(daemons, config_json['daemon'])
        self.log_name = config_json['log_name']
        self.control_ips = [getattr(IP, machine) for machine in config_json['control_machines']]
        self.azimuth_serial_port = config_json['azimuth_serial_port']
        self.azimuth_serial_baud = config_json['azimuth_serial_baud']
        self.azimuth_serial_timeout = config_json['azimuth_serial_timeout']
        self.shutter_serial_port = config_json['shutter_serial_port']
        self.shutter_serial_baud = config_json['shutter_serial_baud']
        self.shutter_serial_timeout = config_json['shutter_serial_timeout']
        self.home_azimuth = config_json['home_azimuth']
        self.park_azimuth = config_json['park_azimuth']
        self.azimuth_loop_delay = float(config_json['azimuth_loop_delay'])
        self.azimuth_moving_loop_delay = float(config_json['azimuth_moving_loop_delay'])
        self.azimuth_move_timeout = int(config_json['azimuth_move_timeout'])
        self.shutter_move_timeout = int(config_json['shutter_move_timeout'])
