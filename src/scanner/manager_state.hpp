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

namespace ScanAmati {

enum RunType {
	RUN_NONE,
	RUN_BACKGROUND,
	RUN_INITIATION,
	RUN_IMAGE_ACQUISITION,
	RUN_IMAGE_RECONSTRUCTION,
	RUN_COMMANDS,
	RUN_LINING_ACQUISITION
};

enum ProcessType {
	PROCESS_NONE,
	PROCESS_START,
	PROCESS_ERROR,
	PROCESS_EXPOSURE,
	PROCESS_PARKING,
	PROCESS_ACQUISITION,
	PROCESS_ABORTED,
	PROCESS_FINISH
};

namespace Scanner {

class ManagerState {
friend class Manager;

public:
	ManagerState();
	void set_aborted();
	void set_error();
	void set_process_finish();
	void set_process_aborted();
	bool device_connected() const { return connected_; }
	bool device_disconnected() const { return !connected_; }
	double progress() const { return progress_; }
	RunType run() const { return run_; }
	ProcessType process() const { return process_; }
	bool process_finished() const { return process_ == PROCESS_FINISH; }
	bool process_error() const { return process_ == PROCESS_ERROR; }
	bool process_aborted() const { return process_ == PROCESS_ABORTED; }
	bool acquisition_finished() const;

private:
	double progress_;
	RunType run_;
	ProcessType process_;
	bool connected_;
};

inline
ManagerState::ManagerState()
	:
	progress_(0.0),
	run_(RUN_NONE),
	process_(PROCESS_NONE),
	connected_(false)
{
}

inline
void
ManagerState::set_error()
{
	progress_ = 0;
	run_ = RUN_BACKGROUND;
	process_ = PROCESS_ERROR;
}

inline
void
ManagerState::set_aborted()
{
	progress_ = 0;
	run_ = RUN_BACKGROUND;
	process_ = PROCESS_ABORTED;
}

inline
void
ManagerState::set_process_aborted()
{
	progress_ = 0;
	process_ = PROCESS_ABORTED;
}

inline
void
ManagerState::set_process_finish()
{
	progress_ = 0;
	process_ = PROCESS_FINISH;
}

inline
bool
ManagerState::acquisition_finished() const
{
	return (run_ == RUN_IMAGE_ACQUISITION && process_finished());
}

} // namespace Scanner

} // namespace ScanAmati
