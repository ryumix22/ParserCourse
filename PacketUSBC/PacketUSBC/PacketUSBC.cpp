#include "PacketUSBC.h"
#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include <sstream>
#include <string>
#include <iomanip>
#include <stdio.h>
#include <functional>

// Control message type
const std::string CTRL_TYPES[] = {
	"reserved",
	"GOOD CRC",
	"GOTO MIN",
	"ACCEPT",
	"REJECT",
	"PING",
	"PS_RDY",
	"GET_SOURCE_CAP",
	"GET_SINK_CAP",
	"DR_SWAP",
	"PR_SWAP",
	"VCONN_SWAP",
	"WAIT",
	"SOFT_RESET",
	"reserved",
	"reserved",
	"NOT_SUPPORTED",
	"GET_SOURCE_CAP_EXTENDED",
	"GET_STATUS",
	"FR_SWAP",
	"GET_PPS_STATUS",
	"GET_COUNTRY_CODES",
	"GET_SINK_CAP_EXTENDED",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved"
};

// Data message type
const std::string DATA_TYPES[] = {
	"reserved",
	"SOURCE_CAPABILITIES",
	"REQUEST",
	"BIST",
	"SINK_CAPABILITIES",
	"BATTERY_STATUS",
	"ALERT",
	"GET_COUNTRY_INFO",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"VENDOR_DEFINED",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved"
};

const unsigned char DEC4B5B[] = {
	0x10,   //# Error      00000
	0x10,   //# Error      00001
	0x10,   //# Error      00010
	0x10,   //# Error      00011
	0x10,   //# Error      00100
	0x10,   //# Error      00101
	0x13,   //# Sync-3     00110
	0x14,   //# RST-1      00111
	0x10,   //# Error      01000
	0x01,   //# 1 = 0001   01001
	0x04,   //# 4 = 0100   01010
	0x05,   //# 5 = 0101   01011
	0x10,   //# Error      01100
	0x16,   //# EOP        01101
	0x06,   //# 6 = 0110   01110
	0x07,   //# 7 = 0111   01111
	0x10,   //# Error      10000
	0x12,   //# Sync-2     10001
	0x08,   //# 8 = 1000   10010
	0x09,   //# 9 = 1001   10011
	0x02,   //# 2 = 0010   10100
	0x03,   //# 3 = 0011   10101
	0x0A,   //# A = 1010   10110
	0x0B,   //# B = 1011   10111
	0x11,   //# Sync-1     11000
	0x15,   //# RST-2      11001
	0x0C,   //# C = 1100   11010
	0x0D,   //# D = 1101   11011
	0x0E,   //# E = 1110   11100
	0x0F,   //# F = 1111   11101
	0x00,   //# 0 = 0000   11110
	0x10    //# Error      11111
};

const std::map<int, std::string> START_OF_PACKETS2 = {
	{0x12111111, "SOP"},
	{0x13131111, "SOP'"},
	{0x13111311, "SOP\""},
	{0x13151511, "SOP' Debug"},
	{0x12131511, "SOP\" Debug"},
	{0x13141114, "Cable Reset"},
	{0x15141414, "Hard Reset"}
};

const std::pair<std::string,std::string> SYM_NAME[] = {
	{"0x0", "0"},
	{"0x1", "1"},
	{"0x2", "2"},
	{"0x3", "3"},
	{"0x4", "4"},
	{"0x5", "5"},
	{"0x6", "6"},
	{"0x7", "7"},
	{"0x8", "8"},
	{"0x9", "9"},
	{"0xA", "A"},
	{"0xB", "B"},
	{"0xC", "C"},
	{"0xD", "D"},
	{"0xE", "E"},
	{"0xF", "F"},
	{"ERROR", "X"},
	{"SYNC-1", "S1"},
	{"SYNC-2", "S2"},
	{"SYNC-3", "S3"},
	{"RST-1", "R1"},
	{"RST-2", "R2"},
	{"EOP", "#"}
};

std::string int_to_hex(int num, int size)
{
	std::ostringstream stream;
	stream << std::setw(size) << std::setfill('0') << std::hex << num << std::dec;
	return stream.str();
}

void PacketUSBC::rec_sym(unsigned char sym)
{
	// putx(i, i+5, [7, SYM_NAME[sym]])
	text += " ";
	text += SYM_NAME[sym].second;
}

unsigned char PacketUSBC::get_sym(int i, bool rec)
{

	int v = bits[i] + (bits[i + 1] << 1) + (bits[i + 2] << 2) +
		(bits[i + 3] << 3) + (bits[i + 4] << 4);
	unsigned char sym = DEC4B5B[v];
	if (rec)
	{
		rec_sym(sym);
	}

	return sym;
}


int PacketUSBC::scan_sop() 
{
	for (int i = 0; i < length - 19; ++i) {
		uint32_t startOfPacket = get_int(i);
		SOP start = *((SOP*)&startOfPacket);
		auto result = START_OF_PACKETS2.find(*((uint32_t*)&start));
		if (result != START_OF_PACKETS2.end()) {
			text += (*result).second;
			if ((*result).second == "SOP") sopFlag = true;
			text += " Preamble";
			// # annotate each symbol
			rec_sym(start.kode1);
			rec_sym(start.kode2);
			rec_sym(start.kode3);
			rec_sym(start.kode4);
			if ((*result).second == "Hard Reset")
			{
				text += " HRST\n";
				return -1; //  # Hard reset
			}
			else if ((*result).second == "Cable Reset")
			{
				text += " CRST\n";
				return -1; //   # Cable reset
			}
			else
			{
				text += " \n"; //" S"
				// self.putx(i, i+20, [2, [sym, 'S']])
			}
			return i + 20;
		}
	}

#if 0
	self.putx(0, len(self.bits), [1, ['Junk???', 'XXX']])
		self.text += 'Junk???'
		self.putwarn('No start of packet found', 'XXX')
#endif
		text += "No start of packet found";
	return -1; //   # No Start Of Packet
}

int PacketUSBC::get_short(int idx)
{
	// # Check it's not a truncated packet
	if (bits.size() - idx <= 20)
	{
		text += " Truncated!";
		return 0x0BAD;
	}
	//# TODO check bad symbols
	int val = get_sym(idx) | (get_sym(idx + 5) << 4) | (get_sym(idx + 10) << 8) | (get_sym(idx + 15) << 12);
	return val;
}

int PacketUSBC::get_int(int idx)
{
	// # Check it's not a truncated packet
	if (bits.size() - idx <= 20)
	{
		text += " Truncated!";
		return 0x0BAD;
	}
	//# TODO check bad symbols
	int val = get_sym(idx) | (get_sym(idx + 5) << 8) | (get_sym(idx + 10) << 16) | (get_sym(idx + 15) << 24);
	return val;
}

int PacketUSBC::get_word(int idx)
{
	int lo = get_short(idx);
	int hi = get_short(idx + 20);
	return lo + (hi << 16);
}

void PacketUSBC::get_source_cap(int _dataObj)
{
	PDO dataObj = (*(PDO*)&_dataObj);
	char txt[1024] = "";
	switch (dataObj.type)
	{
	case 0:
	{
		FixedPDOSource data = (*(FixedPDOSource*)&dataObj);
		sprintf_s(txt, " Fixed supply\n\
		Voltage %.1fV, Max Current %.1fA\n\
		Peak current: %d\n\
		Unchunked: %d\n\
		Dual-Role Data: %d\n\
		USB Communication Capable: %d\n\
		Unconstraited Power: %d\n\
		USB Suspend Supported: %d\n\
		Dual-Role Power: %d", data.voltage_in_50mV * 50 / 1000.0, data.max_cur_in_10mA * 10 / 1000.0, data.peak_current, data.unchunked, data.dualrole_data, 
			data.usb_com_capable, data.unconstraited_power, data.usb_suspend_sup, data.dualrole_power);
		text += txt;
	}
	break;
	case 1:
	{
		BatteryPDO data = (*(BatteryPDO*)&dataObj);
		sprintf_s(txt, " Battery supply\n\
		Max allowable power %.1fW\n\
		Min voltage %.1fV\n\
		Max voltage %.1fV", data.power_in_250mW * 250 / 1000.0, data.min_voltage_in_50mV * 50 / 1000.0, data.max_voltage_in_50mV * 50 / 1000.0);
		text += txt;
	}
	break;
	case 2:
	{
		VariableSupPDO data = (*(VariableSupPDO*)&dataObj);
		sprintf_s(txt, " Variable supply\n\
		Max current %.1fA\n\
		Min voltage %.1fV\n\
		Max voltage %.1fV", data.cur_in_10mA * 10 / 1000.0, data.min_voltage_in_50mV * 50 / 1000.0, data.max_voltage_in_50mV * 50 / 1000.0);
		text += txt;
	}
	break;
	case 3:
	{
		APDOSource data = (*(APDOSource*)&dataObj);
		sprintf_s(txt, " Programmable power supply\n\
		Max current increments %.1fA\n\
		Min voltage increments %.1fV\n\
		Max voltage increments %.1fV\n\
		PPS power limited: %d\n", data.max_cur_in_50mA_inc * 50 / 1000.0, data.min_voltage_in_100mV_inc * 100 / 1000.0, data.max_voltage_in_100mV_inc * 100 / 1000.0, data.pps_power_limited);
		text += txt;
	}
	break;
	default:
		txt[0] = 0;
	}
}

void PacketUSBC::get_request(int dataObj)
{
	RDO data = (*(RDO*)&dataObj);
	char txt[1024] = "";
	switch (data.position)
	{
	case 1:
	{
		if (data.giveback) {
			sprintf_s(txt, 
			"Fixed and Variable Request\n\
		Min operating current %.1fA, Operating Current %.1fA\n\
		Unchunked: %d\n\
		No USB Suspend: %d\n\
		USB Communication Capable: %d\n\
		Capability mismatch: %d\n\
		GiveBack flag: %d\n\
		Object position: %d", data.min_max * 10 / 1000.0, data.operating * 10 / 1000.0, data.unchunked, data.no_usb_suspend,
				data.usb_com_capable, data.cap_mismatch, data.giveback, data.position);
			text += txt;
		}
		else {
			sprintf_s(txt, " Fixed and Variable Request\n\
		Max operating current %.1fA, Operating Current %.1fA\n\
		Unchunked: %d\n\
		No USB Suspend: %d\n\
		USB Communication Capable: %d\n\
		Capability mismatch: %d\n\
		GiveBack flag: %d\n\
		Object position: %d", data.min_max * 10 / 1000.0, data.operating * 10 / 1000.0, data.unchunked, data.no_usb_suspend,
				data.usb_com_capable, data.cap_mismatch, data.giveback, data.position);
			text += txt;
		}
	
	}
	break;
	case 2: //battery
	{
		if (data.giveback) {
			sprintf_s(txt, " Battery Request\n\
		Min operating power %.1fW, Operating power %.1fW\n\
		Unchunked: %d\n\
		No USB Suspend: %d\n\
		USB Communication Capable: %d\n\
		Capability mismatch: %d\n\
		GiveBack flag: %d\n\
		Object position: %d", data.min_max * 250 / 1000.0, data.operating * 250 / 1000.0, data.unchunked, data.no_usb_suspend,
				data.usb_com_capable, data.cap_mismatch, data.giveback, data.position);
			text += txt;
		}
		else {
			sprintf_s(txt, " Battery Request\n\
		Max operating power %.1fW, Operating power %.1fW\n\
		Unchunked: %d\n\
		No USB Suspend: %d\n\
		USB Communication Capable: %d\n\
		Capability mismatch: %d\n\
		GiveBack flag: %d\n\
		Object position: %d", data.min_max * 250 / 1000.0, data.operating * 250 / 1000.0, data.unchunked, data.no_usb_suspend,
				data.usb_com_capable, data.cap_mismatch, data.giveback, data.position);
			text += txt;
		}
	}
	break;
	case 3: 
	{
		if (data.giveback) {
			sprintf_s(txt, " Fixed and Variable Request\n\
		Min operating current %.1fA, Operating Current %.1fA\n\
		Unchunked: %d\n\
		No USB Suspend: %d\n\
		USB Communication Capable: %d\n\
		Capability mismatch: %d\n\
		GiveBack flag: %d\n\
		Object position: %d", data.min_max * 10 / 1000.0, data.operating * 10 / 1000.0, data.unchunked, data.no_usb_suspend,
				data.usb_com_capable, data.cap_mismatch, data.giveback, data.position);
			text += txt;
		}
		else {
			sprintf_s(txt, " Fixed and Variable Request\n\
		Max operating current %.1fA, Operating Current %.1fA\n\
		Unchunked: %d\n\
		No USB Suspend: %d\n\
		USB Communication Capable: %d\n\
		Capability mismatch: %d\n\
		GiveBack flag: %d\n\
		Object position: %d", data.min_max * 10 / 1000.0, data.operating * 10 / 1000.0, data.unchunked, data.no_usb_suspend,
				data.usb_com_capable, data.cap_mismatch, data.giveback, data.position);
			text += txt;
		}
	}
	break;
	case 4:
	{
		PRDO pdata = (*(PRDO*)&data);
		sprintf_s(txt, " Programmable Request\n\
		Operating current %.1fA, Output voltage %.1fV\n\
		Unchunked: %d\n\
		No USB Suspend: %d\n\
		USB Communication Capable: %d\n\
		Capability mismatch: %d\n\
		Object position: %d", pdata.operating * 50 / 1000.0, pdata.out_voltage_in_20mV * 20 / 1000.0, pdata.unchunked, pdata.no_usb_suspend,
			pdata.usb_com_capable, pdata.cap_mismatch, pdata.position);
		text += txt;
	}
	break;
	default:
		txt[0] = 0;
	}
}

void PacketUSBC::get_sink(int _dataObj) 
{
	PDO dataObj = (*(PDO*)&_dataObj);
	char txt[1024] = "";
	switch (dataObj.type)
	{
	case 0:
	{
		FixedPDOSink data = (*(FixedPDOSink*)&dataObj);
		switch (data.fast_role_swap)
		{
		case 0:
		{
			sprintf_s(txt, " Fixed supply\n\
		Voltage %.1fV, Max Current %.1fA\n\
		Fast swap not supported\n\
		Dual-Role Data: %d\n\
		USB Communication Capable: %d\n\
		Unconstraited Power: %d\n\
		Higher Capability: %d\n\
		Dual-Role Power: %d", data.operational_cur_in_10mA * 10 / 1000.0, data.voltage_in_50mV * 50 / 1000.0, data.dualrole_power, 
				data.usb_com_capable, data.unconstraited_power, data.higher_cap, data.dualrole_power);
			text += txt;
		}
		break;
		case 1:
		{
			sprintf_s(txt, " Fixed supply\n\
		Voltage %.1fV, Max Current %.1fA\n\
		Fast role swap: Default USB power\n\
		Dual-Role Data: %d\n\
		USB Communication Capable: %d\n\
		Unconstraited Power: %d\n\
		Higher Capability: %d\n\
		Dual-Role Power: %d", data.operational_cur_in_10mA * 10 / 1000.0, data.voltage_in_50mV * 50 / 1000.0, data.dualrole_power,
				data.usb_com_capable, data.unconstraited_power, data.higher_cap, data.dualrole_power);
			text += txt;
		}
		break;
		case 2:
		{
			sprintf_s(txt, " Fixed supply\n\
		Voltage %.1fV, Max Current %.1fA\n\
		Fast role swap: 1.5A @ 5V\n\
		Dual-Role Data: %d\n\
		USB Communication Capable: %d\n\
		Unconstraited Power: %d\n\
		Higher Capability: %d\n\
		Dual-Role Power: %d", data.operational_cur_in_10mA * 10 / 1000.0, data.voltage_in_50mV * 50 / 1000.0, data.dualrole_power,
				data.usb_com_capable, data.unconstraited_power, data.higher_cap, data.dualrole_power);
			text += txt;
		}
		break;
		case 3:
		{
			sprintf_s(txt, " Fixed supply\n\
		Voltage %.1fV, Max Current %.1fA\n\
		Fast role swap: 3.0A @ 5V\n\
		Dual-Role Data: %d\n\
		USB Communication Capable: %d\n\
		Unconstraited Power: %d\n\
		Higher Capability: %d\n\
		Dual-Role Power: %d", data.operational_cur_in_10mA * 10 / 1000.0, data.voltage_in_50mV * 50 / 1000.0, data.dualrole_power,
				data.usb_com_capable, data.unconstraited_power, data.higher_cap, data.dualrole_power);
			text += txt;
		}
		break;
		default:
			txt[0] = 0;
		}
	}
	break;
	case 1:
	{
		BatteryPDO data = (*(BatteryPDO*)&dataObj);
		sprintf_s(txt, " Battery supply\n\
		Operational power %.1fW\n\
		Min voltage %.1fV\n\
		Max voltage %.1fV", data.power_in_250mW * 250 / 1000.0, data.min_voltage_in_50mV * 50 / 1000.0, data.max_voltage_in_50mV * 50 / 1000.0);
		text += txt;
	}
	break;
	case 2:
	{
		VariableSupPDO data = (*(VariableSupPDO*)&dataObj);
		sprintf_s(txt, " Variable supply\n\
		Operational current %.1fA\n\
		Min voltage %.1fV\n\
		Max voltage %.1fV", data.cur_in_10mA * 10 / 1000.0, data.min_voltage_in_50mV * 50 / 1000.0, data.max_voltage_in_50mV * 50 / 1000.0);
		text += txt;
	}
	break;
	case 3:
	{
		APDOSink data = (*(APDOSink*)&dataObj);
		sprintf_s(txt, " Programmable power supply\n\
		Max current increments %.1fA\n\
		Min voltage increments %.1fV\n\
		Max voltage increments %.1fV", data.max_cur_in_50mA_inc * 50 / 1000.0, data.min_voltage_in_100mV_inc * 100 / 1000.0, data.max_voltage_in_100mV_inc * 100 / 1000.0);
		text += txt;
	}
	break;
	default:
		txt[0] = 0;
	}
}

void PacketUSBC::get_battery(int dataObj)
{
	BatteryStatus data = (*(BatteryStatus*)&dataObj);
	char txt[1024] = "";
	if (data.present) 
	{
		switch (data.status)
		{
		case 0:
		{
			if (data.capacity == 0xffff)
			{
				sprintf_s(txt, " Battery's State of Charge unknown");
				text += txt;
			}
			else
			{
				sprintf_s(txt, " Battery's State of Charge: %.1fWH\n\
						Battery is Charging\n\
						Invalid battery reference: %d", data.capacity / 10.0, data.inv_battery_ref);
				text += txt;
			}
		}
		break;
		case 1: 
		{
			if (data.capacity == 0xffff)
			{
				sprintf_s(txt, " Battery's State of Charge unknown");
				text += txt;
			}
			else
			{
				sprintf_s(txt, " Battery's State of Charge: %.1fWH\n\
						Battery is Discharging\n\
						Invalid battery reference: %d", data.capacity / 10.0, data.inv_battery_ref);
				text += txt;
			}
		}
		break;
		case 2:
		{
			if (data.capacity == 0xffff)
			{
				sprintf_s(txt, " Battery's State of Charge unknown");
				text += txt;
			}
			else
			{
				sprintf_s(txt, " Battery's State of Charge: %.1fWH\n\
						Battery is Idle\n\
						Invalid battery reference: %d", data.capacity / 10.0, data.inv_battery_ref);
				text += txt;
			}
		}
		break;
		default:
			txt[0] = 0;
		}
	}
	else 
	{
		if (data.capacity == 0xffff) 
		{
			sprintf_s(txt, " Battery's State of Charge unknown");
			text += txt;
		}
		else
		{
			sprintf_s(txt, " Battery's State of Charge: %.1fWH\n\
						Battery is not present\n\
						Invalid battery reference: %d", data.capacity / 10.0, data.inv_battery_ref);
			text += txt;
		}
	}
}

void PacketUSBC::get_alert(int dataObj)
{
	Alert data = (*(Alert*)&dataObj);
	char txt[1024] = "";
	if (data.status_change) 
	{
		sprintf_s(txt, " Battery status change: %d\n\
					OCP: %d\n\
					OTP: %d\n\
					OCC: %d\n\
					SICE: %d\n\
					OVP: %d", data.status_change, data.ocp, data.otp, data.occ, data.sice, data.ovp);
		text += txt;
	}
	else 
	{
		sprintf_s(txt, " Battery status change: %d\n\
					B16 status change: %d\n\
					B17 status change: %d\n\
					B18 status change: %d\n\
					B19 status change: %d\n\
					B20 status change: %d\n\
					B21 status change: %d\n\
					B22 status change: %d\n\
					B23 status change: %d\n\
					OCP: %d\n\
					OTP: %d\n\
					OCC: %d\n\
					SICE: %d\n\
					OVP: %d", data.status_change, data.hot_swap_batteries & 1, (data.hot_swap_batteries >> 1) & 1, (data.hot_swap_batteries >> 2) & 1, (data.hot_swap_batteries >> 3) & 1, 
			data.fixed_batteries & 1, (data.fixed_batteries >> 1) & 1, (data.fixed_batteries >> 2) & 1, (data.fixed_batteries >> 3) & 1,
			data.ocp, data.otp, data.occ, data.sice, data.ovp);
		text += txt;
	}
}

void PacketUSBC::get_bist(int dataObj) {
	BIST data = (*(BIST*)&dataObj);
	char txt[1024] = "";
	if (data.mode == 5) {
		text += "BIST Carrier Mode";
	}
	if (data.mode == 8) {
		text += "BIST Test Data";
	}
}

void PacketUSBC::printDataObj(void(PacketUSBC::*foo)(int dataObj), int numOfData, int dataStart) 
{
	for (int i = 0; i < numOfData; i++) {
		int dataByte = get_word(dataStart + i * 40);
		text += "[" + std::to_string(i) + "]0x" + int_to_hex(dataByte, 8) + " -";
		(this->*foo)(dataByte);
		text += "\n";
	}
}

void PacketUSBC::get_country(int dataObj)
{
	CountryInfo data = (*(CountryInfo*)&dataObj);
	text += " " + (char)data.first_char + (char)data.second_char;
}

void PacketUSBC::get_vdo_header(int dataObj)
{
	StructuredVDMHeader data = (*(StructuredVDMHeader*)&dataObj);
	char txt[1024] = "";
	const char *version[4] = {"Version 1.0", "Version 2.0", "-", "-"};
	const char *command_type[4] = {"REQ", "ACK", "NAK", "BUSY"};
	const char *commands[8] = { "", "Discover Identity", "Discover SVIDs", "Discover Modes", "Enter Mode", "Exit Mode", "Attention", "SVID Specific Commands" };
	if (data.type) 
	{
		if (data.command < 7) {
			text += " SVID: 0x" + int_to_hex(data.svid, 4) + "\n";
			sprintf_s(txt, "		Structured VDM\n\
		VDM %s\n\
		Object position %d\n\
		Command type: %s\n\
		Command: %s", version[data.version], data.position, command_type[data.command_type], commands[data.command]);
			text += txt;
		}
		else if (data.command >= 7 && data.command < 16) {
			text += "wrong command";
		}
		else {
			text += " SVID: 0x" + int_to_hex(data.svid, 4) + "\n";
			sprintf_s(txt, "		Structured VDM\n\
		VDM %s\n\
		Object position %d\n\
		Command type: %s\n\
		Command: %s", version[data.version], data.position, command_type[data.command_type], commands[7]);
			text += txt;
		}
	}
	else {
		UnstructuredVDMHeader unstructedData = (*(UnstructuredVDMHeader*)&data);
		text += " SVID: 0x" + int_to_hex(unstructedData.svid, 4) + "\n";
		text += "		Vendor usage: 0x" + int_to_hex(unstructedData.vendor_usage, 4) + "\n";
		text += txt;
	}
}

void PacketUSBC::parse(const unsigned char *data, size_t _length)
{
	bits.push_back(0);
	length = 8 * _length;
	std::stringstream s;
	if (length < 10)
	{
		text = "ERR: Too short PD packet: 0x" + int_to_hex(data[0], 2);
		return;
	}

	for (int i = 0; i < _length; i++)
	{
		unsigned char d = data[i];
		for (int j = 0; j < 8; j++, d >>= 1)
		{
			bits.push_back(d & 1);
		}
	}

	text = "[PD] ";
	int idx = scan_sop();
	int sizeOfDataBite = 0;
	int data_start;
	int crc_start = 0;
	int crc = 0;
	int a = 0;
	Header header;
	if (idx < 0) 
	{
		text += "";
	}
	else 
	{
		if (bits.size() - idx <= 20) 
		{
			text += "No message";
			return;
		}
		else 
		{
			a = get_short(idx);
			header = (*(Header*)&a);
			if (header.extended) {
				a = get_word(idx);
				std::cout << "a > 16" << std::endl;
				//header = get_word(idx);
				//if (header >> )
			} //else header = (*(Header*)&a);

			text += "\r\nHeader:0x" + int_to_hex(a, 4) + "\n";

			if (sopFlag) {
				text += header.port__cable ? "From SRC" : "From SNK";
				text += header.port_data_role ? "/DFP " : "/UFP ";
			} else text += header.port__cable ? "From Cable Plug or VPD " : "From DFP or UFP ";

			text += "Message ID: " + std::to_string(header.message_id) + " \n";

			if (header.number_of_data_object == 0) {
				text += "Control Message: " + CTRL_TYPES[header.message_type] + "\n";
				crc_start = idx + 20;
			}
			else {
				text += "Data Message: " + DATA_TYPES[header.message_type] + "\n";
				sizeOfDataBite = header.number_of_data_object * 40;
				data_start = idx + 20;
				crc_start = data_start + sizeOfDataBite;
				switch (header.message_type) 
				{
				case 1 :
					printDataObj(&PacketUSBC::get_source_cap, header.number_of_data_object, data_start);
					break;
				case 2: 
					printDataObj(&PacketUSBC::get_request, header.number_of_data_object, data_start);
					break;
				case 3:
					printDataObj(&PacketUSBC::get_bist, header.number_of_data_object, data_start);
					break;
				case 4:
					printDataObj(&PacketUSBC::get_sink, header.number_of_data_object, data_start);
					break;
				case 5:
					printDataObj(&PacketUSBC::get_battery, header.number_of_data_object, data_start);
					break;
				case 6: 
					printDataObj(&PacketUSBC::get_alert, header.number_of_data_object, data_start);
					break;
				case 7:
					printDataObj(&PacketUSBC::get_country, header.number_of_data_object, data_start);
					break;
				case 15:
					printDataObj(&PacketUSBC::get_vdo_header, 32, data_start);
					break;
				default:
					;
				}
			}
			crc = get_word(crc_start);
			text += "\r\nCRC 0x" + int_to_hex(crc, 8);
		}
	}
}

void PacketUSBC::getStringVector() {
	std::vector<bool>::iterator it;
	for (it = bits.begin(); it != bits.end(); it++)
		std::cout << *it;
	std::cout << std::endl;
}

int main() {
	PacketUSBC packet;
	const int size = 11;

	//const unsigned char mas[size] = { 0x18, 0xe3, 0x98, 0x6c, 0x5a, 0x9a, 0xa6, 0x99, 0xfc, 0xa5, 0x9a, 0xa6, 0x4d, 0xbd, 0xf7, 0x9a, 0x26, 0x5d, 0xbd, 0xf7, 0x9a, 0xa6, 0xab, 0xbc, 0xf7, 0x9a, 0x26, 0xe5, 0xbc, 0xf7, 0x35, 0xc9, 0xb4, 0xef, 0x7d, 0x0d };
	const unsigned char mas[size] = {0x18, 0x1b, 0x93, 0x54, 0xf2, 0x9b, 0xea, 0xa5, 0x6f, 0xdf, 0x0d};
	//const unsigned char mas[size] = { 0x8c, 0x71, 0xa4, 0xca, 0x27, 0x4d, 0xd3, 0x55, 0x7e, 0x25, 0xe9, 0x65, 0xbd, 0x4f, 0xdd, 0x06 };
	//const unsigned char mas[size] = { 0x18, 0x1b, 0x93, 0x94, 0xf7, 0xf7, 0x6a, 0x77, 0xaf, 0xb4, 0x0d };
	//const unsigned char mas[size] = { 0x8c, 0x71, 0x4c, 0xca, 0xfb, 0x7b, 0xb5, 0xbb, 0x57, 0xda, 0x06 };
	packet.parse(mas, size);
	packet.getStringVector();
	std::string txt = packet.getText();
	std::cout << txt;
	return 0;

}