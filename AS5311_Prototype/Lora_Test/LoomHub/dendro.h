#include <ArduinoJson.h>

struct Dendro_Base_t {
	char name[16];
	int instance;
	char datestamp[16];
	char timestamp[16];
	int pktnumber;
	float vbatt;
	float sht31dTemp;
	float sht31dHumidity;
    int magneticSerial;
    float displacementMM;
    float displacementUM;
    float differenceMM;
    float differenceUM;
    char status[16];
};

typedef union {
	Dendro_Base_t data;
	uint8_t raw[sizeof(Dendro_Base_t)];
} Dendro_t;

void json_to_struct(const JsonObjectConst& data, Dendro_t& out);

void struct_to_json(const Dendro_t& in, const JsonObject& out);