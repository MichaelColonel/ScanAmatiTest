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

// files from src directory begin
#include "dcmtk_defines.hpp"
#include "exceptions.hpp"
// files from src directory begin

#include "utils.hpp"

namespace ScanAmati {

namespace DICOM {

struct Server;

class UserCommand {
public:
	UserCommand( const Server&, unsigned int retrieve_port = 0,
		bool ignore = true) throw(Exception);
	virtual ~UserCommand();

	struct StoreCallbackData {
		char* imageFileName;
		DcmFileFormat* dcmff;
		T_ASC_Association* assoc;
	};

protected:
	T_ASC_Network *network_;
	T_ASC_Parameters *parameters_;
};

class EchoCommand : public UserCommand {
public:
	EchoCommand(const Server&) throw(Exception);
	virtual ~EchoCommand();
	bool run() throw(Exception);
};

class StoreCommand : public UserCommand {
public:
	StoreCommand(const Server&) throw(Exception);
	virtual ~StoreCommand();
	bool run( Dataset* dataset,
		DIMSE_StoreUserCallback callback) throw(Exception);
protected:
	T_ASC_Association *association_;
};

class MoveCommand : public UserCommand {
public:
	enum QueryModel {
		MODEL_PATIENT_ROOT = 0,
		MODEL_STUDY_ROOT = 1,
		MODEL_PATIENT_STUDY_ONLY = 2
	};
	MoveCommand( const Server&,
		QueryModel query_model = MODEL_PATIENT_ROOT) throw(Exception);
	virtual ~MoveCommand();
	bool run( const DcmDataset& query, const char* filename = 0,
		DIMSE_StoreProviderCallback callback = 0) throw(Exception);
protected:
	T_ASC_Association *association_;
	QueryModel query_model_;
};

} // namespace DICOM

} // namespace ScanAmati
