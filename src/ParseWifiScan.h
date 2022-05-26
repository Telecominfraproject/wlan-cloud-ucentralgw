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

	inline void WFS_WLAN_EID_COUNTRY(const std::vector<unsigned char> &data, Poco::JSON::Object &new_ie) {
		std::string CountryName;
		CountryName += (char)data[0];
		CountryName += (char)data[1];
		Poco::JSON::Object ie_data;
		Poco::JSON::Array constraints;
		for (std::size_t i = 3; i < data.size(); i += 3) {
			Poco::JSON::Object Inner;
			Inner.set("first_channel", (uint64_t)data[i]);
			Inner.set("number_of_channel", (uint64_t)data[i + 1]);
			Inner.set("max_tx_power", (uint64_t)data[i + 2]);
			constraints.add(Inner);
		}
		ie_data.set("country", CountryName);
		// ie_data.set("constraints", constraints);
		// new_ie.set("data", ie_data);
		new_ie.set("name", "country");
		new_ie.set("type", WLAN_EID_COUNTRY);
	}

	inline void WFS_WLAN_EID_EXT_SUPP_RATES(const std::vector<unsigned char> &data, Poco::JSON::Object &new_ie) {
		Poco::JSON::Array	Rates;
		for(const auto &c:data) {
			Poco::JSON::Object	Rate;
			bool Mandatory = c & 0x01;
			Rate.set("mandatory", Mandatory);
			uint  BitRate = c >> 1;
			if(BitRate==2)
				Rate.set("rate","1 Mbps");
			else if (BitRate==4)
				Rate.set("rate","2 Mbps");
			else if (BitRate==11)
				Rate.set("rate","5.5 Mbps");
			else if (BitRate==12)
				Rate.set("rate","6 Mbps");
			else if (BitRate==18)
				Rate.set("rate","9 Mbps");
			else if (BitRate==22)
				Rate.set("rate","11 Mbps");
			else if (BitRate==24)
				Rate.set("rate","12 Mbps");
			else if (BitRate==36)
				Rate.set("rate","18 Mbps");
			else if (BitRate==44)
				Rate.set("rate","22 Mbps");
			else if (BitRate==48)
				Rate.set("rate","24 Mbps");
			else if (BitRate==66)
				Rate.set("rate","33 Mbps");
			else if (BitRate==72)
				Rate.set("rate","36 Mbps");
			else if (BitRate==96)
				Rate.set("rate","48 Mbps");
			else if (BitRate==108)
				Rate.set("rate","54 Mbps");
			else
				Rate.set("rate","> 108 Mbps");
			Rates.add(Rate);
		}
		new_ie.set("name", "supported_rates");
		new_ie.set("data", Rates);
		new_ie.set("type", WLAN_EID_SUPP_RATES);
	}

	inline void WFS_WLAN_EID_FH_PARAMS(const std::vector<unsigned char> &data, Poco::JSON::Object &new_ie) {
		Poco::JSON::Object ie_data;
		ie_data.set("dwell_time", (uint64_t) (data[0] * 256 + data[1]) );
		ie_data.set("hop_set",(uint)data[2]);
		ie_data.set("hop_pattern",(uint)data[3]);
		ie_data.set("hop_index",(uint)data[4]);
		new_ie.set("data", ie_data);
		new_ie.set("name", "FH_Parameter_Set");
		new_ie.set("type", WLAN_EID_FH_PARAMS);
	}

	inline void WFS_WLAN_EID_DS_PARAMS(const std::vector<unsigned char> &data, Poco::JSON::Object &new_ie) {
		Poco::JSON::Object ie_data;
		ie_data.set("current_channel", (uint64_t) data[0] );
		new_ie.set("data", ie_data);
		new_ie.set("name", "DS_Parameter_Set");
		new_ie.set("type", WLAN_EID_DS_PARAMS);
	}

	inline void WFS_WLAN_EID_TIM(const std::vector<unsigned char> &data, Poco::JSON::Object &new_ie) {
		Poco::JSON::Object ie_data;
		ie_data.set("DTIM_count", (uint64_t) data[0] );
		ie_data.set("DTIM_period", (uint64_t) data[1] );
		new_ie.set("data", ie_data);
		new_ie.set("name", "Traffic_Indication_Map");
		new_ie.set("type", WLAN_EID_TIM);
	}

	inline void WFS_WLAN_EID_QBSS_LOAD(const std::vector<unsigned char> &data, Poco::JSON::Object &new_ie) {
		Poco::JSON::Object ie_data;
		ie_data.set("version", (uint64_t )(data[0]*256 + data[1]) );
		new_ie.set("data", ie_data);
		new_ie.set("name", "QBSS_Load");
		new_ie.set("type", WLAN_EID_QBSS_LOAD);
	}

	inline bool ParseWifiScan(Poco::JSON::Object::Ptr &Obj, std::stringstream &Result) {
		std::cout << "Start of parsing wifi" << std::endl;
		if (Obj->has("status")) {
			auto Status = Obj->get("status").extract<Poco::JSON::Object::Ptr>();
			if (Status->has("scan") && Status->isArray("scan")) {
				auto ScanArray = Status->getArray("scan");
				Poco::JSON::Array ParsedScan;
				for (auto &scan_entry : *ScanArray) {
					auto Entry = scan_entry.extract<Poco::JSON::Object::Ptr>();
					if (Entry->has("ies") && Entry->isArray("ies")) {
						auto ies = Entry->getArray("ies");
						Poco::JSON::Array new_ies;
						for (auto &ie : *ies) {
							auto ie_obj = ie.extract<Poco::JSON::Object::Ptr>();
							try {
								if (ie_obj->has("type") && ie_obj->has("data")) {
									std::ostringstream ofs;
									ie_obj->stringify(ofs);
									std::cout << ">>>" << ofs.str() << std::endl;
									auto ie_type = (uint64_t)ie_obj->get("type");
									auto ie_data = ie_obj->get("data").toString();
									std::cout << "TYPE:" << ie_type << "  DATA:" << ie_data
											  << std::endl;
									auto data = Base64Decode2Vec(ie_data);
									Poco::JSON::Object new_ie;
									if (ie_type == ieee80211_eid::WLAN_EID_COUNTRY) {
										WFS_WLAN_EID_COUNTRY(data, new_ie);
										new_ies.add(new_ie);
/*									} else if (ie_type == ieee80211_eid::WLAN_EID_EXT_SUPP_RATES) {
										WFS_WLAN_EID_EXT_SUPP_RATES(data, new_ie);
										new_ies.add(new_ie);
									} else if (ie_type == ieee80211_eid::WLAN_EID_FH_PARAMS) {
										WFS_WLAN_EID_FH_PARAMS(data, new_ie);
										new_ies.add(new_ie);
									} else if (ie_type == ieee80211_eid::WLAN_EID_DS_PARAMS) {
										WFS_WLAN_EID_DS_PARAMS(data, new_ie);
										new_ies.add(new_ie);
									} else if (ie_type == ieee80211_eid::WLAN_EID_TIM) {
										WFS_WLAN_EID_TIM(data, new_ie);
										new_ies.add(new_ie);
									} else if (ie_type == ieee80211_eid::WLAN_EID_QBSS_LOAD) {
										WFS_WLAN_EID_QBSS_LOAD(data, new_ie);
										new_ies.add(new_ie);
									*/ } else
									{
										new_ies.add(ie_obj);
									}
								} else {
									new_ies.add(ie_obj);
								}
							} catch (...) {
								new_ies.add(ie_obj);
							}
						}
						Entry->set("ies", new_ies);
						ParsedScan.add(Entry);
					} else {
						ParsedScan.add(scan_entry);
					}
				}
				Status->remove("scan");
				Status->set("scan", ParsedScan);
				Obj->remove("status");
				Obj->set("status", Status);
			}
		}
		std::cout << "End of parsing wifi - 1 " << std::endl;
		Obj->stringify(Result);
		std::cout << "End of parsing wifi - 2 " << std::endl;
		return false;
	}

} // namespace OpenWifi

