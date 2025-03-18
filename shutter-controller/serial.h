//**********************************************************************************
//  Copyright 2016, 2017, 2023, 2025 Paul Chote
//  This file is part of the Robotic Observatory Control Kit (rockit)
// 
//  rockit is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  rockit is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with rockit.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************

#include <stdarg.h>
#include <stdint.h>

#ifndef SERIAL_H
#define SERIAL_H

void serial_initialize(void);
bool serial_can_read(void);
uint8_t serial_read(void);
void serial_write(uint8_t b);

#endif
