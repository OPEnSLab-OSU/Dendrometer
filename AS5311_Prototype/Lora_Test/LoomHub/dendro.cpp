#include "dendro.h"

static bool get_data_point_from_contents_float(const JsonArrayConst& contents, const char* module_name, const char* data_key, float& out) {
	for (const JsonVariantConst& module_data : contents) {
		// if the module name matches, check the data key
		const char* name = module_data["module"];
		if (name == nullptr)
			continue;
		if (strncmp(name, module_name, 20) != 0)
			continue;
		const JsonObjectConst data_obj = module_data["data"];
		if (data_obj.isNull())
			continue;
		// find the data we need
		const JsonVariantConst data_value = data_obj[data_key];
		if (!data_value.isNull() && data_value.is<float>()) {
			// write to out
			out = data_value.as<float>();
			// done!
			return true;
		}
	}
	// return an empty (null) variant
	out = NAN;
	return false;
}

static bool get_data_point_from_contents_int(const JsonArrayConst& contents, const char* module_name, const char* data_key, int& out) {
	for (const JsonVariantConst& module_data : contents) {
		// if the module name matches, check the data key
		const char* name = module_data["module"];
		if (name == nullptr)
			continue;
		if (strncmp(name, module_name, 20) != 0)
			continue;
		const JsonObjectConst data_obj = module_data["data"];
		if (data_obj.isNull())
			continue;
		// find the data we need
		const JsonVariantConst data_value = data_obj[data_key];
		if (!data_value.isNull() && data_value.is<int>()) {
			// write to out
			out = data_value.as<int>();
			// done!
			return true;
		}
	}
	// return an empty (null) variant
	out = -1;
	return false;
}

static bool get_data_point_from_contents_c_string(const JsonArrayConst& contents, const char* module_name, const char* data_key, char* out) {
	for (const JsonVariantConst& module_data : contents) {
		// if the module name matches, check the data key
		const char* name = module_data["module"];
		if (name == nullptr)
			continue;
		if (strncmp(name, module_name, 20) != 0)
			continue;
		const JsonObjectConst data_obj = module_data["data"];
		if (data_obj.isNull())
			continue;
		// find the data we need
		const JsonVariantConst data_value = data_obj[data_key];
		if (!data_value.isNull() && data_value.is<char*>()) {
			// write to out
			strncpy(out, data_value.as<char*>(), 15);
			// done!
			return true;
		}
	}
	// return an empty (null) variant
	out = "ERROR_ON_LORA_COPY";
	return false;
}

void json_to_struct(const JsonObjectConst& data, Dendro_t& out) {
	// name, instance
	const JsonObjectConst id = data["id"];
	if (!id.isNull()) {
		const char* name = id["name"];
		if (name != nullptr)
			strncpy(out.data.name, name, 15);
		else
			out.data.name[0] = 0;
		out.data.instance = id["instance"] | -1;
	}
	// timestamp
	const JsonObjectConst stamp = data["timestamp"];
	if (!stamp.isNull()) {
		const char* date = stamp["date"].as<const char*>();
		if (date != nullptr)
			strncpy(out.data.datestamp, date, 15);
		else
			out.data.datestamp[0] = 0;
		const char* time = stamp["time"].as<const char*>();
		if (time != nullptr)
			strncpy(out.data.timestamp, time, 15);
		else
			out.data.timestamp[0] = 0;
	}
	// get a reference to the "contents" object
	const JsonArrayConst contents = data["contents"];
	if (contents.isNull())
		return;
	// find every data point we care about, and populate the struct with it
	// packet #
	get_data_point_from_contents_int(contents, "Packet", "Number", out.data.pktnumber);
	// vbatt
	get_data_point_from_contents_float(contents, "Analog", "Vbat", out.data.vbatt);
    // SHT31D temperature
	get_data_point_from_contents_float(contents, "SHT31D", "temp", out.data.sht31dTemp);
	// SHT31D humidity
	get_data_point_from_contents_float(contents, "SHT31D", "humid", out.data.sht31dHumidity);
    // AS5311 serial position
	get_data_point_from_contents_int(contents, "AS5311", "Serial Value", out.data.magneticSerial);
    // AS5311 displacements and differences
	get_data_point_from_contents_float(contents, "Displacement (mm)", "mm", out.data.displacementMM);
	get_data_point_from_contents_float(contents, "Displacement (um)", "um", out.data.displacementUM);
	get_data_point_from_contents_float(contents, "Difference (mm)", "mm", out.data.differenceMM);
	get_data_point_from_contents_float(contents, "Difference (um)", "um", out.data.differenceUM);
    // AS5311 status
    get_data_point_from_contents_c_string(contents, "Status", "Color", out.data.status);
}

static JsonObject make_module_object(const JsonArray& contents, const char* name) {
	const JsonObject data = contents.createNestedObject();
	data["module"] = name;
	return data.createNestedObject("data");
}

void struct_to_json(const Dendro_t& in, const JsonObject& out) {
	// start adding object to it!
	out["type"] = "data";
	// id block
	const JsonObject id = out.createNestedObject("id");
	id["name"] = in.data.name;
	id["instance"] = in.data.instance;
	// timestamp block
	const JsonObject timestamp = out.createNestedObject("timestamp");
	timestamp["date"] = in.data.datestamp;
	timestamp["time"] = in.data.timestamp;
	// contents array
	const JsonArray contents = out.createNestedArray("contents");
	// add objects for each sensor!
	// vbatt
	{
		const JsonObject data = make_module_object(contents, "Analog");
		data["Vbat"] = in.data.vbatt;
	}
	// pkt number
	{
		const JsonObject data = make_module_object(contents, "Packet");
		data["Number"] = in.data.pktnumber;
	}
	// sht31d
	{
		const JsonObject data = make_module_object(contents, "SHT31D");
		data["humid"] = in.data.sht31dHumidity;
		data["temp"] = in.data.sht31dTemp;
	}
	// AS5311 values
	{
		const JsonObject data = make_module_object(contents, "AS5311");
		data["Serial_Value"] = in.data.magneticSerial;
	}
    {
		const JsonObject data = make_module_object(contents, "Displacement_(mm)");
		data["mm"] = in.data.displacementMM;
	}
    {
		const JsonObject data = make_module_object(contents, "Displacement_(um)");
		data["um"] = in.data.displacementUM;
	}
    {
		const JsonObject data = make_module_object(contents, "Difference_(mm)");
		data["mm"] = in.data.differenceMM;
	}
    {
		const JsonObject data = make_module_object(contents, "Difference_(um)");
		data["um"] = in.data.differenceUM;
	}
    {
		const JsonObject data = make_module_object(contents, "Status");
		data["Color"] = in.data.status;
	}
}