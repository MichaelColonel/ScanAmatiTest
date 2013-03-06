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

#pragma once

#include <tr1/memory>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

#include <glibmm/fileutils.h>
#include <glibmm/dispatcher.h>
#include <glibmm/thread.h>

#include "temperature_regulator.hpp"
#include "run_arguments.hpp"
#include "state.hpp"
#include "data.hpp"

/* files from src directory begin */
#include "exceptions.hpp"
/* files from src directory end */

namespace ScanAmati {

namespace Scanner {

class Error : public Exception {
public:
	Error(const Exception& ex) : Exception(ex), code_(0) {}
	explicit Error(const Glib::ustring& what, int code = 0)
		: Exception(what), code_(code) {}
	int code() const { return code_; }
private:
	int code_;
};

class Manager;
typedef std::tr1::shared_ptr<Manager> SharedManager;

class Manager : private boost::noncopyable {

public:
	~Manager();
	static SharedManager instance();

	void run( RunType run,
		const AcquisitionParameters& params = AcquisitionParameters());

	void stop(bool stop_everything = false);
	Glib::Dispatcher& signal_update();
	State get_state();
	Data* get_data() { return (data_.thread_) ? 0 : &data_; }
	const Data* get_data() const { return (data_.thread_) ? 0 : &data_; }
	bool run_thread_state() const { return static_cast<bool>(thread_run_); }
	std::vector<Command*> get_lining_commands() const;
	bool set_temperature_control(bool control);
	bool set_temperature_margins( double temperature, double spread);
	void join_run_thread();
	void join_data_thread();
	std::vector<guint> current_broken_strips() const;

protected:
	void run_background();
	void run_initiation();
	void run_image_acquisition(const AcquisitionParameters&);
	void run_reconstruction_loop();
	void run_lining_acquisition(const AcquisitionParameters&);
	void run_commands(const AcquisitionParameters& args);

	void connect_signals();
	bool io_handler(Glib::IOCondition condition);

	void handshake() throw(Exception);
	std::string id() throw(Exception);
	double temperature() throw(Exception);

	bool exposure_start(const AcquisitionParameters&) throw(int);
	bool exposure_finish(const AcquisitionParameters&);
	bool exposure_abort( const AcquisitionParameters&, int);
	bool acquisition_start( const AcquisitionParameters&, AcquireType acquire);
	bool acquisition_finish(const AcquisitionParameters&);
	bool acquisition_abort( const AcquisitionParameters&, bool);
	bool acquire_pedestals( const AcquisitionParameters&, AcquireType type, guint8 arg);
	bool acquire_data(AcquireType type) throw(bool);

	void load_preferences(const std::string& id);
	void save_preferences(const std::string& id);

	void set_error(const Error&);

	virtual void open(const Glib::ustring&) throw(Error);
	virtual void close() throw(Error);
	virtual void check() throw( Exception, Error);
	virtual size_t write( const char*, size_t) throw(Error);
	virtual size_t read( char*, size_t) throw( Exception, Error);
	virtual size_t readn( char*, size_t) throw( Exception, Error);
	void write_command(Command* com) throw(Error);
	void write_command(const CommandSharedPtr& com) throw(Error);

	sigc::connection connect(const sigc::slot< bool, Glib::IOCondition>&);

	sigc::connection connection_io_;

	Glib::Dispatcher signal_update_;
	Glib::Thread* thread_back_; // background thread
	Glib::Thread* thread_run_; // run thread
	Glib::Cond cond_back_;
	Glib::Cond cond_run_;
	Glib::Mutex mutex_; // state mutex

	bool stop_;
	Data data_;
	State state_;
	TemperatureRegulator regulator_;

	struct Buffers {
		Buffers() : image_buffer(0) { com.buf = 0; }
		char* image_buffer;
		union {
			char* buf;
			guint8* data;
		} com;
	} buffers_;
	int fd_;

private:
	Manager();
	static SharedManager instance_;
};

} // namespace Scanner

} // namespace ScanAmati
