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

#include <gtkmm/treeview.h>
#include <gtkmm/stock.h>
#include <glibmm/i18n.h>
#include <iomanip>
#include <ctime>

// files from src directory begin
#include "dicom/summary_information.hpp"

#include "scanner/state.hpp"
#include "scanner/x-ray.hpp"

#include "icon_names.hpp"
// files from src directory end

#include "utils.hpp"

#include "information_notebook.hpp"

#define PATH_XRAY                "0" // x-ray source
#define PATH_XRAY_VOLTAGE      "0:0" // x-ray voltage
#define PATH_XRAY_CURRENT      "0:1" // x-ray current
#define PATH_XRAY_EXPOSURE     "0:2" // x-ray exposure
//#define PATH_XRAY_TIME         "0:3" // x-ray exposure time
#define PATH_XRAY_STATE        "0:3" // x-ray state

#define PATH_TEMPERATURE_CONTROL "1:0" // if temperature control enabled
#define PATH_TEMPERATURE_CURRENT "1:1" // detector current temperature
#define PATH_TEMPERATURE_MARGINS "1:2" // detector temperature margins
#define PATH_TEMPERATURE_PELTIER "1:3" // current TEC value 

#define PATH_DET_ID         "2:0" // detector ID
#define PATH_DET_CAPACITY   "2:1" // detector chips capacity 
#define PATH_DET_CHIP       "2:2" // detector current chip 

#define PATH_PAT_NAME     "0:0" // patient name
#define PATH_PAT_ID       "0:1" // patient id
#define PATH_PAT_SEX      "0:2" // patient sex
#define PATH_PAT_DOB      "0:3" // patient date of birth
#define PATH_PAT_AGE      "0:4" // patient age
#define PATH_PAT_HEIGHT   "0:5" // patient height
#define PATH_PAT_WEIGHT   "0:6" // patient weight

#define PATH_STUD_PHYS    "1:0" // study physician name
#define PATH_STUD_OPER    "1:1" // study operator name
#define	PATH_STUD_DESCR   "1:2" // study description
#define PATH_STUD_ID      "1:3" // study id
#define PATH_STUD_BODY    "1:4" // study body part
#define PATH_STUD_INST    "1:5" // study institution
#define PATH_STUD_DATE    "1:6" // study date
#define PATH_STUD_TIME    "1:7" // study time
#define PATH_STUD_MANU    "1:8" // study manufacturer model name
#define PATH_STUD_DOSE    "1:9" // study dose

#define PATH_STUD_XRAY_VOLTAGE      "2:0" // x-ray voltage
#define PATH_STUD_XRAY_CURRENT      "2:1" // x-ray current
#define PATH_STUD_XRAY_EXPOSURE     "2:2" // x-ray exposure
#define PATH_STUD_XRAY_TIME         "2:3" // x-ray exposure time
#define PATH_STUD_DET_TEMPERATURE   "3:0" // detector temperature

namespace {

// micro Sv (uSv)
Glib::ustring
format_dose_string(double dose)
{
	Glib::ustring text = Glib::ustring::compose( _("%1 \\u00B5Sv"),
		Glib::ustring::format( std::setprecision(1), std::fixed, dose));
	return text;
}
	
} // namespace

namespace ScanAmati {

namespace UI {

struct InfoItem {
	InfoItem();
	InfoItem( const Glib::ustring&, const Glib::ustring&,
		const Glib::ustring& color = "black");
	InfoItem( const Glib::RefPtr<Gdk::Pixbuf>&, const Glib::ustring&,
		const Glib::ustring&, const Glib::ustring& color = "black");
	InfoItem( const Glib::RefPtr<Gdk::Pixbuf>&, const Glib::ustring&,
		const std::vector<InfoItem>&);
	InfoItem(const InfoItem&);
	~InfoItem();
	InfoItem& operator=(const InfoItem&);

	Glib::RefPtr<Gdk::Pixbuf> icon;
	Glib::ustring label;
	Glib::ustring value;
	Glib::ustring color;
	std::vector<InfoItem> children;
};

InfoItem::InfoItem()
	:
	color("black")
{
}

InfoItem::InfoItem( const Glib::ustring& label_,
	const Glib::ustring& value_, const Glib::ustring& color_)
	:
	label(label_),
	value(value_),
	color(color_)
{
}

InfoItem::InfoItem( const Glib::RefPtr<Gdk::Pixbuf>& icon_,
	const Glib::ustring& label_, const Glib::ustring& value_,
	const Glib::ustring& color_)
	:
	icon(icon_),
	label(label_),
	value(value_),
	color(color_)
{
}

InfoItem::InfoItem( const Glib::RefPtr<Gdk::Pixbuf>& icon_,
	const Glib::ustring& label_, const std::vector<InfoItem>& children_)
	:
	icon(icon_),
	label(label_),
	color("black"),
	children(children_)
{
}

InfoItem::InfoItem(const InfoItem& foo)
	:
	icon(foo.icon),
	label(foo.label),
	value(foo.value),
	color(foo.color),
	children(foo.children)
{
}

InfoItem::~InfoItem()
{
}

InfoItem&
InfoItem::operator=(const InfoItem& foo)
{
	if (this == &foo)
		return *this;

	icon = foo.icon;
	label = foo.label;
	value = foo.value;
	color = foo.color;
	children = foo.children;

	return *this;
}

InformationNotebook::InformationNotebook( BaseObjectType* cobject,
	const Glib::RefPtr<Gtk::Builder>& builder)
	:
	Gtk::Notebook(cobject),
	builder_(builder),
	treeview_study_(0),
	treeview_scanner_(0)
{
	init_ui();
	connect_signals();
}

InformationNotebook::~InformationNotebook()
{
}

void
InformationNotebook::init_ui()
{
	treestore_study_ = Gtk::TreeStore::create(columns_study_);
	treestore_scanner_ = Gtk::TreeStore::create(columns_scanner_);

	builder_->get_widget( "treeview-scanner", treeview_scanner_);
	builder_->get_widget( "treeview-study", treeview_study_);

	// create model
	create_model();

	// create tree view
	treeview_study_->set_model(treestore_study_);
	treeview_study_->set_headers_visible(false);

	treeview_scanner_->set_model(treestore_scanner_);
	treeview_scanner_->set_headers_visible(false);

	add_treeview_columns( treeview_scanner_, columns_scanner_);
	add_treeview_columns( treeview_study_, columns_study_);
}

void
InformationNotebook::connect_signals()
{
}

void
InformationNotebook::add_items()
{
	std::vector<InfoItem> patient;
	patient.push_back(InfoItem( _("Name"), ""));
	patient.push_back(InfoItem( Q_("Patient|ID"), ""));
	patient.push_back(InfoItem( _("Sex"), ""));
	patient.push_back(InfoItem( _("Birthday"), ""));
	patient.push_back(InfoItem( _("Age"), ""));

	std::vector<InfoItem> study;
	study.push_back(InfoItem( _("Physician"), ""));
	study.push_back(InfoItem( _("Operator"), ""));
	study.push_back(InfoItem( _("Description"), ""));
	study.push_back(InfoItem( Q_("Study|ID"), ""));
	study.push_back(InfoItem( _("Body part"), ""));
	study.push_back(InfoItem( _("Institution"), ""));
	study.push_back(InfoItem( _("Date"), ""));
	study.push_back(InfoItem( _("Time"), ""));
	study.push_back(InfoItem( _("Model name"), ""));
	study.push_back(InfoItem( _("Effective dose"), ""));

	std::vector<InfoItem> xray_study;
	xray_study.push_back(InfoItem( _("High voltage"), ""));
	xray_study.push_back(InfoItem( _("Current"), ""));
	xray_study.push_back(InfoItem( _("Exposure"), ""));
//	xray_study.push_back(InfoItem( _("Exposure time"), ""));

	std::vector<InfoItem> array_study;
	array_study.push_back(InfoItem( _("Temperature"), ""));

	Glib::RefPtr<Gdk::Pixbuf> icon = render_icon(
		Gtk::StockID(icon_empty),
		Gtk::ICON_SIZE_MENU);

	std::vector<InfoItem> xray;
	xray.push_back(InfoItem( _("High voltage"), ""));
	xray.push_back(InfoItem( _("Current"), ""));
	xray.push_back(InfoItem( _("Exposure"), ""));
//	xray.push_back(InfoItem( _("Exposure time"), ""));
	xray.push_back(InfoItem( icon, Q_("X-ray|State"), ""));

	std::vector<InfoItem> temperature;
	temperature.push_back(InfoItem( Q_("Temperature|Control"), ""));
	temperature.push_back(InfoItem( Q_("Temperature|Current"), ""));
	temperature.push_back(InfoItem( Q_("Temperature|Margins"), ""));
	temperature.push_back(InfoItem( _("TEC value"), ""));

	std::vector<InfoItem> array;
	array.push_back(InfoItem( Q_("Array|ID"), ""));
	array.push_back(InfoItem( Q_("Array|Capasity"), ""));
	array.push_back(InfoItem( Q_("Array|Chip"), ""));

	icon = render_icon( Gtk::StockID(), Gtk::ICON_SIZE_MENU);
	items_study_.push_back(InfoItem( icon, _("Patient"), patient));
	items_study_.push_back(InfoItem( icon, _("Study"), study));
	items_study_.push_back(InfoItem( icon, _("X-ray Source"), xray_study));
	items_study_.push_back(InfoItem( icon, _("Detector Array"), array_study));

	icon = render_icon(
		Gtk::StockID(icon_radiation_inactive),
		Gtk::ICON_SIZE_MENU);
	items_scanner_.push_back(InfoItem( icon, _("X-ray Source"), xray));

	icon = render_icon( Gtk::StockID(),Gtk::ICON_SIZE_MENU);
	items_scanner_.push_back(InfoItem( icon, _("Temperature"), temperature));
	items_scanner_.push_back(InfoItem( icon, _("Detector Array"), array));
}

void
InformationNotebook::create_model()
{
	add_items();

	for ( std::vector<InfoItem>::const_iterator iter = items_scanner_.begin();
		iter != items_scanner_.end(); ++iter)
		add_treemodel_item( treestore_scanner_, columns_scanner_, *iter);

	for ( std::vector<InfoItem>::const_iterator iter = items_study_.begin();
		iter != items_study_.end(); ++iter)
		add_treemodel_item( treestore_study_, columns_study_, *iter);
}

void
InformationNotebook::add_treemodel_item(
	Glib::RefPtr<Gtk::TreeStore>& treemodel,
	const ModelColumns& columns,
	const InfoItem& foo)
{
	Gtk::TreeRow row = *(treemodel->append());

	row[columns.icon] = foo.icon;
	row[columns.label] = foo.label;
	row[columns.visible] = false;

	// Add Children:
	for ( std::vector<InfoItem>::const_iterator it = foo.children.begin();
		it != foo.children.end(); ++it) {

		const InfoItem& child = *it;

		Gtk::TreeRow child_row = *(treemodel->append(row.children()));

		child_row[columns.icon] = child.icon;
		child_row[columns.label] = child.label;
		child_row[columns.value] = child.value;
		child_row[columns.color] = child.color;
		child_row[columns.visible] = true;
	}
}

void
InformationNotebook::add_treeview_columns( Gtk::TreeView* treeview,
	const ModelColumns& columns)
{
	/* column for names */
	{
		int cc; // column count
		{
			Gtk::TreeView::Column* column;
			column = Gtk::manage(new Gtk::TreeView::Column(_("Label")));
			column->pack_start( columns.icon, false);
			column->pack_start( columns.label);
			cc = treeview->append_column(*column);
		}
		Gtk::TreeViewColumn* column = treeview->get_column(cc - 1);

		if (column) {
			Gtk::CellRenderer* renderer = column->get_first_cell_renderer();
			renderer->property_xalign().set_value(0.0);
		}
	}

	/* column for values */
	{
		int cc = treeview->append_column( _("Value"), columns.value);
		Gtk::TreeViewColumn* column = treeview->get_column(cc - 1);

		if (column) {
			Gtk::CellRendererText* renderer;

			renderer = dynamic_cast<Gtk::CellRendererText*>(
				column->get_first_cell_renderer());
			renderer->property_xalign().set_value(0.0);

			column->add_attribute( renderer->property_visible(),
				columns.visible);
			column->add_attribute( renderer->property_foreground(),
				columns.color);
			column->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
			column->set_fixed_width(50);
		}
	}
}

void
InformationNotebook::update_scanner_state(const Scanner::State& state)
{
	switch (state.manager_state().run()) {
	case RUN_INITIATION:
		break;
	case RUN_IMAGE_ACQUISITION:
		switch (state.manager_state().process()) {
		case PROCESS_EXPOSURE:
			{
				Glib::RefPtr<Gdk::Pixbuf> icon = render_icon(
					Gtk::StockID(icon_radiation_active), Gtk::ICON_SIZE_MENU);
				set_scanner_path_value( PATH_XRAY, icon, _("X-ray source"), "");
			}
			break;
		case PROCESS_ACQUISITION:
		case PROCESS_PARKING:
		case PROCESS_FINISH:
			{
				Glib::RefPtr<Gdk::Pixbuf> icon = render_icon(
					Gtk::StockID(icon_radiation_inactive),
					Gtk::ICON_SIZE_MENU);
				set_scanner_path_value( PATH_XRAY, icon, _("X-ray source"), "");

				icon = render_icon( Gtk::StockID(Gtk::Stock::NO),
					Gtk::ICON_SIZE_MENU);
				set_scanner_path_value( PATH_XRAY_STATE, icon, _("Disconnected"),
					"");
			}
			break;
		case PROCESS_ABORTED:
		case PROCESS_START:
		default:
			break;
		}
		break;
	case RUN_LINING_ACQUISITION:
	case RUN_COMMANDS:
		break;
	case RUN_BACKGROUND:
	{
		Glib::ustring text = (state.temperature_control()) ?
			Glib::ustring(_("On")) : Glib::ustring(_("Off"));
		set_scanner_path_value( PATH_TEMPERATURE_CONTROL, text);

		double t = state.temperature();		
		text = format_temperature_value(t);
		if (Scanner::TemperatureRegulator::temperature_within_range(t))
			set_scanner_path_value( PATH_TEMPERATURE_CURRENT, text);
		else
			set_scanner_path_value( PATH_TEMPERATURE_CURRENT, text, "red");

		double average, spread;
		state.temperature_margins( average, spread);
		text = format_temperature_margins( average, spread);
		set_scanner_path_value( PATH_TEMPERATURE_MARGINS, text);

		text = Glib::ustring::format(state.peltier_code());
		set_scanner_path_value( PATH_TEMPERATURE_PELTIER, text);

		set_scanner_path_value( PATH_DET_ID, state.id());

		text = (state.chip()) ? Glib::ustring( 1, state.chip())
			: Glib::ustring(Q_("Chip|None"));
		set_scanner_path_value( PATH_DET_CHIP, text);

		text = format_precision_value<double>( state.capacity(), _("pF"), 1);
		set_scanner_path_value( PATH_DET_CAPACITY, text);

	}
		break;
	case RUN_IMAGE_RECONSTRUCTION:
		break;
	case RUN_NONE:
	default:
		{
			Glib::RefPtr<Gdk::Pixbuf> icon = render_icon(
				Gtk::StockID(icon_empty), Gtk::ICON_SIZE_MENU);
			set_scanner_path_value( PATH_XRAY_STATE, icon, Q_("X-ray|State"),
				"");
		}
		set_scanner_path_value( PATH_XRAY_VOLTAGE, "");
		set_scanner_path_value( PATH_XRAY_CURRENT, "");
		set_scanner_path_value( PATH_XRAY_EXPOSURE, "");
		set_scanner_path_value( PATH_TEMPERATURE_CONTROL, "");
		set_scanner_path_value( PATH_TEMPERATURE_CURRENT, "");
		set_scanner_path_value( PATH_TEMPERATURE_PELTIER, "");
		set_scanner_path_value( PATH_TEMPERATURE_MARGINS, "");
		set_scanner_path_value( PATH_DET_ID, "");
		set_scanner_path_value( PATH_DET_CAPACITY, "");
		set_scanner_path_value( PATH_DET_CHIP, "");
		break;
	}
}

void
InformationNotebook::on_realize()
{
	treeview_study_->expand_all();
	treeview_scanner_->expand_all();

	// call base class:
	Gtk::Notebook::on_realize();
}

void
InformationNotebook::set_study_path_value( const char* path,
	const Glib::ustring& value,
	const Glib::ustring& color)
{
	Gtk::TreeModel::iterator iter = treestore_study_->get_iter(path);
	if (iter) {
		// Store the user's new text value in the model:
		Gtk::TreeRow row = *iter;
		row[columns_study_.value] = value;
		row[columns_study_.color] = color;
	}
}

void
InformationNotebook::set_scanner_path_value( const char* path,
	const Glib::ustring& value,
	const Glib::ustring& color)
{
	Gtk::TreeModel::iterator iter = treestore_scanner_->get_iter(path);

	if (iter) {
		// Store the user's new text value in the model:
		Gtk::TreeRow row = *iter;
		row[columns_scanner_.value] = value;
		row[columns_scanner_.color] = color;
	}
}

void
InformationNotebook::set_scanner_path_value( const char* path,
	const Glib::RefPtr<Gdk::Pixbuf>& icon,
	const Glib::ustring& label,
	const Glib::ustring& value,
	const Glib::ustring& color)
{
	Gtk::TreeModel::iterator iter = treestore_scanner_->get_iter(path);
	if (iter) {
		// Store the user's new text value in the model:
		Gtk::TreeRow row = *iter;
		row[columns_scanner_.icon] = icon;
		row[columns_scanner_.label] = label;
		row[columns_scanner_.value] = value;
		row[columns_scanner_.color] = color;
	}
}

void
InformationNotebook::clear_dicom_info()
{
	set_study_path_value( PATH_PAT_NAME, "");
	set_study_path_value( PATH_PAT_ID, "");
	set_study_path_value( PATH_PAT_SEX, "");
	set_study_path_value( PATH_PAT_DOB, "");
	set_study_path_value( PATH_PAT_AGE, "");
	set_study_path_value( PATH_STUD_PHYS, "");
	set_study_path_value( PATH_STUD_OPER, "");
	set_study_path_value( PATH_STUD_DESCR, "");
	set_study_path_value( PATH_STUD_ID, "");
	set_study_path_value( PATH_STUD_BODY, "");
	set_study_path_value( PATH_STUD_DOSE, "");
	set_study_path_value( PATH_STUD_INST, "");
	set_study_path_value( PATH_STUD_DATE, "");
	set_study_path_value( PATH_STUD_TIME, "");
	set_study_path_value( PATH_STUD_MANU, "");
}

void
InformationNotebook::on_xray_connection(bool state)
{
	Gtk::BuiltinStockID id = state ? Gtk::Stock::YES : Gtk::Stock::NO;

	Glib::RefPtr<Gdk::Pixbuf> icon = render_icon( Gtk::StockID(id),
		Gtk::ICON_SIZE_MENU);

	state ? set_scanner_path_value( PATH_XRAY_STATE, icon, _("Connected"), "") :
		set_scanner_path_value( PATH_XRAY_STATE, icon, _("Disconnected"), "");
}

void
InformationNotebook::on_xray_parameters(const XrayParameters& params)
{
	set_scanner_path_value( PATH_XRAY_EXPOSURE,
		format_value<double>( params.exposure, _("mAs")));

	set_scanner_path_value( PATH_XRAY_VOLTAGE,
		format_value<int>( params.voltage, _("kV")));

	set_scanner_path_value( PATH_XRAY_CURRENT,
		format_value<double>( params.current(), _("mA")));
/*
	set_scanner_path_value( PATH_XRAY_TIME,
		format_precision_value<double>( params.exposure_time(), _("s"), 2));
*/
}

void
InformationNotebook::on_dicom_info(const DICOM::SummaryInfo& summary)
{
	DICOM::StudyInfo* study = 0;
	DICOM::PatientInfo* patient = 0;
	DICOM::SummaryInfo* info = const_cast<DICOM::SummaryInfo*>(&summary);
	if (info && (patient = info->get_patient_info())) {
		Glib::ustring str = DICOM::format_person_name(patient->get_name());
		set_study_path_value( PATH_PAT_NAME, str);

		set_study_path_value( PATH_PAT_ID, patient->get_id());
		set_study_path_value( PATH_PAT_SEX, info->get_patient_sex());

		Glib::Date date = patient->get_birth_date();
		set_study_path_value( PATH_PAT_DOB, date.format_string("%x"));

		DICOM::PatientAge age = info->get_patient_age();
		set_study_path_value( PATH_PAT_AGE, age.age_string());
	}
	if (info && (study = info->get_study_info())) {
		Glib::ustring str = DICOM::format_person_name(study->get_physician_name());
		set_study_path_value( PATH_STUD_PHYS, str);

		str = DICOM::format_person_name(study->get_operator_name());
		set_study_path_value( PATH_STUD_OPER, str);

		set_study_path_value( PATH_STUD_DESCR, study->get_description());
		set_study_path_value( PATH_STUD_ID, study->get_id());
		set_study_path_value( PATH_STUD_BODY, study->get_LO(PrivateBodyPartTag));

		double dose = study->get_DS(PrivateEffectiveDoseTag);
		if (dose > 0) {
			Glib::ustring dose_string = format_dose_string(dose);
			set_study_path_value( PATH_STUD_DOSE, dose_string);
		}
		else {
			set_study_path_value( PATH_STUD_DOSE, Q_("Effective Dose|Unknown"));
		}

		set_study_path_value( PATH_STUD_INST, study->get_institution_name());

		Glib::Date date = study->get_date();
		set_study_path_value( PATH_STUD_DATE, date.format_string("%x"));

		OFTime time = study->get_time();

		str = DICOM::format_oftime(time);
		set_study_path_value( PATH_STUD_TIME, str);
		set_study_path_value( PATH_STUD_MANU,
			study->get_LO(DCM_ManufacturerModelName));
	}
}

void
InformationNotebook::on_xray_info(const DICOM::SummaryInfo& summary)
{
	DICOM::StudyInfo* study = 0;
	DICOM::SummaryInfo* info = const_cast<DICOM::SummaryInfo*>(&summary);
	if (info && (study = info->get_study_info())) {
		int exposure = study->get_IS(DCM_ExposureInuAs);
		set_study_path_value( PATH_STUD_XRAY_EXPOSURE,
			format_value<double>( double(exposure / 1000.0), _("mAs")));

		double voltage = study->get_DS(DCM_KVP); 
		set_study_path_value( PATH_STUD_XRAY_VOLTAGE,
			format_value<double>( voltage, _("kV")));

		int current = study->get_IS(DCM_XRayTubeCurrent); 
		set_study_path_value( PATH_STUD_XRAY_CURRENT,
			format_value<int>( current, _("mA")));
	}
}

void
InformationNotebook::on_array_info(const DICOM::SummaryInfo& summary)
{
	DICOM::StudyInfo* study = 0;
	DICOM::SummaryInfo* info = const_cast<DICOM::SummaryInfo*>(&summary);
	if (info && (study = info->get_study_info())) {
		double temperature = study->get_DS(DCM_DetectorTemperature);
		Glib::ustring text = format_temperature_value(temperature); 
		set_study_path_value( PATH_STUD_DET_TEMPERATURE, text);
	}
}

} // namespace UI

} // namespace ScanAmati
