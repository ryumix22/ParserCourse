
#include <string>
#include <bitset>
#include <vector>
#include <utility>
#include <map>
#include <functional>

class PacketUSBC
{
#pragma pack(push, 1)
	typedef struct
	{
		uint32_t kode1 : 8;
		uint32_t kode2 : 8;
		uint32_t kode3 : 8;
		uint32_t kode4 : 8;
	} SOP;

	typedef struct
	{
		uint16_t message_type : 5;
		uint16_t port_data_role : 1;
		uint16_t spec_revision : 2;
		uint16_t port__cable : 1;
		uint16_t message_id : 3;
		uint16_t number_of_data_object : 3;
		uint16_t extended : 1;
	} Header;

	typedef struct
	{
		uint32_t type : 2;
		uint32_t : 30;
	} PDO;

	typedef struct
	{
		uint32_t max_cur_in_10mA : 10;
		uint32_t voltage_in_50mV : 10;
		uint32_t peak_current : 2;
		uint32_t : 2;
		uint32_t unchunked: 1;
		uint32_t dualrole_data: 1;
		uint32_t usb_com_capable: 1;
		uint32_t unconstraited_power: 1;
		uint32_t usb_suspend_sup: 1;
		uint32_t dualrole_power: 1;
		uint32_t fixed_sup: 2;
	} FixedPDOSource;

	typedef struct
	{
		uint32_t max_cur_in_50mA_inc : 7;
		uint32_t : 1;
		uint32_t min_voltage_in_100mV_inc : 8;
		uint32_t : 1;
		uint32_t max_voltage_in_100mV_inc : 8;
		uint32_t : 2;
		uint32_t pps_power_limited : 1;
		uint32_t pps : 2;
		uint32_t apdo : 2;
	} APDOSource;

	typedef struct
	{
		uint32_t operational_cur_in_10mA : 10;
		uint32_t voltage_in_50mV : 10;
		uint32_t a: 2;
		uint32_t fast_role_swap : 2;
		uint32_t dualrole_data : 1;
		uint32_t usb_com_capable : 1;
		uint32_t unconstraited_power : 1;
		uint32_t higher_cap : 1;
		uint32_t dualrole_power : 1;
		uint32_t fixed_sup : 2;
	} FixedPDOSink;

	typedef struct
	{
		uint32_t max_cur_in_50mA_inc : 7;
		uint32_t : 1;
		uint32_t min_voltage_in_100mV_inc : 8;
		uint32_t : 1;
		uint32_t max_voltage_in_100mV_inc : 8;
		uint32_t : 3;
		uint32_t pps : 2;
		uint32_t apdo : 2;
	} APDOSink;

	typedef struct
	{
		uint32_t cur_in_10mA : 10;
		uint32_t min_voltage_in_50mV : 10;
		uint32_t max_voltage_in_50mV : 10;
		uint32_t variable_sup: 2;
	} VariableSupPDO;

	typedef struct
	{
		uint32_t power_in_250mW : 10;
		uint32_t min_voltage_in_50mV : 10;
		uint32_t max_voltage_in_50mV : 10;
		uint32_t battery : 2;
	} BatteryPDO;

	typedef struct
	{
		uint32_t operating : 10;
		uint32_t min_max : 10;
		uint32_t : 3;
		uint32_t unchunked : 1;
		uint32_t no_usb_suspend : 1;
		uint32_t usb_com_capable : 1;
		uint32_t cap_mismatch : 1;
		uint32_t giveback : 1;
		uint32_t position: 3;
		uint32_t : 1;
 	} RDO;

	typedef struct
	{
		uint32_t operating : 7;
		uint32_t : 2;
		uint32_t out_voltage_in_20mV : 11;
		uint32_t : 3;
		uint32_t unchunked : 1;
		uint32_t no_usb_suspend : 1;
		uint32_t usb_com_capable : 1;
		uint32_t cap_mismatch : 1;
		uint32_t : 1;
		uint32_t position: 3;
		uint32_t : 1;
 	} PRDO;

	typedef struct
	{
		uint32_t mode : 4;
		uint32_t : 28;
	} BIST;

	typedef struct 
	{
		uint32_t : 8;
		uint32_t inv_battery_ref : 1;
		uint32_t present : 1;
		uint32_t status : 2;
		uint32_t : 4;
		uint32_t capacity : 16;
	} BatteryStatus;

	typedef struct
	{
		uint32_t : 16;
		uint32_t hot_swap_batteries : 4;
		uint32_t fixed_batteries : 4;
		uint32_t : 1;
		uint32_t status_change : 1;
		uint32_t ocp : 1;
		uint32_t otp : 1;
		uint32_t occ : 1;
		uint32_t sice : 1;
		uint32_t ovp : 1;
		uint32_t : 1;
	} Alert;

	typedef struct
	{
		uint32_t : 16;
		uint32_t second_char : 8;
		uint32_t first_char : 8;
	} CountryInfo;

	//Vendor Defined Messages

	typedef struct
	{
		uint32_t vendor_usage : 15;
		uint32_t type : 1;
		uint32_t svid : 16;
	} UnstructuredVDMHeader;

	typedef struct 
	{
		uint32_t command : 5;
		uint32_t : 1;
		uint32_t command_type : 2;
		uint32_t position : 3;
		uint32_t : 2;
		uint32_t version : 2;
		uint32_t type : 1;
		uint32_t svid : 16;
	} StructuredVDMHeader;

	/*typedef struct
	{
		uint32_t usb_id : 16;
		uint32_t : 7;
	};*/

#pragma pack(pop)

	size_t length;
	std::vector<bool> bits;
	std::string text;
	//std::string briefText;
	bool sopFlag = false;
	void rec_sym(unsigned char sym);
	unsigned char get_sym(int i, bool rec = false);
	int scan_sop();
	int get_short(int idx);
	int get_int(int idx);
	int get_word(int idx);
	void get_source_cap(int dataObj);
	void get_request(int dataObj);
	void get_sink(int dataObj);
	void get_bist(int dataObj);
	void get_battery(int dataObj);
	void get_alert(int dataObj);
	void get_country(int dataObj);
	void get_vdo_header(int dataObj);
	void printDataObj(void(PacketUSBC::*foo)(int dataObj), int numOfData, int dataStart);

public:
	void parse(const unsigned char *data, size_t size);
	std::string getText() { return text; }
	//std::string getBriefText() { return briefText; }
	void getStringVector();
};

