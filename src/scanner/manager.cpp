/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <iostream>
#include <glibmm/timer.h>
#include <glibmm/dispatcher.h>
#include <glibmm/ustring.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>

#include <cmath>

#include <errno.h>
#include <unistd.h>

/* files from src directory begin */
#include "application.hpp"
#include "global_strings.hpp"
#include "utils.hpp"
/* files from src directory end */

#include "defines.hpp"
#include "commands.hpp"
#include "adc_count.hpp"
#include "manager.hpp"

namespace ScanAmati {

namespace Scanner {

SharedManager Manager::instance_;

Manager::Manager()
	:
	thread_back_(0),
	thread_run_(0),
	stop_(false),
	regulator_(10),
	fd_(-1)
{
	buffers_.com.buf = new char[SCANNER_BUFFER];
}

Manager::~Manager()
{
	delete [] buffers_.com.buf;
}

void
Manager::run( RunType run_type, const AcquisitionParameters& params)
{
	sigc::slot<void> slot;
	switch (run_type) {
	case RUN_INITIATION:
		slot = sigc::mem_fun( *this, &Manager::run_initiation);

		if (thread_back_) {
			thread_back_->join();
			thread_back_ = 0;
		}
		thread_back_ = Glib::Thread::create(
			sigc::mem_fun( *this, &Manager::run_background), true);
		break;
	case RUN_IMAGE_ACQUISITION:
		data_.width_type_ = params.width_type;
		data_.calibration_type_ = params.calibration_type;
		data_.intensity_type_ = params.intensity_type;
		data_.filter_type_ = params.filter_type;
		slot = sigc::bind( sigc::mem_fun( *this, &Manager::run_image_acquisition),
			params);
		break;
	case RUN_IMAGE_RECONSTRUCTION:
		slot = sigc::mem_fun( *this, &Manager::run_reconstruction_loop);
		break;
	case RUN_COMMANDS:
		slot = sigc::bind( sigc::mem_fun( *this, &Manager::run_commands),
			params);
		break;
	case RUN_LINING_ACQUISITION:
		data_.lining_count_ = params.lining_count;
		slot = sigc::bind(
			sigc::mem_fun( *this, &Manager::run_lining_acquisition),
			params);
		break;
	default:
		return;
	}

	thread_run_ = Glib::Thread::create( slot, true);
}
/*
void
Manager::run( RunType run_type, RunArguments args, boost::any arg)
{
	sigc::slot<void> slot;
	switch (run_type) {
	case RUN_INITIATION:
		slot = sigc::bind( sigc::mem_fun( *this, &Manager::run_initiation),
			args);

		if (thread_back_) {
			thread_back_->join();
			thread_back_ = 0;
		}
		thread_back_ = Glib::Thread::create(
			sigc::mem_fun( *this, &Manager::run_background), true);
		break;
	case RUN_IMAGE_ACQUISITION:
		{
			WidthCalibrationPair pair( WIDTH_FULL, CALIBRATION_ROUGH);
			if (arg.type() == typeid(WidthCalibrationPair)) {
				pair = boost::any_cast<WidthCalibrationPair>(arg);
				data_.width_type_ = pair.first;
				data_.calibration_type_ = pair.second;
			}
			slot = sigc::bind(
				sigc::mem_fun( *this, &Manager::run_image_acquisition),
				args);
		}
		break;
	case RUN_IMAGE_RECONSTRUCTION:
		slot = sigc::mem_fun( *this, &Manager::run_reconstruction_loop);
		break;
	case RUN_COMMANDS:
//		slot = sigc::bind( sigc::mem_fun( *this, &Manager::run_commands),
//			args, arg);
		break;
	case RUN_LINING_ACQUISITION:
		{
			LiningPair pair( LINING_ACCURACY_OPTIMAL, SCANNER_LINING_COUNT);
			if (arg.type() == typeid(LiningPair)) {
				pair = boost::any_cast<LiningPair>(arg);
				data_.lining_count_ = pair.second;
			}
			slot = sigc::bind(
				sigc::mem_fun( *this, &Manager::run_lining_acquisition),
				args, pair.first);
		}
		break;
	default:
		return;
	}

	thread_run_ = Glib::Thread::create( slot, true);
}
*/
void
Manager::run_commands(const AcquisitionParameters& params)
{
	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_COMMANDS;
		cond_run_.wait(mutex_);
		state_.manager_state_.process_ = PROCESS_START;
	}
	OFLOG_DEBUG( app.log, "Commands thread has been started");

	signal_update_();

	try {
		size_t i = 0;
		size_t size = params.commands.size();
		std::vector<CommandSharedPtr>::const_iterator it;
		for ( it = params.commands.begin(); it != params.commands.end(); ++it, ++i)
		{
			write_command(*it);
			{
				Glib::Mutex::Lock lock(mutex_);
				state_.manager_state_.progress_ = double(i + 1) / size;
			}
			signal_update_();
			Glib::usleep(5000);
		}
	}
	catch (const Error& er) {
		std::cout << er.what() << std::endl;
		set_error(er);
		return;
	}
	catch (const Exception& ex) {
		std::cout << ex.what() << std::endl;
		set_error(ex);
		return;
	}

	{
		Glib::Mutex::Lock lock(mutex_);
		if (params.value.type() == typeid(double)) {
			// set new chip capacity
			state_.capacity_ = boost::any_cast<double>(params.value);
			OFLOG_DEBUG( app.log, "Select capacity (pF): " << state_.capacity_);
		}
		else if (params.value.type() == typeid(char)) {
			// set new chip code
			state_.chip_ = boost::any_cast<char>(params.value);
			OFLOG_DEBUG( app.log, "Select chip code: " << state_.chip_);
		}
		else
			;

		state_.manager_state_.set_process_finish();
	}
	signal_update_();
	Glib::usleep(200000);

	OFLOG_DEBUG( app.log, "Commands thread has been finished");

	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_BACKGROUND;
		cond_back_.signal();
	}
}

void
Manager::stop(bool stop_everything)
{
	OFLOG_DEBUG( app.log, "Stop has been initiated");
	{
		Glib::Mutex::Lock lock(mutex_);
		stop_ = true;
	}

	// Here we block to truly wait for the run thread to complete
	join_run_thread();

	if (stop_everything) {
		OFLOG_DEBUG( app.log, "Stop everything has been initiated");
		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.run_ = RUN_BACKGROUND;
			cond_back_.signal();
		}

		if (thread_back_) {
			// Here we block to truly wait for the background thread to complete
			thread_back_->join();
			thread_back_ = 0;
		}

		OFLOG_DEBUG( app.log, "No more active (running) threads");
		// no more active (running) threads
		if (connection_io_.connected()) {
			connection_io_.disconnect();

			std::string scanner_id;
			{
				Glib::Mutex::Lock lock(mutex_);
				scanner_id = state_.id_;
			}
			if (!scanner_id.empty()) {
				save_preferences(scanner_id);

				std::string filename = get_lining_file(scanner_id);
				data_.save_lining(filename);
			}
			try {
				close();
			}
			catch (const Error& err) {
			}
			data_.clear();
			regulator_.reset();
		}
		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.progress_ = 0;
			state_.what_.clear();
			state_.todo_.clear();
			stop_ = false;
		}
	}
	else {
		Glib::Mutex::Lock lock(mutex_);
		stop_ = false;
	}
}

void
Manager::run_initiation()
{
	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_INITIATION;
		state_.manager_state_.process_ = PROCESS_START;
		state_.manager_state_.connected_ = false;
	}

	try {
		open(device_name);

		connection_io_ = connect(sigc::mem_fun( *this, &Manager::io_handler));

		handshake();

		std::string scanner_id = id();

		if (check_scanner_temperature_file(scanner_id)) {
			std::string filename = get_temperature_file(scanner_id);
			regulator_.load_data(filename);

			if (check_scanner_lining_file(scanner_id)) {
				filename = get_lining_file(scanner_id);
				data_.load_lining(filename);
			}
			if (check_scanner_bad_strips_file(scanner_id)) {
				filename = get_bad_strips_file(scanner_id);
				data_.load_bad_strips(filename);
			}
		}
		else {
			throw Exception(_("Unable to load scanner data."));
		}

		load_preferences(scanner_id);

		{
			Glib::Mutex::Lock lock(mutex_);
			Command* com = Commands::create(COMMAND_ARRAY_RESET);
			write_command(com);
		}

		for ( AssemblyConstIter it = data_.assembly_.begin();
			it != data_.assembly_.end(); ++it) {
			Command* com = Commands::create( it->code, it->lining);
			write_command(com);
			{
				Glib::Mutex::Lock lock(mutex_);
				int i = Data::chip_number(it->code);
				state_.manager_state_.progress_ = double(i + 1) / SCANNER_CHIPS;
			}
			signal_update_();
			Glib::usleep(1000);
		}

		{
			Glib::Mutex::Lock lock(mutex_);
			Command* com = Commands::create(state_.capacity_);
			write_command(com);
		}

		{
			Glib::Mutex::Lock lock(mutex_);
			if (state_.temperature_control_) {
				Command* com = Commands::create( COMMAND_PELTIER_ON,
					state_.peltier_code_);
				write_command(com);
				regulator_.code_value_changed_ = true;
			}
		}
	}
	catch (const Error& err) {
		if (connection_io_.connected()) {
			close();
			connection_io_.disconnect();
		}
		set_error(err);
		return;
	}
	catch (const Exception& ex) {
		if (connection_io_.connected()) {
			close();
			connection_io_.disconnect();
		}
		set_error(ex);
		return;
	}

	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.set_process_finish();
		stop_ = false;
	}
	signal_update_();
	Glib::usleep(200000);

	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_BACKGROUND;
		state_.manager_state_.connected_ = true;
		cond_back_.signal();
	}
}

void
Manager::run_background()
{
	while (1) {
		{
			Glib::Mutex::Lock lock(mutex_);
			while (state_.manager_state_.run_ != RUN_BACKGROUND) {
				cond_run_.signal();
				cond_back_.wait(mutex_);
			}
			if (stop_ || state_.manager_state_.process_error())
				break;
		}

//		::sleep(1);
		Glib::usleep(500000);

		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.process_ = PROCESS_NONE;
		}
		try {
			double temperature = this->temperature();
			bool temperature_control = false;
			{
				Glib::Mutex::Lock lock(mutex_);
				state_.temperature_ = temperature;
				temperature_control = state_.temperature_control_;
			}

			if (temperature_control) {
				regulator_.add(temperature);
				if (regulator_.code_value_changed_) {
					Glib::Mutex::Lock lock(mutex_);
					state_.peltier_code_ = regulator_.code_;
				}
				Command* com = Commands::create( COMMAND_PELTIER_ON,
					regulator_.code_);
				write_command(com);
			}
			std::cout << temperature << " " << int(regulator_.code_) << " "
				<< regulator_.temperature_margins_[0] << " "
				<< regulator_.temperature_margins_[1] << " "
				<< regulator_.temperature_margins_[2] << std::endl;
			signal_update_();
		}
		catch (const Error& er) {
			OFLOG_ERROR( app.log, "Background error exception: " << er.what());
		}
		catch (const Exception& ex) {
			OFLOG_ERROR( app.log, "Background error exception: " << ex.what());
		}
		catch (...) {
			OFLOG_ERROR( app.log, "Background unexpected error");
		}
	}

	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.progress_ = 0;
		state_.manager_state_.run_ = RUN_NONE;
		if (!state_.manager_state_.process_error())
			state_.manager_state_.process_ = PROCESS_NONE;
		stop_ = false;
	}
	signal_update_();
}

void
Manager::run_lining_acquisition(const AcquisitionParameters& params)
{
	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_LINING_ACQUISITION;
		cond_run_.wait(mutex_);
		state_.manager_state_.process_ = PROCESS_START;
	}
	signal_update_();

	OFLOG_DEBUG( app.log, "Lining acquisition has been started");

	for ( AssemblyIter it = data_.assembly_.begin();
		it != data_.assembly_.end(); ++it) {
		it->clear_for_lining_acquisition();
	}

	std::vector<double> codes = equal_distant_points( SCANNER_LINING_CODE_MIN,
		SCANNER_LINING_CODE_MAX, params.lining_accuracy_type);

	std::vector<char> chips = boost::any_cast< std::vector<char> >(params.value);
	
	for ( std::vector<double>::const_iterator iter = codes.begin();
		iter != codes.end(); ++iter) {
		guint8 v = static_cast<guint8>(ceil(*iter));
		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.progress_ = v / double(SCANNER_LINING_CODE_MAX);
			state_.manager_state_.process_ = PROCESS_ACQUISITION;
		}

		try {
			for ( std::vector<char>::const_iterator it = chips.begin();
				it != chips.end(); ++it) {
				int j = Data::chip_number(*it);
				Command* com = Commands::create( *it, v);
				write_command(com);
			}

			Command* com = Commands::create(COMMAND_ALTERA_START_PEDESTALS);
			write_command(com);
			Glib::usleep(40000);

			if (!acquisition_start( params, ACQUIRE_LINING_PEDESTALS)) {
				std::cerr << "Acquisition start error" << std::endl;
				return;
			}

			if (!acquire_data(ACQUIRE_LINING_PEDESTALS)) {
				std::cerr << "Acquisition data error" << std::endl;
				return;
			}

			{
				Glib::Mutex::Lock lock(mutex_);
				if (stop_)
					break;
			}

		}
		catch (const Error& err) {
			std::cerr << err.what() << std::endl;
			set_error(err);
			return;
		}
		catch (const Exception& ex) {
			std::cout << ex.what() << std::endl;
			set_error(ex);
			return;
		}
		catch (...) {
			std::cout << "shit" << std::endl;
			set_error(Error("shit"));
			return;
		}

		data_.reconstruct( ACQUIRE_LINING_PEDESTALS, v);
		if (v == SCANNER_LINING_CODE_MAX) {
			OFLOG_DEBUG( app.log, "Lining calculation has been started");
			data_.calculate_lining(params.lining_accuracy_type);
			OFLOG_DEBUG( app.log, "Lining calculation has been finished");
		}

		signal_update_();
	}

	{
		Glib::Mutex::Lock lock(mutex_);
		if (stop_) {
			state_.manager_state_.set_process_aborted();
			stop_ = false;
			OFLOG_DEBUG( app.log, "Lining acquisition has been aborted");
		}
		else {
			OFLOG_DEBUG( app.log, "Lining acquisition has been finished");
			state_.manager_state_.set_process_finish();
		}
	}
	signal_update_();
	Glib::usleep(200000);

	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_BACKGROUND;
		cond_back_.signal();
	}
}

void
Manager::run_image_acquisition(const AcquisitionParameters& params)
{
	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_IMAGE_ACQUISITION;
		cond_run_.wait(mutex_);
		state_.manager_state_.process_ = PROCESS_START;
	}
	signal_update_();

	if (params.with_acquisition) {
		if (!acquire_pedestals( params, ACQUIRE_IMAGE_PEDESTALS, 0))
			return;
		else {
			Command* com = Commands::create(COMMAND_ALTERA_START);
			write_command(com);
		}
	}
/*
	for ( AssemblyConstIter it = data_.assembly_.begin();
		it != data_.assembly_.end(); ++it) {
		Command* com = Commands::create( it->code, it->lining);
		write_command(com);
		{
			Glib::Mutex::Lock lock(mutex_);
			int i = Data::chip_number(it->code);
			state_.manager_state_.progress_ = double(i + 1) / SCANNER_CHIPS;
		}
		Glib::usleep(1000);
	}
*/
	try {
		if (!exposure_start(params))
			return;
	}
	catch (int pos) {
		acquisition_abort( params, pos);
		return;
	}

	if (!exposure_finish(params))
		return;
	else
		Glib::usleep(200000);

	if (params.with_acquisition) {
		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.process_ = PROCESS_ACQUISITION;
		}
		signal_update_();
		// acquisition

		if (!acquisition_start( params, ACQUIRE_IMAGE))
			return;

		try {
			if (acquire_data(ACQUIRE_IMAGE)) {
				Glib::usleep(200000);
			}
		}
		catch (bool res) {
			acquisition_abort( params, res);
			return;
		}
	}

	if (acquisition_finish(params)) {
		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.set_process_finish();
		}
		signal_update_();
		Glib::usleep(200000);
	}
	else
		return;

	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_BACKGROUND;
		cond_back_.signal();
	}
}

void
Manager::run_reconstruction_loop()
{
	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_IMAGE_RECONSTRUCTION;
		cond_run_.wait(mutex_);
		state_.manager_state_.process_ = PROCESS_START;
	}

	OFLOG_DEBUG( app.log, "Reconstruction loop has been started");

	while (1) {
		{
			Glib::Mutex::Lock lock(mutex_);
			if (stop_)
				break;
		}

		signal_update_();
		Glib::usleep(200000);
	}
	{
		Glib::Mutex::Lock lock(mutex_);
		if (stop_) {
			state_.manager_state_.set_process_finish();
			stop_ = false;
			OFLOG_DEBUG( app.log, "Reconstruction loop has been finished");
		}
	}
	signal_update_();
	Glib::usleep(200000);

	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_BACKGROUND;
		cond_back_.signal();
	}
}

bool
Manager::exposure_start(const AcquisitionParameters& params) throw(int)
{
	// move forward

	if (params.with_exposure) {
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.process_ = PROCESS_EXPOSURE;
	}
	try {
		Command* com = Commands::create( params.acquisition.movement_forward,
			params.movement_type, DIRECTION_FORWARD);
		write_command(com);
		OFLOG_DEBUG( app.log, "Scanner forward movement started");
	}
	catch (const Exception& ex) {
		set_error(ex);
		return false;
	}

	for ( int i = 0; i < 101; i++) {
		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.progress_ = i / 100.0;
			if (stop_) {
				OFLOG_DEBUG( app.log,
					"Exposure has been interupted, scanner position is " << i);
				throw i;
			}
		}
		signal_update_();
		Glib::usleep(params.acquisition.movement_forward.time * 10000);
	}

	return true;
}

bool
Manager::exposure_abort( const AcquisitionParameters& params, int pos)
{
	{
		Glib::Mutex::Lock lock(mutex_);
		stop_ = false;
		state_.manager_state_.set_process_aborted();
	}

	try {
		if (params.with_exposure) {
			Command* com = Commands::create(COMMAND_XRAY_CHECK_OFF);
			write_command(com);
		}

		Command* com = Commands::create( params.acquisition.movement_reverse,
			params.movement_type, DIRECTION_REVERSE);
		write_command(com);

		for ( int i = pos; i < 101; i++) {
			{
				Glib::Mutex::Lock lock(mutex_);
				state_.manager_state_.progress_ = (100. - i) / 100.0;
			}
			signal_update_();
			Glib::usleep(params.acquisition.movement_reverse.time * 10000);
		}

		com = Commands::create(COMMAND_STEPPERS_STOP);
		write_command(com);

		com = Commands::create(COMMAND_ALTERA_RESET);
		write_command(com);

		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.set_aborted();
		}
		cond_back_.signal();
		return true;
	}
	catch (const Error& err) {
		set_error(err);
		return false;
	}
}

bool
Manager::exposure_finish(const AcquisitionParameters& params)
{
	try {
		Command* com = Commands::create(COMMAND_STEPPERS_STOP);
		write_command(com);
		OFLOG_DEBUG( app.log, "Scanner movement finished");
	}
	catch (const Exception& ex) {
		set_error(ex);
		return false;
	}
	return true;
}

bool
Manager::acquisition_start( const AcquisitionParameters& params, AcquireType acquire)
{
	Command* com = 0;
	size_t size;
	switch (acquire) {
	case ACQUIRE_IMAGE_PEDESTALS:
	case ACQUIRE_LINING_PEDESTALS:
		std::fill( data_.memory_ + SCANNER_MEMORY,
			data_.memory_ + SCANNER_MEMORY_ALL, 0);
		buffers_.image_buffer =
			reinterpret_cast<char*>(data_.memory_ + SCANNER_MEMORY);
		size = SCANNER_MEMORY_PART;
		break;
	case ACQUIRE_IMAGE:
		std::fill( data_.memory_, data_.memory_ + SCANNER_MEMORY, 0);
		buffers_.image_buffer = reinterpret_cast<char*>(data_.memory_);
		size = params.acquisition.memory_size;
		data_.memory_size_ = params.acquisition.memory_size;
		data_.image_height_ = params.acquisition.image_height;
		break;
	default:
		break;
	}
	com = Commands::create(size);

	try {
		write_command(com);
	}
	catch (const Exception& ex) {
		set_error(ex);
		return false;
	}
	return true;
}

bool
Manager::acquisition_finish(const AcquisitionParameters& params)
{
	{
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.process_ = PROCESS_PARKING;
	}
	signal_update_();

	// move reverse
	try {
		Command* com = Commands::create( params.acquisition.movement_reverse,
			params.movement_type, DIRECTION_REVERSE);
		write_command(com);
		OFLOG_DEBUG( app.log, "Scanner movement reverse started");
	}
	catch (const Exception& ex) {
		set_error(ex);
		return false;
	}

	for ( int i = 0; i < 101; i++) {
		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.progress_ = i / 100.0;
		}
		signal_update_();
		Glib::usleep(params.acquisition.movement_reverse.time * 10000);
	}

	try {
		Command* com = Commands::create(COMMAND_STEPPERS_STOP);
		write_command(com);

		com = Commands::create(COMMAND_ALTERA_RESET);
		write_command(com);
		OFLOG_DEBUG( app.log, "Scanner movement reverse finished");
	}
	catch (const Exception& ex) {
		set_error(ex);
		return false;
	}

	return true;
}

bool
Manager::acquisition_abort( const AcquisitionParameters& params, bool)
{
	{
		Glib::Mutex::Lock lock(mutex_);
		stop_ = false;
		state_.manager_state_.set_process_aborted();
	}
	// move reverse
	try {
		Command* com = Commands::create( params.acquisition.movement_reverse,
			params.movement_type, DIRECTION_REVERSE);
		write_command(com);

		for ( int i = 0; i < 101; i++) {
			{
				Glib::Mutex::Lock lock(mutex_);
				state_.manager_state_.progress_ = i / 100.0;
			}
			signal_update_();
			Glib::usleep(params.acquisition.movement_reverse.time * 20000);
		}

		com = Commands::create(COMMAND_STEPPERS_STOP);
		write_command(com);

		com = Commands::create(COMMAND_ALTERA_RESET);
		write_command(com);

		{
			Glib::Mutex::Lock lock(mutex_);
			state_.manager_state_.set_aborted();
		}
		cond_back_.signal();
		return true;
	}
	catch (const Error& err) {
		set_error(err);
		return false;
	}

	return true;
}

bool
Manager::acquire_data(AcquireType acquire) throw(bool)
{
	size_t left, size;
	switch (acquire) {
	case ACQUIRE_IMAGE:
		size = left = data_.memory_size_;
		break;
	case ACQUIRE_IMAGE_PEDESTALS:
	case ACQUIRE_LINING_PEDESTALS:
		size = left = SCANNER_MEMORY_PART;
		break;
	default:
		break;
	}

	try {
		// readout full amount of memory
		size_t sum = 0, nread;

		while (left) {
			nread = readn( buffers_.image_buffer, SCANNER_FT245_BUFFER_SIZE);

			buffers_.image_buffer += nread;

			if (acquire == ACQUIRE_IMAGE) {
				Glib::Mutex::Lock lock(mutex_);
				if (stop_) {
					OFLOG_DEBUG( app.log,
						"Data acquisition has been interupted, data left"
						<< left << " bytes");
					throw true;
					state_.manager_state_.set_process_aborted();
					cond_back_.signal();
					return false;
				}
			}
			left -= nread;
			sum += nread;

			switch (acquire) {
			case ACQUIRE_IMAGE:
				if (!(sum % (size >> 3))) {
					{
						Glib::Mutex::Lock lock(mutex_);
						state_.manager_state_.progress_ = double(sum) / size;
					}
					signal_update_();
				}
				break;
			case ACQUIRE_IMAGE_PEDESTALS:
			case ACQUIRE_LINING_PEDESTALS:
				break;
			default:
				break;
			}
		}
	}
	catch (const Exception& ex) {
		set_error(ex);
		return false;
	}

	return true;
}

bool
Manager::acquire_pedestals( const AcquisitionParameters& params, AcquireType acquire,
	guint8 arg)
{
	try {
		Command* com = Commands::create( COMMAND_ALTERA_START, '1');
		write_command(com);
		Glib::usleep(50000);

		if (!acquisition_start( params, acquire))
			return false;

		if (!acquire_data(acquire))
			return false;
	}
	catch (const Exception& ex) {
		set_error(ex);
		return false;
	}

	data_.reconstruct( acquire, arg);

	return true;
}

bool
Manager::set_temperature_control(bool control)
{
	if (!thread_run_ && thread_back_) {
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_COMMANDS;
		cond_run_.wait(mutex_);

		state_.temperature_control_ = control;
		if (control) {
			OFLOG_DEBUG( app.log, "Temperature control enabled");
		}
		else {
			OFLOG_DEBUG( app.log, "Temperature control disabled");
			regulator_.reset();
		}

		regulator_.code_value_changed_ = control;

		state_.manager_state_.run_ = RUN_BACKGROUND;
		cond_back_.signal();
		return true;
	}

	return false;
}

bool
Manager::set_temperature_margins( double temperature, double spread)
{
	if (!thread_run_ && thread_back_) {
		Glib::Mutex::Lock lock(mutex_);
		state_.manager_state_.run_ = RUN_COMMANDS;
		cond_run_.wait(mutex_);

		state_.temperature_average_ = temperature;
		state_.temperature_spread_ = spread;
		regulator_.code_value_changed_ = true;
		regulator_.set_margins( temperature, spread);

		state_.manager_state_.run_ = RUN_BACKGROUND;
		cond_back_.signal();
		return true;
	}

	return false;
}

std::string
Manager::id() throw(Exception)
{
	Command* com = Commands::create(COMMAND_ID);
	write_command(com);

	readn( buffers_.com.buf, SCANNER_WELCOME);

	std::string id_str( buffers_.com.buf, buffers_.com.buf + SCANNER_WELCOME);
	if (id_str.find(id_template)) {
		throw Exception(_("Wrong scanner id."));
	}

	return id_str;
}

void
Manager::handshake() throw(Exception)
{
	Command* com = Commands::create(COMMAND_HANDSHAKE);
	write_command(com);

	readn( buffers_.com.buf, SCANNER_WELCOME);

	std::string welcome_str( buffers_.com.buf, buffers_.com.buf + SCANNER_WELCOME);
	if (welcome_str.compare(handshake_message)) {
		throw Exception(_("Wrong handshake response."));
	}
}

double
Manager::temperature() throw(Exception)
{
	Command* com = Commands::create(COMMAND_TEMPERATURE);
	write_command(com);

	readn( buffers_.com.buf, 6);

	return regulator_.temperature(AdcCount( buffers_.com.data[2],
		buffers_.com.data[3]));
}

void
Manager::set_error(const Error& err)
{
	Glib::Mutex::Lock lock(mutex_);
	stop_ = true;

	Glib::ustring what, todo;
	switch (err.code()) {
	case ENOENT:
		what = Glib::ustring::compose( _("The device \"%1\" doesn't exist."),
			device_name);
		todo = Glib::ustring(_("Check the USB cable connection between the" \
			" PC and the device, and check if the scanner" \
			" power supply is turned on."));
		break;
	case EACCES:
		what = Glib::ustring::compose(
			_("The user \"%1\" has no permission to open scanner device."),
			Glib::get_user_name());
		todo = Glib::ustring(
			_("Check your permissions to open, read and write" \
			" to the scanner device."));
		break;
	case EBUSY:
	case ETXTBSY:
		what = Glib::ustring::compose(
			_("The device \"%1\" has been already busy."), device_name);
		todo = Glib::ustring(_("Close other programs which use this" \
			" device, and try to repeat the operation."));
	case EBADF:
		what = _("The device has invalid file descriptor.");
		break;
	case EINTR:
		what = _("Command interrupted by a signal.");
		break;
	case EINVAL:
		what = _("Invalid function argument.");
		break;
	case ENOMEM:
		what = _("The system cannot allocate more virtual"
			" memory because it capacity is full.");
		break;
	case EAGAIN:
		what = _("Resource temporarily unavailable;"
			" the call might work if you try again later.");
		break;
	case EFAULT:
		what = _("You passed in a pointer to bad memory.");
		break;
	case EIO:
		what = _("Input/output error occured.");
		break;
	case EISDIR:
		what = _("File descriptor is attached to a directory.");
		break;
	case ENOSPC:
		what = _("No space left on device.");
		break;
	case EPIPE:
		what = _("There is no process reading from the other end of a pipe.");
		break;
	case EEXIST: case EFBIG: case ELOOP: case EMFILE: case ENODEV:
	case ENOTDIR: case ENXIO: case EOVERFLOW: case EPERM: case EROFS:
		what = _("Unexpected error.");
		todo = Glib::ustring(
			_("The scanner device can't execute this command. " \
			"Please, turn off the device and try again."));
		break;
	default:
		what = err.what();
	}

	state_.what_ = what;
	state_.todo_ = todo;
	state_.manager_state_.set_error();
	cond_back_.signal();
}

void
Manager::load_preferences(const std::string& id)
{
	Glib::Mutex::Lock lock(mutex_);

	state_.load(id);
	
	regulator_.set_margins( state_.temperature_average_,
		state_.temperature_spread_);
	regulator_.temperature_codes_[1] = state_.peltier_code_;
}

void
Manager::save_preferences(const std::string& id)
{
	Glib::Mutex::Lock lock(mutex_);
	state_.save(id);
}

std::vector<Command*>
Manager::get_lining_commands() const
{
	std::vector<Command*> coms;

	// Fills vector if only run thread disabled and backgroud thread is active
	if (!thread_run_ && thread_back_) {
		for ( AssemblyConstIter it = data_.assembly_.begin();
			it != data_.assembly_.end(); ++it) {
			Command* com = Commands::create( it->code, it->lining);
			coms.push_back(com);
		}
	}
	else
		OFLOG_DEBUG( app.log, "Lining data is blocked by a run thread");

	return coms;
}

sigc::connection
Manager::connect(const sigc::slot< bool, Glib::IOCondition>& slot)
{
	return Glib::signal_io().connect( slot, fd_, Glib::IO_HUP | Glib::IO_NVAL);
}

SharedManager
Manager::instance()
{
	if (!instance_) {
		instance_ = SharedManager(new Manager());
	}
	return instance_;
}

void
Manager::join_run_thread()
{
	if (thread_run_) {
		thread_run_->join();
		thread_run_ = 0;
		OFLOG_DEBUG( app.log, "Run thread has been joined");
	}
	else
		OFLOG_DEBUG( app.log, "Run thread == 0");
}

void
Manager::join_data_thread()
{
	if (data_.thread_) {
		data_.thread_->join();
		data_.thread_ = 0;
		OFLOG_DEBUG( app.log, "Data thread has been joined");
	}
	else
		OFLOG_DEBUG( app.log, "Data thread == 0");
}

bool
Manager::io_handler(Glib::IOCondition io_condition)
{
	std::cout << "disconnect" << std::endl;
	stop(true);

	return false;
}

State
Manager::get_state()
{
	State state;
	{
		Glib::Mutex::Lock lock(mutex_);
		state = state_;
	}
	return state;
}

std::vector<guint>
Manager::current_broken_strips() const
{
	std::vector<guint> strips;

	if (!data_.thread_)
		strips = data_.form_bad_strips(data_.width_type_);

	return strips;
}

Glib::Dispatcher&
Manager::signal_update()
{
	return signal_update_;
}

} // namespace Scanner

} // namespace ScanAmati
