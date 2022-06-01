//
// Created by stephane bourque on 2022-05-25.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {

	enum ieee80211_eid {
		WLAN_EID_SSID = 0,
		WLAN_EID_SUPP_RATES = 1,
		WLAN_EID_FH_PARAMS = 2, /* reserved now */
		WLAN_EID_DS_PARAMS = 3,
		WLAN_EID_CF_PARAMS = 4,
		WLAN_EID_TIM = 5,
		WLAN_EID_IBSS_PARAMS = 6,
		WLAN_EID_COUNTRY = 7,
		/* 8, 9 reserved */
		WLAN_EID_REQUEST = 10,
		WLAN_EID_QBSS_LOAD = 11,
		WLAN_EID_EDCA_PARAM_SET = 12,
		WLAN_EID_TSPEC = 13,
		WLAN_EID_TCLAS = 14,
		WLAN_EID_SCHEDULE = 15,
		WLAN_EID_CHALLENGE = 16,
		/* 17-31 reserved for challenge text extension */
		WLAN_EID_PWR_CONSTRAINT = 32,
		WLAN_EID_PWR_CAPABILITY = 33,
		WLAN_EID_TPC_REQUEST = 34,
		WLAN_EID_TPC_REPORT = 35,
		WLAN_EID_SUPPORTED_CHANNELS = 36,
		WLAN_EID_CHANNEL_SWITCH = 37,
		WLAN_EID_MEASURE_REQUEST = 38,
		WLAN_EID_MEASURE_REPORT = 39,
		WLAN_EID_QUIET = 40,
		WLAN_EID_IBSS_DFS = 41,
		WLAN_EID_ERP_INFO = 42,
		WLAN_EID_TS_DELAY = 43,
		WLAN_EID_TCLAS_PROCESSING = 44,
		WLAN_EID_HT_CAPABILITY = 45,
		WLAN_EID_QOS_CAPA = 46,
		/* 47 reserved for Broadcom */
		WLAN_EID_RSN = 48,
		WLAN_EID_802_15_COEX = 49,
		WLAN_EID_EXT_SUPP_RATES = 50,
		WLAN_EID_AP_CHAN_REPORT = 51,
		WLAN_EID_NEIGHBOR_REPORT = 52,
		WLAN_EID_RCPI = 53,
		WLAN_EID_MOBILITY_DOMAIN = 54,
		WLAN_EID_FAST_BSS_TRANSITION = 55,
		WLAN_EID_TIMEOUT_INTERVAL = 56,
		WLAN_EID_RIC_DATA = 57,
		WLAN_EID_DSE_REGISTERED_LOCATION = 58,
		WLAN_EID_SUPPORTED_REGULATORY_CLASSES = 59,
		WLAN_EID_EXT_CHANSWITCH_ANN = 60,
		WLAN_EID_HT_OPERATION = 61,
		WLAN_EID_SECONDARY_CHANNEL_OFFSET = 62,
		WLAN_EID_BSS_AVG_ACCESS_DELAY = 63,
		WLAN_EID_ANTENNA_INFO = 64,
		WLAN_EID_RSNI = 65,
		WLAN_EID_MEASUREMENT_PILOT_TX_INFO = 66,
		WLAN_EID_BSS_AVAILABLE_CAPACITY = 67,
		WLAN_EID_BSS_AC_ACCESS_DELAY = 68,
		WLAN_EID_TIME_ADVERTISEMENT = 69,
		WLAN_EID_RRM_ENABLED_CAPABILITIES = 70,
		WLAN_EID_MULTIPLE_BSSID = 71,
		WLAN_EID_BSS_COEX_2040 = 72,
		WLAN_EID_BSS_INTOLERANT_CHL_REPORT = 73,
		WLAN_EID_OVERLAP_BSS_SCAN_PARAM = 74,
		WLAN_EID_RIC_DESCRIPTOR = 75,
		WLAN_EID_MMIE = 76,
		WLAN_EID_ASSOC_COMEBACK_TIME = 77,
		WLAN_EID_EVENT_REQUEST = 78,
		WLAN_EID_EVENT_REPORT = 79,
		WLAN_EID_DIAGNOSTIC_REQUEST = 80,
		WLAN_EID_DIAGNOSTIC_REPORT = 81,
		WLAN_EID_LOCATION_PARAMS = 82,
		WLAN_EID_NON_TX_BSSID_CAP =  83,
		WLAN_EID_SSID_LIST = 84,
		WLAN_EID_MULTI_BSSID_IDX = 85,
		WLAN_EID_FMS_DESCRIPTOR = 86,
		WLAN_EID_FMS_REQUEST = 87,
		WLAN_EID_FMS_RESPONSE = 88,
		WLAN_EID_QOS_TRAFFIC_CAPA = 89,
		WLAN_EID_BSS_MAX_IDLE_PERIOD = 90,
		WLAN_EID_TSF_REQUEST = 91,
		WLAN_EID_TSF_RESPOSNE = 92,
		WLAN_EID_WNM_SLEEP_MODE = 93,
		WLAN_EID_TIM_BCAST_REQ = 94,
		WLAN_EID_TIM_BCAST_RESP = 95,
		WLAN_EID_COLL_IF_REPORT = 96,
		WLAN_EID_CHANNEL_USAGE = 97,
		WLAN_EID_TIME_ZONE = 98,
		WLAN_EID_DMS_REQUEST = 99,
		WLAN_EID_DMS_RESPONSE = 100,
		WLAN_EID_LINK_ID = 101,
		WLAN_EID_WAKEUP_SCHEDUL = 102,
		/* 103 reserved */
		WLAN_EID_CHAN_SWITCH_TIMING = 104,
		WLAN_EID_PTI_CONTROL = 105,
		WLAN_EID_PU_BUFFER_STATUS = 106,
		WLAN_EID_INTERWORKING = 107,
		WLAN_EID_ADVERTISEMENT_PROTOCOL = 108,
		WLAN_EID_EXPEDITED_BW_REQ = 109,
		WLAN_EID_QOS_MAP_SET = 110,
		WLAN_EID_ROAMING_CONSORTIUM = 111,
		WLAN_EID_EMERGENCY_ALERT = 112,
		WLAN_EID_MESH_CONFIG = 113,
		WLAN_EID_MESH_ID = 114,
		WLAN_EID_LINK_METRIC_REPORT = 115,
		WLAN_EID_CONGESTION_NOTIFICATION = 116,
		WLAN_EID_PEER_MGMT = 117,
		WLAN_EID_CHAN_SWITCH_PARAM = 118,
		WLAN_EID_MESH_AWAKE_WINDOW = 119,
		WLAN_EID_BEACON_TIMING = 120,
		WLAN_EID_MCCAOP_SETUP_REQ = 121,
		WLAN_EID_MCCAOP_SETUP_RESP = 122,
		WLAN_EID_MCCAOP_ADVERT = 123,
		WLAN_EID_MCCAOP_TEARDOWN = 124,
		WLAN_EID_GANN = 125,
		WLAN_EID_RANN = 126,
		WLAN_EID_EXT_CAPABILITY = 127,
		/* 128, 129 reserved for Agere */
		WLAN_EID_PREQ = 130,
		WLAN_EID_PREP = 131,
		WLAN_EID_PERR = 132,
		/* 133-136 reserved for Cisco */
		WLAN_EID_PXU = 137,
		WLAN_EID_PXUC = 138,
		WLAN_EID_AUTH_MESH_PEER_EXCH = 139,
		WLAN_EID_MIC = 140,
		WLAN_EID_DESTINATION_URI = 141,
		WLAN_EID_UAPSD_COEX = 142,
		WLAN_EID_WAKEUP_SCHEDULE = 143,
		WLAN_EID_EXT_SCHEDULE = 144,
		WLAN_EID_STA_AVAILABILITY = 145,
		WLAN_EID_DMG_TSPEC = 146,
		WLAN_EID_DMG_AT = 147,
		WLAN_EID_DMG_CAP = 148,
		/* 149 reserved for Cisco */
		WLAN_EID_CISCO_VENDOR_SPECIFIC = 150,
		WLAN_EID_DMG_OPERATION = 151,
		WLAN_EID_DMG_BSS_PARAM_CHANGE = 152,
		WLAN_EID_DMG_BEAM_REFINEMENT = 153,
		WLAN_EID_CHANNEL_MEASURE_FEEDBACK = 154,
		/* 155-156 reserved for Cisco */
		WLAN_EID_AWAKE_WINDOW = 157,
		WLAN_EID_MULTI_BAND = 158,
		WLAN_EID_ADDBA_EXT = 159,
		WLAN_EID_NEXT_PCP_LIST = 160,
		WLAN_EID_PCP_HANDOVER = 161,
		WLAN_EID_DMG_LINK_MARGIN = 162,
		WLAN_EID_SWITCHING_STREAM = 163,
		WLAN_EID_SESSION_TRANSITION = 164,
		WLAN_EID_DYN_TONE_PAIRING_REPORT = 165,
		WLAN_EID_CLUSTER_REPORT = 166,
		WLAN_EID_RELAY_CAP = 167,
		WLAN_EID_RELAY_XFER_PARAM_SET = 168,
		WLAN_EID_BEAM_LINK_MAINT = 169,
		WLAN_EID_MULTIPLE_MAC_ADDR = 170,
		WLAN_EID_U_PID = 171,
		WLAN_EID_DMG_LINK_ADAPT_ACK = 172,
		/* 173 reserved for Symbol */
		WLAN_EID_MCCAOP_ADV_OVERVIEW = 174,
		WLAN_EID_QUIET_PERIOD_REQ = 175,
		/* 176 reserved for Symbol */
		WLAN_EID_QUIET_PERIOD_RESP = 177,
		/* 178-179 reserved for Symbol */
		/* 180 reserved for ISO/IEC 20011 */
		WLAN_EID_EPAC_POLICY = 182,
		WLAN_EID_CLISTER_TIME_OFF = 183,
		WLAN_EID_INTER_AC_PRIO = 184,
		WLAN_EID_SCS_DESCRIPTOR = 185,
		WLAN_EID_QLOAD_REPORT = 186,
		WLAN_EID_HCCA_TXOP_UPDATE_COUNT = 187,
		WLAN_EID_HL_STREAM_ID = 188,
		WLAN_EID_GCR_GROUP_ADDR = 189,
		WLAN_EID_ANTENNA_SECTOR_ID_PATTERN = 190,
		WLAN_EID_VHT_CAPABILITY = 191,
		WLAN_EID_VHT_OPERATION = 192,
		WLAN_EID_EXTENDED_BSS_LOAD = 193,
		WLAN_EID_WIDE_BW_CHANNEL_SWITCH = 194,
		WLAN_EID_TX_POWER_ENVELOPE = 195,
		WLAN_EID_CHANNEL_SWITCH_WRAPPER = 196,
		WLAN_EID_AID = 197,
		WLAN_EID_QUIET_CHANNEL = 198,
		WLAN_EID_OPMODE_NOTIF = 199,

		WLAN_EID_REDUCED_NEIGHBOR_REPORT = 201,

		WLAN_EID_AID_REQUEST = 210,
		WLAN_EID_AID_RESPONSE = 211,
		WLAN_EID_S1G_BCN_COMPAT = 213,
		WLAN_EID_S1G_SHORT_BCN_INTERVAL = 214,
		WLAN_EID_S1G_TWT = 216,
		WLAN_EID_S1G_CAPABILITIES = 217,
		WLAN_EID_VENDOR_SPECIFIC = 221,
		WLAN_EID_QOS_PARAMETER = 222,
		WLAN_EID_S1G_OPERATION = 232,
		WLAN_EID_CAG_NUMBER = 237,
		WLAN_EID_AP_CSN = 239,
		WLAN_EID_FILS_INDICATION = 240,
		WLAN_EID_DILS = 241,
		WLAN_EID_FRAGMENT = 242,
		WLAN_EID_RSNX = 244,
		WLAN_EID_EXTENSION = 255
	};

	/* Element ID Extensions for Element ID 255 */
	enum ieee80211_eid_ext {
		WLAN_EID_EXT_ASSOC_DELAY_INFO = 1,
		WLAN_EID_EXT_FILS_REQ_PARAMS = 2,
		WLAN_EID_EXT_FILS_KEY_CONFIRM = 3,
		WLAN_EID_EXT_FILS_SESSION = 4,
		WLAN_EID_EXT_FILS_HLP_CONTAINER = 5,
		WLAN_EID_EXT_FILS_IP_ADDR_ASSIGN = 6,
		WLAN_EID_EXT_KEY_DELIVERY = 7,
		WLAN_EID_EXT_FILS_WRAPPED_DATA = 8,
		WLAN_EID_EXT_FILS_PUBLIC_KEY = 12,
		WLAN_EID_EXT_FILS_NONCE = 13,
		WLAN_EID_EXT_FUTURE_CHAN_GUIDANCE = 14,
		WLAN_EID_EXT_HE_CAPABILITY = 35,
		WLAN_EID_EXT_HE_OPERATION = 36,
		WLAN_EID_EXT_UORA = 37,
		WLAN_EID_EXT_HE_MU_EDCA = 38,
		WLAN_EID_EXT_HE_SPR = 39,
		WLAN_EID_EXT_NDP_FEEDBACK_REPORT_PARAMSET = 41,
		WLAN_EID_EXT_BSS_COLOR_CHG_ANN = 42,
		WLAN_EID_EXT_QUIET_TIME_PERIOD_SETUP = 43,
		WLAN_EID_EXT_ESS_REPORT = 45,
		WLAN_EID_EXT_OPS = 46,
		WLAN_EID_EXT_HE_BSS_LOAD = 47,
		WLAN_EID_EXT_MAX_CHANNEL_SWITCH_TIME = 52,
		WLAN_EID_EXT_MULTIPLE_BSSID_CONFIGURATION = 55,
		WLAN_EID_EXT_NON_INHERITANCE = 56,
		WLAN_EID_EXT_KNOWN_BSSID = 57,
		WLAN_EID_EXT_SHORT_SSID_LIST = 58,
		WLAN_EID_EXT_HE_6GHZ_CAPA = 59,
		WLAN_EID_EXT_UL_MU_POWER_CAPA = 60,
		WLAN_EID_EXT_EHT_OPERATION = 106,
		WLAN_EID_EXT_EHT_MULTI_LINK = 107,
		WLAN_EID_EXT_EHT_CAPABILITY = 108,
	};


	inline std::vector<unsigned char> Base64Decode2Vec(const std::string &F) {
		std::istringstream ifs(F);
		Poco::Base64Decoder b64in(ifs);
		std::ostringstream ofs;
		Poco::StreamCopier::copyStream(b64in, ofs);
		std::vector<unsigned char> r;
		auto s = ofs.str();
		for (const auto &c : s) {
			r.push_back((unsigned char)c);
		}
		return r;
	}

	inline nlohmann::json WFS_WLAN_EID_COUNTRY(const std::vector<unsigned char> &data) {
		nlohmann::json new_ie;
		std::string CountryName;
		CountryName += (char)data[0];
		CountryName += (char)data[1];
		nlohmann::json ie_data;
		nlohmann::json constraints = nlohmann::json::array();
		for (std::size_t i = 3; (i+3)<= data.size(); i += 3) {
			nlohmann::json constraint;
			constraint["first_channel"] = (uint64_t)data[i];
			constraint["number_of_channel"] = (uint64_t)data[i + 1];
			constraint["max_tx_power"] = (uint64_t)data[i + 2];
			constraints.push_back(constraint);
		}
		ie_data["country"] = CountryName;
		ie_data["constraints"] = constraints;
		new_ie["content"] = ie_data;
		new_ie["name"] = "country";
		new_ie["type"] = WLAN_EID_COUNTRY;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_SUPP_RATES(const std::vector<unsigned char> &data) {
		nlohmann::json 	Rates;
		nlohmann::json 	new_ie;
		for(const auto &c:data) {
			nlohmann::json 	Rate;
			bool Mandatory = c & 0x01;
			Rate["mandatory"] = Mandatory;
			uint  BitRate = c >> 1;
			if(BitRate==2)
				Rate["rate"]="1 Mbps";
			else if (BitRate==4)
				Rate["rate"]="2 Mbps";
			else if (BitRate==11)
				Rate["rate"]="5.5 Mbps";
			else if (BitRate==12)
				Rate["rate"]= "6 Mbps";
			else if (BitRate==18)
				Rate["rate"] = "9 Mbps";
			else if (BitRate==22)
				Rate["rate"] = "11 Mbps";
			else if (BitRate==24)
				Rate["rate"] = "12 Mbps";
			else if (BitRate==36)
				Rate["rate"] = "18 Mbps";
			else if (BitRate==44)
				Rate["rate"] = "22 Mbps";
			else if (BitRate==48)
				Rate["rate"] = "24 Mbps";
			else if (BitRate==66)
				Rate["rate"] = "33 Mbps";
			else if (BitRate==72)
				Rate["rate"] = "36 Mbps";
			else if (BitRate==96)
				Rate["rate"] = "48 Mbps";
			else if (BitRate==108)
				Rate["rate"] = "54 Mbps";
			else
				Rate["rate"] = "> 108 Mbps";
			Rates.push_back(Rate);
		}
		new_ie["name"]="supported_rates";
		new_ie["content"]=Rates;
		new_ie["type"]=WLAN_EID_SUPP_RATES;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_FH_PARAMS(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		content["dwell_time"] = (uint64_t) (data[0] * 256 + data[1]);
		content["hop_set"] = (uint)data[2];
		content["hop_pattern"] = (uint)data[3];
		content["hop_index"] = (uint)data[4];
		new_ie["name"]="FH Params";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_FH_PARAMS;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_DS_PARAMS(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		content["current_channel"] = (uint64_t) data[0];
		new_ie["name"]="DS Params";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_DS_PARAMS;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_TIM(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		Poco::JSON::Object ie_data;
		content["DTIM_count"] = (uint64_t) data[0];
		content["DTIM_period"] = (uint64_t) data[1];
		new_ie["name"]="Traffic Indication Map";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_TIM;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_QBSS_LOAD(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		if(data.size()==5) {
			content["QBSS_Version"] = (uint64_t)(data[0]);
			content["Station_Count"] = (uint)( data[2] + data[1]*256);
			content["Channel_Utilization"] = (uint) data[3];
			content["Available_Admission_Capabilities"] = (uint) data[4];
		}
		new_ie["name"]="QBSS Load";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_QBSS_LOAD;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_PWR_CONSTRAINT(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		content["Local_Power_Constraint"] = (uint) data[0];
		new_ie["name"]="Local Power Constraint";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_PWR_CONSTRAINT;
		return new_ie;
	}

	bool bitSet(unsigned char c, uint bit) {
		switch (bit) {
		case 0: return (c & 0x01);
		case 1: return (c & 0x02);
		case 2: return (c & 0x04);
		case 3: return (c & 0x08);
		case 4: return (c & 0x10);
		case 5: return (c & 0x20);
		case 6: return (c & 0x40);
		case 7: return (c & 0x90);
		default: return false;
		}
	}

	bool bitSet(uint16_t c, uint bit) {
		switch (bit) {
		case 0: return  (c & 0x0001);
		case 1: return  (c & 0x0002);
		case 2: return  (c & 0x0004);
		case 3: return  (c & 0x0008);
		case 4: return  (c & 0x0010);
		case 5: return  (c & 0x0020);
		case 6: return  (c & 0x0040);
		case 7: return  (c & 0x0080);
		case 8: return  (c & 0x0100);
		case 9: return  (c & 0x0200);
		case 10: return (c & 0x0400);
		case 11: return (c & 0x0800);
		case 12: return (c & 0x1000);
		case 13: return (c & 0x2000);
		case 14: return (c & 0x4000);
		case 15: return (c & 0x8000);
		default: return false;
		}
	}

	const static std::vector<std::pair<uint8_t, std::string>> ieee80211_supported_rates_vals = {
		{ 0x02, "1" },
		{ 0x03, "1.5" },
		{ 0x04, "2" },
		{ 0x05, "2.5" },
		{ 0x06, "3" },
		{ 0x09, "4.5" },
		{ 0x0B, "5.5" },
		{ 0x0C, "6" },
		{ 0x12, "9" },
		{ 0x16, "11" },
		{ 0x18, "12" },
		{ 0x1B, "13.5" },
		{ 0x24, "18" },
		{ 0x2C, "22" },
		{ 0x30, "24" },
		{ 0x36, "27" },
		{ 0x42, "33" },
		{ 0x48, "36" },
		{ 0x60, "48" },
		{ 0x6C, "54" },
		{ 0x82, "1(B)" },
		{ 0x83, "1.5(B)" },
		{ 0x84, "2(B)" },
		{ 0x85, "2.5(B)" },
		{ 0x86, "3(B)" },
		{ 0x89, "4.5(B)" },
		{ 0x8B, "5.5(B)" },
		{ 0x8C, "6(B)" },
		{ 0x92, "9(B)" },
		{ 0x96, "11(B)" },
		{ 0x98, "12(B)" },
		{ 0x9B, "13.5(B)" },
		{ 0xA4, "18(B)" },
		{ 0xAC, "22(B)" },
		{ 0xB0, "24(B)" },
		{ 0xB6, "27(B)" },
		{ 0xC2, "33(B)" },
		{ 0xC8, "36(B)" },
		{ 0xE0, "48(B)" },
		{ 0xEC, "54(B)" },
		{ 0xFF, "BSS requires support for mandatory features of HT PHY (IEEE 802.11 - Clause 20)" }
	};

	inline nlohmann::json WFS_WLAN_EID_ERP_INFO(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		content["Non_ERP_Present"] = bitSet(data[0],0);
		content["Use_Protection"] = bitSet(data[0],1);
		content["Barker_Preamble_Mode"] = bitSet(data[0],2);
		new_ie["name"]="ERP Information";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_ERP_INFO;
		return new_ie;
	}

	std::string BufferToHex(const std::vector<unsigned char> &b) {
		static const char hex[] = "0123456789abcdef";
		std::string result;
		for(const auto &c:b) {
			if(!result.empty())
				result += ' ';
			result += (hex[ (c & 0xf0) >> 4]);
			result += (hex[ (c & 0x0f) ]);
		}
		return result;
	}

	std::string bitString(unsigned char c) {
		std::string R;
		for(std::size_t i=0;i<8;i++) {
			if(c & 0x80)
				R += '1';
			else
				R += '0';
			c <<= 1;
		}
		return R;
	}

	inline nlohmann::json WFS_WLAN_EID_SUPPORTED_REGULATORY_CLASSES(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		content["Supported_Regulatory_Classes"] = BufferToHex(data);
		new_ie["name"]="Supported Regulatory Classes";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_SUPPORTED_REGULATORY_CLASSES;
		return new_ie;
	}

	inline void ParseMCSset(const unsigned char * data, nlohmann::json & content) {
		content["MCS Set"]["Rx Bitmask Bits 0-7"] = bitString(data[0]);
		content["MCS Set"]["Rx Bitmask Bits 8-15"] = bitString(data[1]);
		content["MCS Set"]["Rx Bitmask Bits 16-23"] = bitString(data[2]);
		content["MCS Set"]["Rx Bitmask Bits 24-31"] = bitString(data[3]);
	}

	using value_string = std::vector<std::pair<uint,const char *>>;

	static const value_string txbf_antenna_flags = {
		{0x00, "1 TX antenna sounding"},
		{0x01, "2 TX antenna sounding"},
		{0x02, "3 TX antenna sounding"},
		{0x03, "4 TX antenna sounding"},
		{0x00, NULL}
	};

	static const value_string txbf_feedback_flags = {
		{0x00, "not supported"},
		{0x01, "delayed feedback capable"},
		{0x02, "immediate feedback capable"},
		{0x03, "delayed and immediate feedback capable"},
		{0x00, NULL}
	};

	static const value_string txbf_calib_flag = {
		{0x00, "incapable"},
		{0x01, "Limited involvement, cannot initiate"},
		{0x02, "Limited involvement, can initiate"},
		{0x03, "Fully capable"},
		{0x00, NULL}
	};

	static const value_string txbf_csi_max_rows_bf_flags = {
		{0x00, "1 row of CSI"},
		{0x01, "2 rows of CSI"},
		{0x02, "3 rows of CSI"},
		{0x03, "4 rows of CSI"},
		{0x00, NULL}
	};

	static const value_string txbf_chan_est_flags = {
		{0x00, "1 space time stream"},
		{0x01, "2 space time streams"},
		{0x02, "3 space time streams"},
		{0x03, "4 space time streams"},
		{0x00, NULL}
	};

	static const value_string txbf_min_group_flags = {
		{0x00, "No grouping supported"},
		{0x01, "Groups of 1,2 supported"},
		{0x02, "Groups of 1,4 supported"},
		{0x03, "Groups of 1,2,4 supported"},
		{0x00, NULL}
	};

	static const value_string vht_max_mpdu_length_flag = {
		{0x00, "3 895"},
		{0x01, "7 991"},
		{0x02, "11 454"},
		{0x03, "Reserved"},
		{0x00, NULL}
	};

	static const value_string vht_supported_chan_width_set_flag = {
		{0x00, "Neither 160MHz nor 80+80 supported"},
		{0x01, "160MHz supported"},
		{0x02, "160MHz and 80+80 Supported"},
		{0x03, "Reserved"},
		{0x00, NULL}
	};

	static const value_string vht_rx_stbc_flag = {
		{0x00, "None"},
		{0x01, "1 Spatial Stream Supported"},
		{0x02, "1 to 2 Spatial Stream Supported"},
		{0x03, "1 to 3 Spatial Stream Supported"},
		{0x04, "1 to 4 Spatial Stream Supported"},
		{0x02, "160MHz and 80+80 Supported"},
		{0x05, "Reserved"},
		{0x06, "Reserved"},
		{0x07, "Reserved"},
		{0x00, NULL}
	};

	static const value_string num_plus_one_3bit_flag = {
		{0x00, "1"},
		{0x01, "2"},
		{0x02, "3"},
		{0x03, "4"},
		{0x04, "5"},
		{0x05, "6"},
		{0x06, "7"},
		{0x07, "8"},
		{0x00, NULL}
	};

	static const value_string vht_max_ampdu_flag = {
		{0x00, "8 191"},
		{0x01, "16 383"},
		{0x02, "32 767"},
		{0x03, "65,535"},
		{0x04, "131 071"},
		{0x05, "262 143"},
		{0x06, "524 287"},
		{0x07, "1 048 575"},
		{0x00, NULL}
	};

	static const value_string vht_link_adapt_flag = {
		{0x00, "No Feedback"},
		{0x01, "Reserved (logically only solicited feedback)"},
		{0x02, "Unsolicited feedback only"},
		{0x03, "Both (can provide unsolicited feedback and respond to VHT MRQ)"},
		{0x00, NULL}
	};

	const char * VALS(const value_string &vals, uint v) {
		for(const auto &e:vals) {
			if(e.first==v && e.second!=NULL)
				return e.second;
		}
		return "unknown";
	}

	inline nlohmann::json WFS_WLAN_EID_HT_CAPABILITY(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;

		if(data.size()==26) {
			uint offset = 0 ;
			uint16_t ht_caps = data[offset+1] * 256 + data[offset];
			content["HT Capabilities Info"]["HT LDPC coding capability"] = bitSet(ht_caps,0);
			content["HT Capabilities Info"]["HT Support channel width"] = bitSet(ht_caps,1);
			content["HT Capabilities Info"]["HT Green Field"] = (ht_caps & 0x00c0) >> 4;
			content["HT Capabilities Info"]["HT Short GI for 20MHz"] = bitSet(ht_caps,5);
			content["HT Capabilities Info"]["HT Short GI for 40MHz"] = bitSet(ht_caps,6);
			content["HT Capabilities Info"]["HT Tx STBC"] = bitSet(ht_caps,7);
			content["HT Capabilities Info"]["HT Rx STBC"] = (ht_caps & 0x0300) >> 8;
			content["HT Capabilities Info"]["HT Delayed Block ACK"] = bitSet(ht_caps,11);
			content["HT Capabilities Info"]["HT Max A-MSDU length"] = bitSet(ht_caps,12);
			content["HT Capabilities Info"]["HT PSMP Support"] = bitSet(ht_caps,13);
			content["HT Capabilities Info"]["HT Forty MHz Intolerant"] = bitSet(ht_caps,14);
			content["HT Capabilities Info"]["HT L-SIG TXOP Protection support"] = bitSet(ht_caps,15);
			offset += 2;

			auto ampduparam = data[offset];
			content["A-MPDU Parameters"]["Maximum Rx A-MPDU Length"] = ampduparam & 0x03;
			content["A-MPDU Parameters"]["MPDU Density"] = (ampduparam & 0x1c) >> 2;
			offset += 1;

			ParseMCSset(&data[offset], content);
			offset += 16;

			content["HT Extended Capabilities"]["Transmitter supports PCO"] = data[offset] & 0x01;
			content["HT Extended Capabilities"]["Time needed to transition between 20MHz and 40MHz"] = (data[offset] & 0x06) >> 1;
			offset++;
			content["HT Extended Capabilities"]["MCS Feedback capability"] = data[offset] & 0x03;
			content["HT Extended Capabilities"]["High Throughput"] = (data[offset] & 0x04) >> 2;
			content["HT Extended Capabilities"]["Reverse Direction Responder"] = (data[offset] & 0x08) >> 3;
			offset++;

			uint32_t caps = data[offset] + data[offset+1]*256 + data[offset+2] * 256 * 256 + data[offset+3] * 256 * 256 * 256 ;

			content["Transmit Beam Forming (TxBF) Capabilities"]["Transmit Beamforming"] = (caps & 0x00000001) >> 0;
			content["Transmit Beam Forming (TxBF) Capabilities"]["Receive Staggered Sounding"] = (caps & 0x00000002) >> 1;
			content["Transmit Beam Forming (TxBF) Capabilities"]["Transmit Staggered Sounding"] = (caps & 0x00000004) >> 2;
			content["Transmit Beam Forming (TxBF) Capabilities"]["Receive Null Data packet (NDP)"] = (caps & 0x00000008) >> 3;
			content["Transmit Beam Forming (TxBF) Capabilities"]["Transmit Null Data packet (NDP)"] = (caps & 0x00000010) >> 4;
			content["Transmit Beam Forming (TxBF) Capabilities"]["Implicit TxBF capable"] = (caps & 0x00000020) >> 5;
			content["Transmit Beam Forming (TxBF) Capabilities"]["Calibration"] = VALS(txbf_calib_flag,(caps & 0x000000c0) >> 6);
			content["Transmit Beam Forming (TxBF) Capabilities"]["STA can apply TxBF using CSI explicit feedback"] = (caps & 0x00000100) >> 8;
			content["Transmit Beam Forming (TxBF) Capabilities"]["STA can apply TxBF using uncompressed beamforming feedback matrix"] = (caps & 0x00000200) >> 9;
			content["Transmit Beam Forming (TxBF) Capabilities"]["STA can apply TxBF using compressed beamforming feedback matrix"] = (caps & 0x00000400) >> 10;
			content["Transmit Beam Forming (TxBF) Capabilities"]["Receiver can return explicit CSI feedback"] = VALS(txbf_feedback_flags,(caps & 0x00001800) >> 11);
			content["Transmit Beam Forming (TxBF) Capabilities"]["Receiver can return explicit uncompressed Beamforming Feedback Matrix"] = VALS(txbf_feedback_flags,(caps & 0x00006000) >>13);
			content["Transmit Beam Forming (TxBF) Capabilities"]["STA can compress and use compressed Beamforming Feedback Matrix"] = VALS(txbf_feedback_flags,(caps & 0x00018000) >> 15);
			content["Transmit Beam Forming (TxBF) Capabilities"]["Minimal grouping used for explicit feedback reports"] = VALS(txbf_min_group_flags,(caps & 0x00060000) >> 17);
			content["Transmit Beam Forming (TxBF) Capabilities"]["Max antennae STA can support when CSI feedback required"] = VALS(txbf_antenna_flags,(caps & 0x00180000) >> 19);
			content["Transmit Beam Forming (TxBF) Capabilities"]["Max antennae STA can support when uncompressed Beamforming feedback required"] = VALS(txbf_antenna_flags,(caps & 0x00600000) >> 21);
			content["Transmit Beam Forming (TxBF) Capabilities"]["Max antennae STA can support when compressed Beamforming feedback required"] = VALS(txbf_antenna_flags,(caps & 0x01800000) >> 23);
			content["Transmit Beam Forming (TxBF) Capabilities"]["Maximum number of rows of CSI explicit feedback"] = VALS(txbf_antenna_flags,(caps & 0x06000000) >> 25);
			content["Transmit Beam Forming (TxBF) Capabilities"]["Maximum number of space time streams for which channel dimensions can be simultaneously estimated"] = VALS(txbf_chan_est_flags,(caps & 0x18000000) >> 27);
			offset+=4;

			caps = data[offset];
			content["Antenna Selection (ASEL) Capabilities"]["Antenna Selection Capable"] = (caps & 0x00000001) >> 0;
			content["Antenna Selection (ASEL) Capabilities"]["Antenna Selection Capable TXCSI"] = (caps & 0x00000002) >> 1;
			content["Antenna Selection (ASEL) Capabilities"]["Antenna Indices Feedback Based Tx ASEL"] = (caps & 0x00000004) >> 2;
			content["Antenna Selection (ASEL) Capabilities"]["Explicit CSI Feedback"] = (caps & 0x00000008) >> 3;
			content["Antenna Selection (ASEL) Capabilities"]["Antenna Indices Feedback"] = (caps & 0x00000010) >> 4;
			content["Antenna Selection (ASEL) Capabilities"]["Rx ASEL"] = (caps & 0x00000020) >> 5;
			content["Antenna Selection (ASEL) Capabilities"]["Tx Sounding PPDUs"] = (caps & 0x00000040) >> 6;

		}

		new_ie["name"]="HT Capabilities";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_HT_CAPABILITY;
		return new_ie;
	}

	std::string GetRate(unsigned char R) {
		for(const auto &[rate,rate_name]:ieee80211_supported_rates_vals) {
			if(rate==R)
				return rate_name;
		}
		return "unknown";
	}

	inline nlohmann::json WFS_WLAN_EID_EXT_SUPP_RATES(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		std::string Rates;

		for(const auto &rate:data) {
			if(!Rates.empty())
				Rates += ", ";
			Rates += GetRate(rate);
		}
		Rates +=  " [Mbit/sec]";
		content["Supported Rates"] = Rates;
		new_ie["name"]="Supported Rates";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_EXT_SUPP_RATES;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_TX_POWER_ENVELOPE(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;

		if(data.size()>=2 && data.size()<=5) {
			auto len = data[0];
			for(uint i=0;i<=len;i++) {
				switch(i) {
				case 0:
					content["Tx Pwr Info"]["Local Max Tx Pwr Constraint 20MHz"] = (uint16_t) data[i+1];
					break;
				case 1:
					content["Tx Pwr Info"]["Local Max Tx Pwr Constraint 40MHz"] = (uint16_t) data[i+1];
					break;
				case 2:
					content["Tx Pwr Info"]["Local Max Tx Pwr Constraint 80MHz"] = (uint16_t) data[i+1];
					break;
				case 3:
					content["Tx Pwr Info"]["Local Max Tx Pwr Constraint 160MHz/80+80 MHz"] = (uint16_t) data[i+1];
					break;
				default:
					content["Tx Pwr Info"]["Local Max Tx Pwr Constraint 160MHz/80+80 MHz"] = (uint16_t) 0xff;
				}
			}
		}
		new_ie["name"]="Tx Pwr Info";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_TX_POWER_ENVELOPE;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_VHT_CAPABILITY(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;

		if(data.size()==12) {
			uint offset=0;
			uint caps = data[offset] + data[offset+1]* 256 + data[offset+2]*256*256 + data[offset+3]*256*256*256;

			content["VHT Capabilities Info"]["Maximum MPDU Length"] = VALS(vht_max_mpdu_length_flag, (caps & 0x00000003) >> 0 );
			content["VHT Capabilities Info"]["Supported Channel Width Set"] = VALS(vht_supported_chan_width_set_flag, (caps & 0x0000000c) >> 2 );
			content["VHT Capabilities Info"]["Rx LDPC"] = (caps & 0x00000010) >> 4 ;
			content["VHT Capabilities Info"]["Short GI for 80MHz"] = (caps & 0x00000020) >> 5 ;
			content["VHT Capabilities Info"]["Short GI for 160MHz and 80+80MHz"] = (caps & 0x00000040) >> 6 ;
			content["VHT Capabilities Info"]["Tx STBC"] = (caps & 0x00000080) >> 7 ;

			content["VHT Capabilities Info"]["Rx STBC"] = VALS(vht_rx_stbc_flag,(caps & 0x00000700) >> 8) ;
			content["VHT Capabilities Info"]["SU Beam-former Capable"] = (caps & 0x00000800) >> 11 ;
			content["VHT Capabilities Info"]["SU Beam-formee Capable"] = (caps & 0x00001000) >> 12 ;
			content["VHT Capabilities Info"]["Compressed Steering Number of Beamformer Antennas Supported"] = VALS(num_plus_one_3bit_flag,(caps & 0x0000e000) >> 13) ;

			content["VHT Capabilities Info"]["Number of Sounding Dimensions"] = VALS(num_plus_one_3bit_flag,(caps & 0x00070000) >> 16);
			content["VHT Capabilities Info"]["MU Beam-former Capable"] = VALS(num_plus_one_3bit_flag,(caps & 0x00080000) >> 17);
			content["VHT Capabilities Info"]["MU Beam-formee Capable"] = (caps & 0x00200000) >> 18;
			content["VHT Capabilities Info"]["VHT TXOP PS"] = (caps & 0x00400000) >> 19;
			content["VHT Capabilities Info"]["HTC-VHT Capable VHT variant HT Control field"] = (caps & 0x00400000) >> 20;

			content["VHT Capabilities Info"]["Max A-MPDU Length"] = VALS(vht_max_ampdu_flag,(caps & 0x03800000) >> 21);
			content["VHT Capabilities Info"]["VHT Link Adaptation"] = VALS(vht_link_adapt_flag,(caps & 0x0c000000) >> 24);

			content["VHT Capabilities Info"]["Rx Antenna Pattern Consistency"] = (caps & 0x10000000) >> 28;
			content["VHT Capabilities Info"]["Tx Antenna Pattern Consistency"] = (caps & 0x20000000) >> 29;
			offset += 4;
			ParseMCSset(&data[offset],content);
		}

		new_ie["name"]="VHT Capabilities Info";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_VHT_CAPABILITY;
		return new_ie;
	}

	inline bool ParseWifiScan(Poco::JSON::Object::Ptr &Obj, std::stringstream &Result, Poco::Logger &Logger) {
		std::ostringstream	ofs;
		Obj->stringify(ofs);

		nlohmann::json D = nlohmann::json::parse(ofs.str());
		std::cout << "Start of parsing wifi" << std::endl;
		if (D.contains("status")) {
			auto Status = D["status"];
			if (Status.contains("scan") && Status["scan"].is_array()) {
				nlohmann::json ScanArray = Status["scan"];
				nlohmann::json ParsedScan = nlohmann::json::array();
				for (auto &scan_entry : ScanArray) {
					if (scan_entry.contains("ies") && scan_entry["ies"].is_array()) {
						auto ies = scan_entry["ies"];
						nlohmann::json 	new_ies=nlohmann::json::array();
						for (auto &ie : ies) {
							try {
								if (ie.contains("type") && ie.contains("data")) {
									uint64_t ie_type = ie["type"];
									std::string ie_data = ie["data"];
									// std::cout << "TYPE:" << ie_type << "  DATA:" << ie_data << std::endl;
									auto data = Base64Decode2Vec(ie_data);
									if (ie_type == ieee80211_eid::WLAN_EID_COUNTRY) {
										new_ies.push_back(WFS_WLAN_EID_COUNTRY(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_SUPP_RATES) {
										new_ies.push_back(WFS_WLAN_EID_SUPP_RATES(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_FH_PARAMS) {
										new_ies.push_back(WFS_WLAN_EID_FH_PARAMS(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_DS_PARAMS) {
										new_ies.push_back(WFS_WLAN_EID_DS_PARAMS(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_TIM) {
										new_ies.push_back(WFS_WLAN_EID_TIM(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_QBSS_LOAD) {
										new_ies.push_back(WFS_WLAN_EID_QBSS_LOAD(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_PWR_CONSTRAINT) {
										new_ies.push_back(WFS_WLAN_EID_PWR_CONSTRAINT(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_ERP_INFO) {
										new_ies.push_back(WFS_WLAN_EID_ERP_INFO(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_SUPPORTED_REGULATORY_CLASSES) {
										new_ies.push_back(WFS_WLAN_EID_SUPPORTED_REGULATORY_CLASSES(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_HT_CAPABILITY) {
										new_ies.push_back(WFS_WLAN_EID_HT_CAPABILITY(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_EXT_SUPP_RATES) {
										new_ies.push_back(WFS_WLAN_EID_EXT_SUPP_RATES(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_TX_POWER_ENVELOPE) {
										new_ies.push_back(WFS_WLAN_EID_TX_POWER_ENVELOPE(data));
									} else if (ie_type == ieee80211_eid::WLAN_EID_VHT_CAPABILITY) {
										new_ies.push_back(WFS_WLAN_EID_VHT_CAPABILITY(data));
									} else {
											std::cout << "Skipping IE: no parsing available: " << ie_type << std::endl;
											new_ies.push_back(ie);
									}
								} else {
									std::cout << "Skipping IE: no data and type" << std::endl;
									new_ies.push_back(ie);
								}
							} catch (...) {
								std::cout << "Skipping IE: exception" << std::endl;
								Logger.information(fmt::format("Error parsing IEs"));
								new_ies.push_back(ie);
							}
						}
						scan_entry["ies"] = new_ies;
						ParsedScan.push_back(scan_entry);
					} else {
						std::cout << "Skipping scan" << std::endl;
						ParsedScan.push_back(scan_entry);
					}
				}
				Status["scan"] = ParsedScan;
				D["status"] = Status;
			}
		}
		Result << to_string(D);
		return false;
	}

} // namespace OpenWifi

