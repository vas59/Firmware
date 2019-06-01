/****************************************************************************
 *
 *   Copyright (c) 2018-2019 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/*
 * ADIS16477.hpp
 *
 */

#pragma once

#include <drivers/device/spi.h>
#include <ecl/geo/geo.h>
#include <lib/conversion/rotation.h>
#include <lib/drivers/accelerometer/PX4Accelerometer.hpp>
#include <lib/drivers/gyroscope/PX4Gyroscope.hpp>
#include <perf/perf_counter.h>
#include <px4_getopt.h>
#include <px4_work_queue/ScheduledWorkItem.hpp>

class ADIS16477 : public device::SPI, public px4::ScheduledWorkItem
{
public:
	ADIS16477(int bus, uint32_t device, enum Rotation rotation = ROTATION_NONE);
	virtual ~ADIS16477();

	virtual int		init();

	void			print_info();

protected:
	virtual int		probe();

private:

	uint8_t			*_dma_data_buffer{nullptr};

	PX4Accelerometer	_px4_accel;
	PX4Gyroscope		_px4_gyro;

	perf_counter_t		_sample_interval_perf;
	perf_counter_t		_sample_perf;
	perf_counter_t		_bad_transfers;

	// Report conversation with in the ADIS16477, including command byte and interrupt status.
	struct ADISReport {
		uint16_t	cmd;
		uint16_t	diag_stat;
		int16_t		gyro_x;
		int16_t		gyro_y;
		int16_t		gyro_z;
		int16_t		accel_x;
		int16_t		accel_y;
		int16_t		accel_z;
		uint16_t	temp;
		uint16_t	DATA_CNTR;
		uint8_t		checksum;
		uint8_t		_padding; // 16 bit SPI mode
	};
	// ADIS16477 burst report should be 176 bits
	static_assert(sizeof(ADISReport) == (176 / 8), "ADIS16477 report not 176 bits");

	/**
	 * Start automatic measurement.
	 */
	void			start();

	/**
	 * Stop automatic measurement.
	 */
	void			stop();

	/**
	 * Reset chip.
	 *
	 * Resets the chip and measurements ranges, but not scale and offset.
	 */
	int			reset();

	void			Run() override;

	static int		data_ready_interrupt(int irq, void *context, void *arg);

	/**
	 * Fetch measurements from the sensor and update the report buffers.
	 */
	int			measure();

	uint16_t		read_reg16(uint8_t reg);

	void			write_reg(uint8_t reg, uint8_t value);
	void			write_reg16(uint8_t reg, uint16_t value);

	// ADIS16477 onboard self test
	bool 			self_test_memory();
	bool 			self_test_sensor();

};
