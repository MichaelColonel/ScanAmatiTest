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

#include <glibmm/i18n.h>

// files from src directory begin
#include "application.hpp"
#include "global_strings.hpp"
// files from src directory begin

#include "server.hpp"
#include "user_commands.hpp"

namespace {

typedef ScanAmati::DICOM::UserCommand::StoreCallbackData StoreCallbackData;

OFLogger& log = ScanAmati::app.log;

const char* cmove_filename = 0;

DIMSE_StoreProviderCallback store_provider_callback;

struct CallbackInfo {
	T_ASC_Association *assoc;
	T_ASC_PresentationContextID presId;
};

const struct QuerySyntax {
	const char* find_syntax;
	const char* move_syntax;
} query_syntax[3] = {
	{ UID_FINDPatientRootQueryRetrieveInformationModel,
		UID_MOVEPatientRootQueryRetrieveInformationModel },
	{ UID_FINDStudyRootQueryRetrieveInformationModel,
		UID_MOVEStudyRootQueryRetrieveInformationModel },
    { UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel,
		UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel }
};

void
substituteOverrideKeys(DcmDataset*)
{
	return; // nothing to do
}

void
moveCallback( void*, T_DIMSE_C_MoveRQ*, int count, T_DIMSE_C_MoveRSP* response)
{
    OFString temp_str;
    OFLOG_DEBUG( log, "Move Response " << count << ":" << std::endl
		<< DIMSE_dumpMessage( temp_str, *response, DIMSE_INCOMING));
}

OFBool
isaListMember( OFList<OFString>& lst, OFString& s)
{
	OFListIterator(OFString) cur = lst.begin();
	OFListIterator(OFString) end = lst.end();

	OFBool found = OFFalse;

	while (cur != end && !found) {
		found = (s == *cur);
		++cur;
	}
	return found;
}

OFCondition
addPresentationContext( T_ASC_Parameters *params,
	int presentationContextId, const OFString& abstractSyntax,
	const OFString& transferSyntax,
	T_ASC_SC_ROLE proposedRole = ASC_SC_ROLE_DEFAULT)
{
	const char* c_p = transferSyntax.c_str();
	OFCondition cond = ASC_addPresentationContext(params, presentationContextId,
		abstractSyntax.c_str(), &c_p, 1, proposedRole);
	return cond;
}

OFCondition
addPresentationContext( T_ASC_Parameters *params,
	T_ASC_PresentationContextID pid, const char* abstractSyntax,
	E_TransferSyntax networkTransferSyntax = EXS_Unknown)
{
	/*
	** We prefer to use Explicitly encoded transfer syntaxes.
	** If we are running on a Little Endian machine we prefer
	** LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
	** Some SCP implementations will just select the first transfer
	** syntax they support (this is not part of the standard) so
	** organise the proposed transfer syntaxes to take advantage
	** of such behaviour.
	**
	** The presentation contexts proposed here are only used for
	** C-FIND and C-MOVE, so there is no need to support compressed
	** transmission.
	*/

	const char* transferSyntaxes[] = { 0, 0, 0 };
	int numTransferSyntaxes = 0;

	switch (networkTransferSyntax) {
	case EXS_LittleEndianImplicit:
		/* we only support Little Endian Implicit */
		transferSyntaxes[0]  = UID_LittleEndianImplicitTransferSyntax;
		numTransferSyntaxes = 1;
		break;
	case EXS_LittleEndianExplicit:
		/* we prefer Little Endian Explicit */
		transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
		transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
		transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
		numTransferSyntaxes = 3;
		break;
	case EXS_BigEndianExplicit:
		/* we prefer Big Endian Explicit */
		transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
		transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
		transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
		numTransferSyntaxes = 3;
		break;
	default:
		/* We prefer explicit transfer syntaxes.
		* If we are running on a Little Endian machine we prefer
		* LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
		*/

		// gLocalByteOrder defined in dcxfer.h
		if (gLocalByteOrder == EBO_LittleEndian) {
			transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
			transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
		}
		else {
			transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
			transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
		}

		transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
		numTransferSyntaxes = 3;
		break;
	}

	return ASC_addPresentationContext( params, pid, abstractSyntax,
		transferSyntaxes, numTransferSyntaxes);
}

OFCondition
addPresentationContext( T_ASC_Parameters *params,
	int presentationContextId, const OFString& abstractSyntax,
	const OFList<OFString>& transferSyntaxList,
	T_ASC_SC_ROLE proposedRole = ASC_SC_ROLE_DEFAULT)
{
	// create an array of supported/possible transfer syntaxes
	const char **transferSyntaxes = new const char*[transferSyntaxList.size()];
	int transferSyntaxCount = 0;
	OFListConstIterator(OFString) s_cur = transferSyntaxList.begin();
	OFListConstIterator(OFString) s_end = transferSyntaxList.end();

	while (s_cur != s_end) {
		transferSyntaxes[transferSyntaxCount++] = (*s_cur).c_str();
		++s_cur;
	}

	OFCondition cond = ASC_addPresentationContext( params,
		presentationContextId, abstractSyntax.c_str(),
		transferSyntaxes, transferSyntaxCount, proposedRole);

	delete [] transferSyntaxes;
	return cond;
}

OFCondition
addStoragePresentationContexts( T_ASC_Parameters *params,
	OFList<OFString>& sopClasses)
{
	/*
	 * Each SOP Class will be proposed in two presentation contexts (unless
	 * the opt_combineProposedTransferSyntaxes global variable is true).
	 * The command line specified a preferred transfer syntax to use.
	 * This prefered transfer syntax will be proposed in one
	 * presentation context and a set of alternative (fallback) transfer
	 * syntaxes will be proposed in a different presentation context.
	 *
	 * Generally, we prefer to use Explicitly encoded transfer syntaxes
	 * and if running on a Little Endian machine we prefer
	 * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
	 * Some SCP implementations will just select the first transfer
	 * syntax they support (this is not part of the standard) so
	 * organise the proposed transfer syntaxes to take advantage
	 * of such behaviour.
	 */


	// Which transfer syntax was preferred on the command line
	OFString preferredTransferSyntax;

	/* gLocalByteOrder is defined in dcxfer.h */
	if (gLocalByteOrder == EBO_LittleEndian) {
		/* we are on a little endian machine */
		preferredTransferSyntax = UID_LittleEndianExplicitTransferSyntax;
	} else {
		/* we are on a big endian machine */
		preferredTransferSyntax = UID_BigEndianExplicitTransferSyntax;
	}

	OFListIterator(OFString) s_cur;
	OFListIterator(OFString) s_end;

	OFList<OFString> fallbackSyntaxes;
	// - If little endian implicit is preferred, we don't need any fallback syntaxes
	//   because it is the default transfer syntax and all applications must support it.
	// - If MPEG2 MP@ML/HL is preferred, we don't want to propose any fallback solution
	//   because this is not required and we cannot decompress the movie anyway.
	fallbackSyntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
	fallbackSyntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
	fallbackSyntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);
	// Remove the preferred syntax from the fallback list
	fallbackSyntaxes.remove(preferredTransferSyntax);

	// create a list of transfer syntaxes combined from the preferred and fallback syntaxes
	OFList<OFString> combinedSyntaxes;
	s_cur = fallbackSyntaxes.begin();
	s_end = fallbackSyntaxes.end();
	combinedSyntaxes.push_back(preferredTransferSyntax);

	while (s_cur != s_end) {
		if (!isaListMember( combinedSyntaxes, *s_cur))
			combinedSyntaxes.push_back(*s_cur);
		++s_cur;
	}

	// add the (short list of) known storage SOP classes to the list
	// the array of Storage SOP Class UIDs comes from dcuid.h
	for (int i = 0; i < numberOfDcmShortSCUStorageSOPClassUIDs; i++)
		sopClasses.push_back(dcmShortSCUStorageSOPClassUIDs[i]);

	// thin out the SOP classes to remove any duplicates
	OFList<OFString> sops;
	s_cur = sopClasses.begin();
	s_end = sopClasses.end();
	while (s_cur != s_end) {
		if (!isaListMember( sops, *s_cur)) {
			sops.push_back(*s_cur);
		}
		++s_cur;
	}

	// add a presentations context for each SOP class / transfer syntax pair
	OFCondition cond = EC_Normal;
	int pid = 1; // presentation context id
	s_cur = sops.begin();
	s_end = sops.end();

	while (s_cur != s_end && cond.good()) {
		if (pid > 255) {
			OFLOG_ERROR( log, "Too many presentation contexts");
			return ASC_BADPRESENTATIONCONTEXTID;
		}

		// SOP class with preferred transfer syntax
		cond = addPresentationContext( params, pid, *s_cur, preferredTransferSyntax);
		pid += 2;   /* only odd presentation context id's */

		if (fallbackSyntaxes.size() > 0) {
			if (pid > 255) {
				OFLOG_ERROR( log, "Too many presentation contexts");
				return ASC_BADPRESENTATIONCONTEXTID;
			}

			// SOP class with fallback transfer syntax
			cond = addPresentationContext( params, pid, *s_cur, fallbackSyntaxes);
			pid += 2; /* only odd presentation context id's */
		}
		++s_cur;
	}

	return cond;
}

OFCondition
acceptSubAssoc( T_ASC_Network* net, T_ASC_Association** assoc)
{
	const char* knownAbstractSyntaxes[] = { UID_VerificationSOPClass };

	const char* transferSyntaxes[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int numTransferSyntaxes;

	OFCondition cond = ASC_receiveAssociation( net, assoc, ASC_DEFAULTMAXPDU);
	if (cond.good()) {
		// gLocalByteOrder defined in dcxfer.h
		if (gLocalByteOrder == EBO_LittleEndian) {
			transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
			transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
		}
		else {
			transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
			transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
		}

		transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
		numTransferSyntaxes = 3;
	}

	/* accept the Verification SOP Class if presented */
	cond = ASC_acceptContextsWithPreferredTransferSyntaxes( (*assoc)->params,
		knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes), transferSyntaxes,
		numTransferSyntaxes);
/*
	if (cond.good()) {
		// the array of Storage SOP Class UIDs comes from dcuid.h
		cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
			(*assoc)->params,
			dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs,
			transferSyntaxes, numTransferSyntaxes);
	}
*/
	if (cond.good())
		cond = ASC_acknowledgeAssociation(*assoc);
	if (cond.bad()) {
		ASC_dropAssociation(*assoc);
		ASC_destroyAssociation(assoc);
	}

	return cond;
}

OFCondition
echoSCP( T_ASC_Association* assoc, T_DIMSE_Message* msg,
	T_ASC_PresentationContextID presID)
{
	OFString temp_str;
	OFLOG_DEBUG( log, "Received Echo Request");
	OFLOG_DEBUG( log,
		DIMSE_dumpMessage( temp_str, msg->msg.CEchoRQ, DIMSE_INCOMING));

	// the echo succeeded !!
	OFCondition cond = DIMSE_sendEchoResponse( assoc, presID, &msg->msg.CEchoRQ,
		STATUS_Success, NULL);

	if (cond.bad()) {
		OFLOG_DEBUG( log,
			"Echo SCP Failed: " << DimseCondition::dump( temp_str, cond));
	}

	return cond;
}

// That callback MUST BE in GUI
void
storeSCPCallback(
	/* in */
	void *callbackData,
	T_DIMSE_StoreProgress* progress,    /* progress state */
	T_DIMSE_C_StoreRQ* req,             /* original store request */
	char* imageFileName,
	DcmDataset** imageDataSet, /* being received into */
	/* out */
	T_DIMSE_C_StoreRSP *rsp,            /* final store response */
	DcmDataset **statusDetail)
{
	DIC_UI sopClass;
	DIC_UI sopInstance;

	// dump some information if required (depending on the progress state)
	// We can't use oflog for the pdu output, but we use a special logger for
	// generating this output. If it is set to level "INFO" we generate the
	// output, if it's set to "DEBUG" then we'll assume that there is debug output
	// generated for each PDU elsewhere.
	switch (progress->state) {
	case DIMSE_StoreBegin:
		std::cout << "RECV: ";
		break;
	case DIMSE_StoreEnd:
		std::cout << std::endl;
		break;
	default:
		std::cout << '.';
		break;
	}
	std::cout.flush();

	if (progress->state == DIMSE_StoreEnd) {
		*statusDetail = NULL;    /* no status detail */

		// could save the image somewhere else, put it in database, etc

		/*
		* An appropriate status code is already set in the resp structure, it need not be success.
		* For example, if the caller has already detected an out of resources problem then the
		* status will reflect this.  The callback function is still called to allow cleanup.
		*/

		if ((imageDataSet != NULL) && (*imageDataSet != NULL)) {
			StoreCallbackData* cbdata = static_cast<StoreCallbackData*>(callbackData);

			/* create full path name for the output file */
			OFString ofname;
			if (cmove_filename)
				ofname = OFString(cmove_filename);
			else
				OFStandard::combineDirAndFilename( ofname, "/tmp/",
					cbdata->imageFileName, OFTrue);

			E_TransferSyntax xfer = (*imageDataSet)->getOriginalXfer();

			OFCondition cond = cbdata->dcmff->saveFile( ofname.c_str(), xfer,
				EET_ExplicitLength, EGL_withGL, EPD_withoutPadding, 0, 0,
				EWM_fileformat);

			if (cond.bad()) {
				OFLOG_DEBUG( log, "Cannot write DICOM file: " << ofname);
				rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
			}


			/* should really check the image to make sure it is consistent,
			* that its sopClass and sopInstance correspond with those in
			* the request.
			*/
			if (rsp->DimseStatus == STATUS_Success) {
				/* which SOP class and SOP instance ? */
/*
				if (!DU_findSOPClassAndInstanceInDataSet( *imageDataSet,
					sopClass, sopInstance, OFFalse)) {

					OFLOG_DEBUG( log, "Bad DICOM file: " << imageFileName);
					rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
				}
				else if (strcmp( sopClass, req->AffectedSOPClassUID) != 0) {
					rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
				}
				else if (strcmp( sopInstance, req->AffectedSOPInstanceUID) != 0) {
					rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
				}
*/
			}
		}
    }
}

OFCondition
storeSCP( T_ASC_Association* assoc, T_DIMSE_Message* msg,
	T_ASC_PresentationContextID presID)
{
	OFCondition cond = EC_Normal;
	T_DIMSE_C_StoreRQ* req;
	char imageFileName[2048];

	req = &msg->msg.CStoreRQ;

	sprintf( imageFileName, "%s.%s",
		dcmSOPClassUIDToModality(req->AffectedSOPClassUID),
		req->AffectedSOPInstanceUID);

	OFString temp_str;
	OFLOG_DEBUG( log, "Received Store Request: MsgID " << req->MessageID
		<< ", (" << dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "OT")
		<< ")");
	OFLOG_DEBUG( log,
		DIMSE_dumpMessage( temp_str, *req, DIMSE_INCOMING, NULL, presID));

	StoreCallbackData callbackData;
	callbackData.assoc = assoc;
	callbackData.imageFileName = imageFileName;
	DcmFileFormat dcmff;
	callbackData.dcmff = &dcmff;

	// store SourceApplicationEntityTitle in metaheader
	if (assoc && assoc->params) {
		const char *aet = assoc->params->DULparams.callingAPTitle;
		if (aet)
			dcmff.getMetaInfo()->putAndInsertString(
				DCM_SourceApplicationEntityTitle, aet);
    }

	DcmDataset *dset = dcmff.getDataset();

	cond = DIMSE_storeProvider( assoc, presID, req, NULL, OFTrue,
		&dset, store_provider_callback, reinterpret_cast<void*>(&callbackData),
		DIMSE_BLOCKING, 0);

	if (cond.bad()) {
		OFLOG_DEBUG( log, "Store SCP Failed: " <<
			DimseCondition::dump( temp_str, cond));
		/* remove file */
		if (strcmp( imageFileName, NULL_DEVICE_NAME) != 0)
			unlink(imageFileName);
    }

    return cond;
}

OFCondition
subOpSCP(T_ASC_Association** subAssoc)
{
	T_DIMSE_Message msg;
	T_ASC_PresentationContextID presID;

	if (!ASC_dataWaiting( *subAssoc, 0)) /* just in case */
		return DIMSE_NODATAAVAILABLE;

	OFCondition cond = DIMSE_receiveCommand( *subAssoc, DIMSE_BLOCKING, 5,
		&presID, &msg, NULL);

	if (cond == EC_Normal) {
		switch (msg.CommandField) {
		case DIMSE_C_STORE_RQ:
			cond = storeSCP(*subAssoc, &msg, presID);
			break;
		case DIMSE_C_ECHO_RQ:
			cond = echoSCP(*subAssoc, &msg, presID);
			break;
		default:
			cond = DIMSE_BADCOMMANDTYPE;
			OFLOG_DEBUG( log, "Cannot handle command: 0x"
				<< std::hex << static_cast<unsigned>(msg.CommandField));
			break;
		}
	}
	/* clean up on association termination */
	if (cond == DUL_PEERREQUESTEDRELEASE) {
		cond = ASC_acknowledgeRelease(*subAssoc);
		ASC_dropSCPAssociation(*subAssoc);
		ASC_destroyAssociation(subAssoc);
		return cond;
	}
	else if (cond == DUL_PEERABORTEDASSOCIATION) {
	}
	else if (cond != EC_Normal) {
		OFString temp_str;
		OFLOG_DEBUG( log, "DIMSE failure (aborting sub-association): "
			<< DimseCondition::dump(temp_str, cond));

		// some kind of error so abort the association
		cond = ASC_abortAssociation(*subAssoc);
	}

	if (cond != EC_Normal) {
		ASC_dropAssociation(*subAssoc);
		ASC_destroyAssociation(subAssoc);
	}
	return cond;
}

void
subOpCallback( void * /*subOpCallbackData*/, T_ASC_Network* net,
	T_ASC_Association** subAssoc)
{
	if (net == NULL) return;   /* help no net ! */

	if (*subAssoc == NULL) {
		/* negotiate association */
		acceptSubAssoc( net, subAssoc);
	} else {
		/* be a service class provider */
		subOpSCP(subAssoc);
	}
}

}

namespace ScanAmati {

namespace DICOM {

UserCommand::UserCommand( const Server& server,
	unsigned int retrieve_port, bool ignore) throw(Exception)
	:
	network_(0),
	parameters_(0)
{
    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded()) {
        std::cerr << "Warning: no data dictionary loaded, ";
        std::cerr << "check environment variable: ";
		std::cerr << DCM_DICT_ENVIRONMENT_VARIABLE << std::endl;
		throw Exception(_("Data Dictionary wasn't loaded."));
    }

	if (!ignore) {
		/* if retrieve port is privileged we must be as well */
		if (retrieve_port < 1024) {
			if (geteuid() != 0) {
				OFLOG_DEBUG( app.log, "Cannot listen on port " << retrieve_port << ", insufficient privileges.");
				Glib::ustring msg = Glib::ustring::compose(
					_("Cannot listen on port %1, insufficient privileges."),
					Glib::ustring::format(retrieve_port));
				throw Exception(msg);
			}
		}
	}

	// network for move request and responses
	T_ASC_NetworkRole role = (retrieve_port > 0) ?
		NET_ACCEPTORREQUESTOR : NET_REQUESTOR;

	// network struct, contains DICOM upper layer FSM etc.
	OFCondition cond = ASC_initializeNetwork( role,
		static_cast<int>(retrieve_port), 5, &network_);
	if (cond.bad()) {
		if (app.debug) DimseCondition::dump(cond);
		throw Exception(_("Unable to initialize network."));
	}

	// parameters of association request
	cond = ASC_createAssociationParameters( &parameters_, ASC_DEFAULTMAXPDU);
	if (cond.bad()) {
		if (app.debug) DimseCondition::dump(cond);
		throw Exception(_("Unable to create parameters of the association."));
	}
}

UserCommand::~UserCommand()
{
	if (network_) ASC_dropNetwork(&network_);
}

EchoCommand::EchoCommand(const Server& server) throw(Exception)
	:
	UserCommand(server)
{
	// set calling and called AE titles
	OFCondition cond = ASC_setAPTitles( parameters_, app.ae_title.c_str(),
		server.title.c_str(), NULL);
	if (cond.bad()) {
		if (app.debug) DimseCondition::dump(cond);
		throw Exception(_("Unable to set application entity titles."));
	}

	DIC_NODENAME local_host;
	gethostname( local_host, sizeof(local_host) - 1);

	// the DICOM server accepts connections at hostname:port
	cond = ASC_setPresentationAddresses( parameters_, local_host,
		server.called_address().c_str());
	if (cond.bad()) {
		if (app.debug) DimseCondition::dump(cond);
		throw Exception(_("Unable to set presentation addresses."));
	}
}

EchoCommand::~EchoCommand()
{
}

bool
EchoCommand::run() throw(Exception)
{
	// list of transfer syntaxes, only a single entry here
	const char* ts[] = { UID_LittleEndianImplicitTransferSyntax };

	// add presentation context to association request
	OFCondition cond = ASC_addPresentationContext( parameters_,
		1, UID_VerificationSOPClass, ts, 1);

	// request DICOM association
	T_ASC_Association *assoc;

	DIC_US status; // DIMSE status of C-ECHO-RSP will be stored here

	cond = ASC_requestAssociation( network_, parameters_, &assoc);
	if (cond.good()) {
		if (ASC_countAcceptedPresentationContexts(parameters_) == 1) {
			// the remote SCP has accepted the Verification Service Class
			DIC_US id = assoc->nextMsgID++; // generate next message ID
			DcmDataset *sd = NULL; // status detail will be stored here

			// send C-ECHO-RQ and handle response
			OFCondition result = DIMSE_echoUser( assoc, id, DIMSE_BLOCKING,
				0, &status, &sd);

			if (sd) {
				OFLOG_DEBUG( app.log, "Status Detail:" << std::endl
					<< DcmObject::PrintHelper(*sd));
			}

			delete sd; // we don't care about status detail
		}
	}
	else if (cond.bad()) {
		if (cond == DUL_ASSOCIATIONREJECTED) {
			T_ASC_RejectParameters rej;

			ASC_getRejectParameters( parameters_, &rej);
			OFLOG_DEBUG( app.log, "Association Rejected.");
			if (app.debug) ASC_printRejectParameters( stderr, &rej);
			throw Exception(_("Association rejected."));
		}
		else {
			OFLOG_DEBUG( app.log, "Association Request Failed.");
			if (app.debug) DimseCondition::dump(cond);
			throw Exception(_("Failed to establish association."));
		}
	}

	ASC_releaseAssociation(assoc); // release association
	ASC_destroyAssociation(&assoc); // delete assoc structure

	return static_cast<bool>(!status);
}

StoreCommand::StoreCommand(const Server& server) throw(Exception)
	:
	UserCommand(server),
	association_(0)
{
	OFList<OFString> sopClassUIDList; // the list of sop classes

	// set calling and called AE titles
	OFCondition cond = ASC_setAPTitles( parameters_, app.ae_title.c_str(),
		server.title.c_str(), NULL);
	if (cond.bad()) {
		parameters_ = 0;
		if (app.debug) DimseCondition::dump(cond);
		throw Exception(_("Unable to set application entity titles."));
	}

    /* Set the transport layer type (type of network connection) in the params */
    /* strucutre. The default is an insecure connection; where OpenSSL is  */
    /* available the user is able to request an encrypted,secure connection. */
    cond = ASC_setTransportLayerType( parameters_, OFFalse);
    if (cond.bad()) {
        DimseCondition::dump(cond);
        throw Exception(_("Unable to set type of network connection."));
    }

	DIC_NODENAME local_host;
	gethostname( local_host, sizeof(local_host) - 1);
	ASC_setPresentationAddresses( parameters_, local_host,
		server.called_address().c_str());

	cond = addStoragePresentationContexts( parameters_, sopClassUIDList);
	if (cond.bad()) {
		DimseCondition::dump(cond);
		throw Exception(_("Unable to add storage presentation contexts."));
	}

	/* dump presentation contexts if required */
	OFLOG_DEBUG( app.log, "Request Parameters:");
	if (app.debug)
		ASC_dumpParameters( parameters_, std::cout);

	/* create association, i.e. try to establish a network connection to another */
	/* DICOM application. This call creates an instance of T_ASC_Association*. */
	OFLOG_DEBUG( app.log, "Requesting Association");

	cond = ASC_requestAssociation( network_, parameters_, &association_);
	if (cond.bad()) {
		if (cond == DUL_ASSOCIATIONREJECTED) {
			T_ASC_RejectParameters rej;

			ASC_getRejectParameters( parameters_, &rej);
			OFLOG_DEBUG( app.log, "Association Rejected");
			ASC_printRejectParameters(stderr, &rej);
			throw Exception(_("Association rejected."));
		} else {
			OFLOG_DEBUG( app.log, "Association Request Failed");
			DimseCondition::dump(cond);
			throw Exception(_("Association request failed."));
		}
	}

	/* dump the presentation contexts which have been accepted/refused */
	OFLOG_DEBUG( app.log, "Association Parameters Negotiated");
	if (app.debug)
		ASC_dumpParameters( parameters_, std::cout);

	/* count the presentation contexts which have been accepted by the SCP */
	/* If there are none, finish the execution */
	if (ASC_countAcceptedPresentationContexts(parameters_) == 0) {
		OFLOG_DEBUG( app.log, "No Acceptable Presentation Contexts");
		throw Exception(_("No acceptable presentation contexts."));
	}
}

StoreCommand::~StoreCommand()
{
	/* destroy the association, i.e. free memory of T_ASC_Association* structure. This */
	/* call is the counterpart of ASC_requestAssociation(...) which was called above. */

	OFCondition cond = ASC_releaseAssociation(association_); // release association
	if (cond.bad()) {
		DimseCondition::dump(cond);
	}
	
	cond = ASC_destroyAssociation(&association_);
	if (cond.bad()) {
		DimseCondition::dump(cond);
	}
}

bool
StoreCommand::run( Dataset* dataset,
	DIMSE_StoreUserCallback callback) throw(Exception)
{
	DIC_US msgId = association_->nextMsgID++;
	T_ASC_PresentationContextID presId;
	T_DIMSE_C_StoreRQ req;
	T_DIMSE_C_StoreRSP rsp;
	DIC_UI sopClass;
	DIC_UI sopInstance;
	DcmDataset* statusDetail = 0;
	DcmDataset* set = dataset;
	OFCondition cond;

	OFLOG_DEBUG( app.log, "--------------------------");
	OFLOG_DEBUG( app.log, "Sending dataset");

	/* figure out which SOP class and SOP instance is encapsulated in the file */
/*
	if (!DU_findSOPClassAndInstanceInDataSet( set,
		sopClass, sopInstance, OFFalse)) {
		OFLOG_DEBUG( app.log, "No SOP Class & Instance UIDs in dataset");
		throw Exception(_("No SOP class and instance UIDs in dataset."));
	}
*/
	/* figure out which of the accepted presentation contexts should be used */
	DcmXfer filexfer(set->getOriginalXfer());

	/* special case: if the file uses an unencapsulated transfer syntax (uncompressed
	 * or deflated explicit VR) and we prefer deflated explicit VR, then try
	 * to find a presentation context for deflated explicit VR first.
	*/
	if (filexfer.isNotEncapsulated()) {
        filexfer = EXS_DeflatedLittleEndianExplicit;
    }

	if (filexfer.getXfer() != EXS_Unknown)
		presId = ASC_findAcceptedPresentationContextID( association_, sopClass,
			filexfer.getXferID());
    else
		presId = ASC_findAcceptedPresentationContextID( association_, sopClass);

    if (presId == 0) {
        const char *modalityName = dcmSOPClassUIDToModality(sopClass);
        if (!modalityName)
			modalityName = dcmFindNameOfUID(sopClass);
        if (!modalityName)
			modalityName = "unknown SOP class";
        OFLOG_DEBUG( app.log, "No presentation context for: " << modalityName
			<< " " << sopClass);
        throw Exception(_("Presentation context is absent."));
    }

    /* if required, dump general information concerning transfer syntaxes */
	if (app.debug) {
		DcmXfer fileTransfer(set->getOriginalXfer());
		T_ASC_PresentationContext pc;
		ASC_findAcceptedPresentationContext( association_->params, presId, &pc);
        DcmXfer netTransfer(pc.acceptedTransferSyntax);
        OFLOG_DEBUG( app.log, "Transfer: " <<
			dcmFindNameOfUID(fileTransfer.getXferID()) << " -> " <<
			dcmFindNameOfUID(netTransfer.getXferID()));
	}

    /* prepare the transmission of data */
    memset( (char*)&req, 0, sizeof(req));
	req.MessageID = msgId;
	strcpy( req.AffectedSOPClassUID, sopClass);
	strcpy( req.AffectedSOPInstanceUID, sopInstance);
	req.DataSetType = DIMSE_DATASET_PRESENT;
	req.Priority = DIMSE_PRIORITY_LOW;

    /* if required, dump some more general information */
	OFLOG_DEBUG( app.log, "Store SCU RQ: MsgID " << msgId << " , "
		<< "(" << dcmSOPClassUIDToModality(sopClass) << ")");

	/* finally conduct transmission of data */
	cond = DIMSE_storeUser( association_, presId, &req,
		NULL, set, callback, NULL, DIMSE_BLOCKING, 0,
		&rsp, &statusDetail, NULL, dataset->size());

	/*
	 * If store command completed normally, with a status
	 * of success or some warning then the image was accepted.
	 */
	if (cond == EC_Normal &&
		(rsp.DimseStatus == STATUS_Success ||
		DICOM_WARNING_STATUS(rsp.DimseStatus))) {
		;
	}

	/* dump some more general information */
	if (cond == EC_Normal) {
		if (app.debug)
			DIMSE_printCStoreRSP( stdout, &rsp);
	}
	else {
		OFLOG_DEBUG( app.log, "Dataset store failed");
		DimseCondition::dump(cond);
		throw Exception(cond.text());
	}

	/* dump status detail information if there is some */
	if (statusDetail) {
		OFLOG_DEBUG( app.log, "Status Detail: " << std::endl
			<< DcmObject::PrintHelper(*statusDetail));
		delete statusDetail;
    }

    return static_cast<bool>(cond.good());
}

MoveCommand::MoveCommand( const Server& server,
	QueryModel query_model) throw(Exception)
	:
	UserCommand( server, app.port, false),
	association_(0),
	query_model_(query_model)
{
	// set calling and called AE titles
	OFCondition cond = ASC_setAPTitles( parameters_, app.ae_title.c_str(),
		server.title.c_str(), NULL);
	if (cond.bad()) {
		parameters_ = 0;
		if (app.debug) DimseCondition::dump(cond);
		throw Exception(_("Unable to set application entity titles."));
	}

    /* Set the transport layer type (type of network connection) in the params */
    /* strucutre. The default is an insecure connection; where OpenSSL is  */
    /* available the user is able to request an encrypted,secure connection. */
    cond = ASC_setTransportLayerType( parameters_, OFFalse);
    if (cond.bad()) {
        DimseCondition::dump(cond);
        throw Exception(_("Unable to set type of network connection."));
    }

	DIC_NODENAME local_host;
	gethostname( local_host, sizeof(local_host) - 1);
	ASC_setPresentationAddresses( parameters_, local_host,
		server.called_address().c_str());

    /*
     * We also add a presentation context for the corresponding
     * find sop class.
     */
    cond = addPresentationContext( parameters_, 1,
        query_syntax[query_model_].find_syntax);

    cond = addPresentationContext( parameters_, 3,
        query_syntax[query_model_].move_syntax);
    if (cond.bad()) {
		DimseCondition::dump(cond);
		throw Exception(_("Unable to add presentation context."));
	}

	/* dump presentation contexts if required */
	OFLOG_DEBUG( app.log, "Request Parameters:");
	if (app.debug)
		ASC_dumpParameters( parameters_, std::cout);

	/* create association, i.e. try to establish a network connection to another */
	/* DICOM application. This call creates an instance of T_ASC_Association*. */
	OFLOG_DEBUG( app.log, "Requesting Association");

	cond = ASC_requestAssociation( network_, parameters_, &association_);
	if (cond.bad()) {
		if (cond == DUL_ASSOCIATIONREJECTED) {
			T_ASC_RejectParameters rej;

			ASC_getRejectParameters( parameters_, &rej);
			OFLOG_DEBUG( app.log, "Association Rejected");
			ASC_printRejectParameters(stderr, &rej);
			throw Exception(_("Association rejected."));
		} else {
			OFLOG_DEBUG( app.log, "Association Request Failed");
			DimseCondition::dump(cond);
			throw Exception(_("Association request failed."));
		}
	}

	/* dump the presentation contexts which have been accepted/refused */
	OFLOG_DEBUG( app.log, "Association Parameters Negotiated");
	if (app.debug)
		ASC_dumpParameters( parameters_, std::cout);

	/* count the presentation contexts which have been accepted by the SCP */
	/* If there are none, finish the execution */
	if (ASC_countAcceptedPresentationContexts(parameters_) == 0) {
		OFLOG_DEBUG( app.log, "No Acceptable Presentation Contexts");
		throw Exception(_("No acceptable presentation contexts."));
	}
}

MoveCommand::~MoveCommand()
{
}

bool
MoveCommand::run( const DcmDataset& query, const char* filename,
	DIMSE_StoreProviderCallback callback) throw(Exception)
{
	T_ASC_PresentationContextID presId;
	T_DIMSE_C_MoveRQ req;
	T_DIMSE_C_MoveRSP rsp;
	DIC_US msgId = association_->nextMsgID++;
	DcmDataset* rspIds = 0;
	const char* sopClass;
	DcmDataset* statusDetail = 0;
	CallbackInfo callbackData;

	store_provider_callback = callback ? callback : &storeSCPCallback;
	cmove_filename = filename;
	sopClass = query_syntax[query_model_].move_syntax;

	// which presentation context should be used
	presId = ASC_findAcceptedPresentationContextID( association_, sopClass);
    if (presId == 0)
		throw Exception(_("No valid presentation context ID."));

	OFLOG_DEBUG( app.log, "Sending Move Request: MsgID " << msgId);
	DcmDataset* dataset = const_cast<DcmDataset*>(&query);
	OFLOG_DEBUG( app.log, "Request:" << OFendl << DcmObject::PrintHelper(*dataset));

	callbackData.assoc = association_;
	callbackData.presId = presId;

	req.MessageID = msgId;

	strcpy( req.AffectedSOPClassUID, sopClass);
	req.Priority = DIMSE_PRIORITY_MEDIUM;
	req.DataSetType = DIMSE_DATASET_PRESENT;

	/* set the destination to be me */
//	ASC_getAPTitles( association_->params, req.MoveDestination, NULL, NULL);

	OFCondition cond = DIMSE_moveUser( association_, presId, &req, dataset,
		moveCallback, &callbackData, DIMSE_BLOCKING, 0, network_, subOpCallback,
        NULL, &rsp, &statusDetail, &rspIds, OFTrue);

	if (cond == EC_Normal) {
		OFString temp_str;
		OFLOG_DEBUG( app.log, DIMSE_dumpMessage(temp_str, rsp, DIMSE_INCOMING));
		if (rspIds != NULL) {
			OFLOG_DEBUG( app.log, "Response Identifiers:"
				<< std::endl << DcmObject::PrintHelper(*rspIds));
		}
	}
	else {
		OFString temp_str;
		OFLOG_DEBUG( app.log, "Move Request Failed: "
			<< DimseCondition::dump(temp_str, cond));
		throw Exception(_("Move request failed."));
    }
    if (statusDetail) {
        OFLOG_DEBUG( app.log, "Status Detail:"
			<< std::endl << DcmObject::PrintHelper(*statusDetail));
        delete statusDetail;
    }

    if (rspIds)
		delete rspIds;

    return static_cast<bool>(cond.good());
}

} // namespace DICOM

} // namespace ScanAmati
