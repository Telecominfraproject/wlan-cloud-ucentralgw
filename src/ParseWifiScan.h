//
// Created by stephane bourque on 2022-05-25.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {

#define OUI_ENCAP_ETHER     0x000000    /* encapsulated Ethernet */
#define OUI_XEROX           0x000006    /* Xerox */
#define OUI_CISCO           0x00000C    /* Cisco (future use) */
#define OUI_IANA            0x00005E    /* the IANA */
#define OUI_NORTEL          0x000081    /* Nortel SONMP */
#define OUI_CISCO_90        0x0000F8    /* Cisco (IOS 9.0 and above?) */
#define OUI_CISCO_2         0x000142    /* Cisco */
#define OUI_CISCO_3         0x000143    /* Cisco */
#define OUI_FORCE10         0x0001E8    /* Force10 */
#define OUI_ERICSSON        0x0001EC    /* Ericsson Group */
#define OUI_CATENA          0x00025A    /* Catena Networks */
#define OUI_ATHEROS         0x00037F    /* Atheros Communications */
#define OUI_ORACLE          0x0003BA    /* Oracle */
#define OUI_AVAYA_EXTREME   0x00040D    /* Avaya Extreme access point */
#define OUI_EXTREME_MESH    0x000512    /* Extreme MESH */
#define OUI_SONY_ERICSSON   0x000AD9    /* Sony Ericsson Mobile Communications AB */
#define OUI_ARUBA           0x000B86    /* Aruba Networks */
#define OUI_ROUTERBOARD     0x000C42    /* Formerly listed as Mikrotik, however this OUI is owned by Routerboard*/
#define OUI_MERU            0x000CE6    /* Meru Network (Fortinet) */
#define OUI_SONY_ERICSSON_2 0x000E07    /* Sony Ericsson Mobile Communications AB */
#define OUI_PROFINET        0x000ECF    /* PROFIBUS Nutzerorganisation e.V. */
#define OUI_RSN             0x000FAC    /* Wi-Fi : RSN */
#define OUI_SONY_ERICSSON_3 0x000FDE    /* Sony Ericsson Mobile Communications AB */
#define OUI_FORTINET        0x00090F    /* Fortinet */
#define OUI_CIMETRICS       0x001090    /* Cimetrics, Inc. */
#define OUI_IEEE_802_3      0x00120F    /* IEEE 802.3 */
#define OUI_MEDIA_ENDPOINT  0x0012BB    /* Media (TIA TR-41 Committee) */
#define OUI_SONY_ERICSSON_4 0x0012EE    /* Sony Ericsson Mobile Communications AB */
#define OUI_ERICSSON_MOBILE 0x0015E0    /* Ericsson Mobile Platforms */
#define OUI_SONY_ERICSSON_5 0x001620    /* Sony Ericsson Mobile Communications AB */
#define OUI_SONY_ERICSSON_6 0x0016B8    /* Sony Ericsson Mobile Communications AB */
#define OUI_APPLE_AWDL      0x0017F2    /* Apple AWDL */
#define OUI_SONY_ERICSSON_7 0x001813    /* Sony Ericsson Mobile Communications AB */
#define OUI_BLUETOOTH       0x001958    /* Bluetooth SIG */
#define OUI_SONY_ERICSSON_8 0x001963    /* Sony Ericsson Mobile Communications AB */
#define OUI_AEROHIVE        0x001977    /* Aerohive AP to AP communication */
#define OUI_ITU_T           0x0019a7    /* International Telecommunication Union (ITU) Telecommunication Standardization Sector */
#define OUI_DCBX            0x001B21    /* Data Center Bridging Capabilities Exchange Protocol */
#define OUI_CISCO_UBI       0x001B67    /* Cisco/Ubiquisys */
#define OUI_IEEE_802_1QBG   0x001B3F    /* IEEE 802.1 Qbg */
#define OUI_NINTENDO        0x001F32
#define OUI_TURBOCELL       0x0020F6    /* KarlNet, who brought you Turbocell */
#define OUI_AVAYA           0x00400D    /* Avaya */
#define OUI_CISCOWL         0x004096    /* Cisco Wireless (Aironet) */
#define OUI_MARVELL         0x005043    /* Marvell Semiconductor */
#define OUI_WPAWME          0x0050F2    /* Wi-Fi : WPA / WME */
#define OUI_ERICSSON_2      0x008037    /* Ericsson Group */
#define OUI_HP_2            0x00805F    /* Hewlett-Packard */
#define OUI_IEEE_802_1      0x0080C2    /* IEEE 802.1 Committee */
#define OUI_PRE11N          0x00904C    /* Wi-Fi : 802.11 Pre-N */
#define OUI_ATM_FORUM       0x00A03E    /* ATM Forum */
#define OUI_ZEBRA_EXTREME   0x00A0F8    /* Extreme/WING (Zebra) */
#define OUI_EXTREME         0x00E02B    /* Extreme EDP/ESRP */
#define OUI_CABLE_BPDU      0x00E02F    /* DOCSIS spanning tree BPDU */
#define OUI_FOUNDRY         0x00E052    /* Foundry */
#define OUI_SIEMENS         0x080006    /* Siemens AG */
#define OUI_APPLE_ATALK     0x080007    /* Appletalk */
#define OUI_HP              0x080009    /* Hewlett-Packard */
#define OUI_CERN            0x080030    /* CERN, The European Organization for Nuclear Research */
#define OUI_IEEE_C37_238    0x1C129D    /* IEEE PES PSRC/SUB Working Group H7/Sub C7 (IEEE PC37.238) */
#define OUI_HYTEC_GER       0x30B216    /* Hytec Geraetebau GmbH */
#define OUI_ZIGBEE          0x4A191B    /* ZigBee Alliance */
#define OUI_WFA             0x506F9A    /* Wi-Fi Alliance */
#define OUI_MIST            0x5C5B35    /* Mist Systems */
#define OUI_RUCKUS          0x001392    /* Ruckus Networks */
#define OUI_SMPTE           0x6897E8    /* Society of Motion Picture and Television Engineers */
#define OUI_SGDSN           0x6A5C35    /* Secrétariat Général de la Défense et de la Sécurité Nationale http://www.sgdsn.gouv.fr */
#define OUI_ONOS            0xA42305    /* Open Networking Laboratory (ONOS) */
#define OUI_3GPP2           0xCF0002    /* 3GPP2 */
#define OUI_AVAYA_EXTREME2  0xD88466    /* Avaya Extreme Fabric */

/*
* COMPOSE_FRAME_TYPE() values for management frames.
 */
#define MGT_ASSOC_REQ          0x00  /* association request        */
#define MGT_ASSOC_RESP         0x01  /* association response       */
#define MGT_REASSOC_REQ        0x02  /* reassociation request      */
#define MGT_REASSOC_RESP       0x03  /* reassociation response     */
#define MGT_PROBE_REQ          0x04  /* Probe request              */
#define MGT_PROBE_RESP         0x05  /* Probe response             */
#define MGT_MEASUREMENT_PILOT  0x06  /* Measurement Pilot          */
#define MGT_BEACON             0x08  /* Beacon frame               */
#define MGT_ATIM               0x09  /* ATIM                       */
#define MGT_DISASS             0x0A  /* Disassociation             */
#define MGT_AUTHENTICATION     0x0B  /* Authentication             */
#define MGT_DEAUTHENTICATION   0x0C  /* Deauthentication           */
#define MGT_ACTION             0x0D  /* Action                     */
#define MGT_ACTION_NO_ACK      0x0E  /* Action No Ack              */
#define MGT_ARUBA_WLAN         0x0F  /* Aruba WLAN Specific        */

/* WFA vendor specific subtypes */
#define WFA_SUBTYPE_P2P 9
#define WFA_SUBTYPE_WIFI_DISPLAY 10
#define WFA_SUBTYPE_HS20_INDICATION 16
#define WFA_SUBTYPE_HS20_ANQP 17

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

	static const value_string ieee80211_supported_rates_vals = {
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
		{ 0xFF, "BSS requires support for mandatory features of HT PHY (IEEE 802.11 - Clause 20)" },
		{ 0,    NULL}
	};

	static const value_string environment_vals = {
		{ 0x20, "Any" },
		{ 0x4f, "Outdoor" },
		{ 0x49, "Indoor" },
		{ 0,    NULL }
	};

	static const value_string service_interval_granularity_vals = {
		{ 0, "5 ms" },
		{ 1, "10 ms" },
		{ 2, "15 ms" },
		{ 3, "20 ms" },
		{ 4, "25 ms" },
		{ 5, "30 ms" },
		{ 6, "35 ms" },
		{ 7, "40 ms" },
		{ 0x00, NULL }
	};

	static const value_string ieee80211_rsn_cipher_vals = {
		{0, "NONE"},
		{1, "WEP (40-bit)"},
		{2, "TKIP"},
		{3, "AES (OCB)"},
		{4, "AES (CCM)"},
		{5, "WEP (104-bit)"},
		{6, "BIP"},
		{7, "Group addressed traffic not allowed"},
		{0, NULL}
	};

	static const value_string ieee80211_rsn_keymgmt_vals = {
		{0, "NONE"},
		{1, "WPA"},
		{2, "PSK"},
		{3, "FT over IEEE 802.1X"},

		{4, "FT using PSK"},
		{5, "WPA (SHA256)"},
		{6, "PSK (SHA256)"},
		{7, "TDLS / TPK Handshake"},
		{0, NULL}
	};

	static const value_string rsn_cap_replay_counter = {
		{0x00, "1 replay counter per PTKSA/GTKSA/STAKeySA"},
		{0x01, "2 replay counters per PTKSA/GTKSA/STAKeySA"},
		{0x02, "4 replay counters per PTKSA/GTKSA/STAKeySA"},
		{0x03, "16 replay counters per PTKSA/GTKSA/STAKeySA"},
		{0, NULL}
	};

	static const value_string ieee802111_wfa_ie_type_vals = {
		{ 1, "WPA Information Element" },
		{ 2, "WMM/WME" },
		{ 4, "WPS" },
		{ 0, NULL }
	};

	static const value_string ieee80211_wfa_ie_wpa_cipher_vals = {
		{ 0, "NONE" },
		{ 1, "WEP (40-bit)" },
		{ 2, "TKIP" },
		{ 3, "AES (OCB)" },
		{ 4, "AES (CCM)" },
		{ 5, "WEP (104-bit)" },
		{ 6, "BIP" },
		{ 7, "Group addressed traffic not allowed" },
		{ 0, NULL }
	};

	static const value_string ieee80211_wfa_ie_wpa_keymgmt_vals = {
		{ 0, "NONE" },
		{ 1, "WPA" },
		{ 2, "PSK" },
		{ 3, "FT over IEEE 802.1X" },
		{ 4, "FT using PSK" },
		{ 5, "WPA (SHA256)" },
		{ 6, "PSK (SHA256)" },
		{ 7, "TDLS / TPK Handshake" },
		{ 0, NULL }
	};

	static const value_string ieee802111_wfa_ie_wme_type = {
		{ 0, "Information Element" },
		{ 1, "Parameter Element" },
		{ 2, "TSPEC Element" },
		{ 0, NULL}
	};

	static const value_string ieee802111_wfa_ie_wme_qos_info_sta_max_sp_length_vals = {
		{ 0, "WMM AP may deliver all buffered frames (MSDUs and MMPDUs)" },
		{ 1, "WMM AP may deliver a maximum of 2 buffered frames (MSDUs and MMPDUs) per USP" },
		{ 2, "WMM AP may deliver a maximum of 4 buffered frames (MSDUs and MMPDUs) per USP" },
		{ 3, "WMM AP may deliver a maximum of 6 buffered frames (MSDUs and MMPDUs) per USP" },
		{ 0, NULL}
	};

	static const value_string ieee80211_wfa_ie_wme_acs_vals = {
		{ 0, "Best Effort" },
		{ 1, "Background" },
		{ 2, "Video" },
		{ 3, "Voice" },
		{ 0, NULL }
	};

	static const value_string ieee80211_wfa_ie_wme_tspec_tsinfo_direction_vals = {
		{ 0, "Uplink" },
		{ 1, "Downlink" },
		{ 2, "Direct link" },
		{ 3, "Bidirectional link" },
		{ 0, NULL }
	};

	static const value_string ieee80211_wfa_ie_wme_tspec_tsinfo_psb_vals = {
		{ 0, "Legacy" },
		{ 1, "U-APSD" },
		{ 0, NULL }
	};

	static const value_string ieee80211_wfa_ie_wme_tspec_tsinfo_up_vals = {
		{ 0, "Best Effort" },
		{ 1, "Background" },
		{ 2, "Spare" },
		{ 3, "Excellent Effort" },
		{ 4, "Controlled Load" },
		{ 5, "Video" },
		{ 6, "Voice" },
		{ 7, "Network Control" },
		{ 0, NULL }
	};

	static const value_string hta_ext_chan_offset_flag = {
		{0x00, "No Extension Channel"},
		{0x01, "Extension Channel above control channel"},
		{0x02, "Undefined"},
		{0x03, "Extension Channel below control channel"},
		{0x00, NULL}
	};

	static const value_string hta_service_interval_flag = {
		{0x00, "5ms"},
		{0x01, "10ms"},
		{0x02, "15ms"},
		{0x03, "20ms"},
		{0x04, "25ms"},
		{0x05, "30ms"},
		{0x06, "35ms"},
		{0x07, "40ms"},
		{0x00, NULL}
	};

	static const value_string hta_operating_mode_flag = {
		{0x00, "Pure HT, no protection"},
		{0x01, "There may be non-HT devices (control & ext channel)"},
		{0x02, "No non-HT is associated, but at least 1 20MHz is. protect on"},
		{0x03, "Mixed: no non-HT is associated, protect on"},
		{0x00, NULL}
	};

	static const value_string wfa_subtype_vals = {
		{ WFA_SUBTYPE_P2P, "P2P" },
		{ WFA_SUBTYPE_HS20_INDICATION, "Hotspot 2.0 Indication" },
		{ WFA_SUBTYPE_HS20_ANQP, "Hotspot 2.0 ANQP" },
		{ 0, NULL }
	};

	const char * VALS(const value_string &vals, uint v) {
		for(const auto &e:vals) {
			if(e.first==v && e.second!=NULL)
				return e.second;
		}
		return "unknown";
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

	std::string BufferToHex(const unsigned char *b,uint size, char separator=' ') {
		static const char hex[] = "0123456789abcdef";
		std::string result;
		while(size) {
			if (!result.empty())
				result += separator;
			result += (hex[(*b & 0xf0) >> 4]);
			result += (hex[(*b & 0x0f)]);
			b++;
			size--;
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

	uint16_t GetUInt16(const unsigned char *d,uint & offset) {
		uint16_t value = d[offset] + d[offset+1]*256;
		offset +=2;
		return value;
	}

	uint32_t GetUInt32(const unsigned char *d,uint & offset) {
		uint32_t value = d[offset+0]+d[offset+1]*256 + d[offset+2]*256*256 + d[offset+3]*256*256*256;
		offset +=4;
		return value;
	}

	uint32_t GetUInt32Big(const unsigned char *d,uint & offset) {
		uint32_t value = d[offset+3]+d[offset+2]*256 + d[offset+1]*256*256 + d[offset+0]*256*256*256;
		offset +=4;
		return value;
	}

	uint32_t GetUInt24Big(const unsigned char *d,uint & offset) {
		uint32_t value = d[offset+2] + d[offset+1]*256 + d[offset+0]*256*256;
		offset +=3;
		return value;
	}

	uint32_t GetUInt24(const unsigned char *d,uint & offset) {
		uint32_t value = d[offset+0] + d[offset+1]*256 + d[offset+2]*256*256;
		offset +=3;
		return value;
	}

	// 0x01
	inline nlohmann::json WFS_WLAN_EID_SUPP_RATES(const std::vector<unsigned char> &data) {
		nlohmann::json 	Rates = nlohmann::json::array();
		nlohmann::json 	new_ie;
		for(const auto &c:data) {
			nlohmann::json 	Rate;
			Rates.push_back(VALS(ieee80211_supported_rates_vals,c));
		}
		new_ie["name"]="Supported Rates (Mbps)";
		new_ie["content"]=Rates;
		new_ie["type"]=WLAN_EID_SUPP_RATES;
		return new_ie;
	}

	// 0x02
	inline nlohmann::json WFS_WLAN_EID_FH_PARAMS(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		content["Dwell Time"] = (uint64_t) (data[0] * 256 + data[1]);
		content["Hop Set"] = (uint)data[2];
		content["Hop Pattern"] = (uint)data[3];
		content["Hop Index"] = (uint)data[4];
		new_ie["name"]="FH Params";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_FH_PARAMS;
		return new_ie;
	}

	// 0x03
	inline nlohmann::json WFS_WLAN_EID_DS_PARAMS(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		content["current_channel"] = (uint64_t) data[0];
		new_ie["name"]="DS Params";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_DS_PARAMS;
		return new_ie;
	}

	// 0x05
	inline nlohmann::json WFS_WLAN_EID_TIM(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		if(data.size()>=4) {
			uint offset=0;
			content["DTIM count"] = (uint64_t)data[offset++];
			content["DTIM period"] = (uint64_t)data[offset++];
			content["Bitmap control"]["Multicast"] = (uint)data[offset] & 0x01;
			content["Bitmap control"]["Bitmap Offset"] = (uint)(data[offset] & 0xFe) >> 1;
			offset++;
			if(offset<data.size()) {
				content["Bitmap control"]["Partial Virtual Bitmap"] = BufferToHex( &data[offset], data.size()-offset);
			}
		}
		new_ie["name"]="Traffic Indication Map (TIM)";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_TIM;
		return new_ie;
	}

	// 0x07
	inline nlohmann::json WFS_WLAN_EID_COUNTRY(const std::vector<unsigned char> &data) {
		nlohmann::json new_ie;
		std::string CountryName;
		CountryName += (char)data[0];
		CountryName += (char)data[1];
		nlohmann::json content;

		content["Code"] = CountryName;
		content["Environment"] = VALS(environment_vals,data[2]);
		nlohmann::json ie_data;
		nlohmann::json constraints = nlohmann::json::array();
		for (std::size_t i = 3; (i+3)<= data.size(); i += 3) {
			nlohmann::json constraint;
			if(data[i]<=200) {
				constraint["Country Info"]["First Channel Number"] = (uint64_t)data[i+0];
				constraint["Country Info"]["Number of Channels"] = (uint64_t)data[i+1];
				constraint["Country Info"]["Maximum Transmit Power Level (in dBm)"] = (uint64_t)data[i+2];
			} else {
				constraint["Country Info"]["Regulatory Extension Identifier"] = (uint64_t)data[i+0];
				constraint["Country Info"]["Regulatory Class"] = (uint64_t)data[i+1];
				constraint["Country Info"]["Coverage Class"] = (uint64_t)data[i+2];
			}
			constraints.push_back(constraint);
		}
		content["constraints"] = constraints;
		new_ie["content"] = content;
		new_ie["name"] = "country";
		new_ie["type"] = WLAN_EID_COUNTRY;
		return new_ie;
	}

	//	11
	inline nlohmann::json WFS_WLAN_EID_QBSS_LOAD(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		if(data.size()==4) {
			content["Cisco QBSS Version 1 - non CCA"]["QBSS Version"] = 1;
			content["Cisco QBSS Version 1 - non CCA"]["Station Count"] = (uint)( data[2] + data[1]*256);
			content["Cisco QBSS Version 1 - non CCA"]["Channel Utilization"] = (uint) data[3];
			content["Cisco QBSS Version 1 - non CCA"]["Available Admission Capabilities"] = (uint) data[4];
		} else if(data.size()==5) {
			content["802.11e CCA Version"]["QBSS Version"] = 2;
			content["802.11e CCA Version"]["Station Count"] = (uint)( data[2] + data[1]*256);
			content["802.11e CCA Version"]["Channel Utilization"] = (uint) data[3];
			content["802.11e CCA Version"]["Available Admission Capabilities"] = (uint) data[4] + data[5]*256;
		}
		new_ie["name"]="QBSS Load";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_QBSS_LOAD;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_PWR_CONSTRAINT(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		content["Local Power Constraint"] = (uint) data[0];
		new_ie["name"]="Local Power Constraint";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_PWR_CONSTRAINT;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_ERP_INFO(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		content["Non ERP Present"] = bitSet(data[0],0);
		content["Use Protection"] = bitSet(data[0],1);
		content["Barker Preamble Mode"] = bitSet(data[0],2);
		new_ie["name"]="ERP Information";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_ERP_INFO;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_TPC_REPORT(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		if(data.size()==2) {
			content["Transmit Power"] = (uint) data[0];
			content["Link Margin"] = (uint) data[1];
		}
		new_ie["name"]="TPC Report";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_TPC_REPORT;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_SUPPORTED_REGULATORY_CLASSES(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		if(data.size()>=2) {
			content["Current Regulatory Class"]= (uint) data[0];
			std::string alternates;
			for(uint i=1;i<data.size();++i) {
				if(!alternates.empty())
					alternates += ", ";
				alternates += std::to_string((uint)data[i]);
			}
			content["Alternate Regulatory Classes"] = alternates;
		}
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

	void dissect_ht_capability_ie(const unsigned char *data,uint size, nlohmann::json & content) {
		if(size==26) {
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

	}

	void dissect_ht_info_ie_1_0(const unsigned char *data, uint size, nlohmann::json & content)
	{
		if (size != 22) {
			return;
		}
		uint offset=0;

		content["HT Control Channel"] = data[offset++];
		auto htcaps = data[offset++];
		content["HT Additional Capabilities"]["Extension Channel Offset"] = VALS(hta_ext_chan_offset_flag, htcaps & 0x0003);
		content["HT Additional Capabilities"]["Recommended Tx Channel Width"] = (htcaps & 0x0003) >> 2;
		content["HT Additional Capabilities"]["Reduced Interframe Spacing (RIFS) Mode"] = (htcaps & 0x0008) >> 3;
		content["HT Additional Capabilities"]["Controlled Access Only"] = (htcaps & 0x0010) >> 4;
		content["HT Additional Capabilities"]["Service Interval Granularity"] = VALS(hta_service_interval_flag, (htcaps & 0x00e0) >> 5);

		htcaps = GetUInt16(data,offset);
		content["HT Additional Capabilities"]["Operating Mode"] = VALS(hta_operating_mode_flag, (htcaps & 0x0003) >> 0);
		content["HT Additional Capabilities"]["Non Greenfield (GF) devices Present"] = (htcaps & 0x0004) >> 2;

		htcaps = GetUInt16(data,offset);
		content["HT Additional Capabilities"]["Basic STB Modulation and Coding Scheme (MCS)"] = (htcaps & 0x007f) >> 0;
		content["HT Additional Capabilities"]["Dual Clear To Send (CTS) Protection"] = (htcaps & 0x0080) >> 7;
		content["HT Additional Capabilities"]["Secondary Beacon"] = (htcaps & 0x0100) >> 8;
		content["HT Additional Capabilities"]["L-SIG TXOP Protection Support"] = (htcaps & 0x0200) >> 9;
		content["HT Additional Capabilities"]["Phased Coexistence Operation (PCO) Active"] = (htcaps & 0x0400) >> 10;
		content["HT Additional Capabilities"]["Phased Coexistence Operation (PCO) Phase"] = (htcaps & 0x0800) >> 11;
		ParseMCSset(&data[offset],content);
	}

	inline nlohmann::json WFS_WLAN_EID_HT_CAPABILITY(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;

		if(data.size()==26) {
			dissect_ht_capability_ie(&data[0],data.size(),content);
		}

		new_ie["name"]="HT Capabilities";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_HT_CAPABILITY;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_EXT_SUPP_RATES(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;
		std::string Rates;

		for(const auto &rate:data) {
			if(!Rates.empty())
				Rates += ", ";
			Rates += VALS(ieee80211_supported_rates_vals,rate);
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

	inline nlohmann::json WFS_WLAN_EID_RRM_ENABLED_CAPABILITIES(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;

		if(data.size()==5) {
			uint offset=0;
			uint caps = data[offset++];
			content["RM Capabilities"]["Link Measurement"] = (caps & 0x00000001) >> 0;
			content["RM Capabilities"]["Neighbor Report"] = (caps & 0x00000002) >> 1;
			content["RM Capabilities"]["Parallel Measurements"] = (caps & 0x00000004) >> 2;
			content["RM Capabilities"]["Repeated Measurements"] = (caps & 0x00000008) >> 3;
			content["RM Capabilities"]["Beacon Passive Measurement"] = (caps & 0x00000010) >> 4;
			content["RM Capabilities"]["Beacon Active Measurement"] = (caps & 0x00000020) >> 5;
			content["RM Capabilities"]["Beacon Table Measurement"] = (caps & 0x00000040) >> 6;
			content["RM Capabilities"]["Beacon Measurement Reporting Conditions"] = (caps & 0x00000080) >> 7;

			caps = data[offset++];
			content["RM Capabilities"]["Frame Measurement"] = (caps & 0x00000001) >> 0;
			content["RM Capabilities"]["Channel Load Measurement"] = (caps & 0x00000002) >> 1;
			content["RM Capabilities"]["Noise Histogram Measurement"] = (caps & 0x00000004) >> 2;
			content["RM Capabilities"]["Statistics Measurement"] = (caps & 0x00000008) >> 3;
			content["RM Capabilities"]["LCI Measurement"] = (caps & 0x00000010) >> 4;
			content["RM Capabilities"]["LCI Azimuth capability"] = (caps & 0x00000020) >> 5;
			content["RM Capabilities"]["Transmit Stream/Category Measurement"] = (caps & 0x00000040) >> 6;
			content["RM Capabilities"]["Triggered Transmit Stream/Category Measurement"] = (caps & 0x00000080) >> 7;

			caps = data[offset++];
			content["RM Capabilities"]["AP Channel Report capability"] = (caps & 0x00000007) >> 0;
			content["RM Capabilities"]["RM MIB capability"] = (caps & 0x00000002) >> 1;
			content["RM Capabilities"]["Operating Channel Max Measurement Duration"] = (caps & 0x0000001c) >> 2;
			content["RM Capabilities"]["Nonoperating Channel Max Measurement Duration"] = (caps & 0x000000e0) >> 5;

			caps = data[offset++];
			content["RM Capabilities"]["Measurement Pilotcapability"] = (caps & 0x00000007) >> 0;
			content["RM Capabilities"]["RM MIB capability"] = (caps & 0x00000008) >> 3;
			content["RM Capabilities"]["Neighbor Report TSF Offset"] = (caps & 0x00000010) >> 4;
			content["RM Capabilities"]["RCPI Measurement capability"] = (caps & 0x00000020) >> 5;
			content["RM Capabilities"]["RSNI Measurement capability"] = (caps & 0x00000040) >> 6;
			content["RM Capabilities"]["BSS Average Access Delay capability"] = (caps & 0x00000080) >> 7;

			caps = data[offset];
			content["RM Capabilities"]["BSS Available Admission Capacity capability"] = (caps & 0x00000001) >> 0;
			content["RM Capabilities"]["Antenna capability"] = (caps & 0x00000002) >> 1;
		}

		new_ie["name"]="RM Capabilities";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_RRM_ENABLED_CAPABILITIES;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_EXT_CAPABILITY(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;

		if(data.size()>=1) {
			uint offset=0;
			content["Extended Capabilities"]["20/40 BSS Coexistence Management Support"] = (data[offset] & 0x01) >> 0;
			content["Extended Capabilities"]["On-demand beacon"] = (data[offset] & 0x02) >> 1;
			content["Extended Capabilities"]["Extended Channel Switching"] = (data[offset] & 0x04) >> 2;
			content["Extended Capabilities"]["WAVE indication"] = (data[offset] & 0x08) >> 3;
			content["Extended Capabilities"]["PSMP Capability"] = (data[offset] & 0x10) >> 4;
			content["Extended Capabilities"]["Reserved"] = (data[offset] & 0x20) >> 5;
			content["Extended Capabilities"]["S-PSMP Support"] = (data[offset] & 0x40) >> 6;
			content["Extended Capabilities"]["Event"] = (data[offset] & 0x80) >> 7;

			offset++;
			if(offset<data.size()) {
				content["Extended Capabilities"]["Diagnostics"] = (data[offset] & 0x01) >> 0;
				content["Extended Capabilities"]["Multicast Diagnostics"] = (data[offset] & 0x02) >> 1;
				content["Extended Capabilities"]["Location Tracking"] = (data[offset] & 0x04) >> 2;
				content["Extended Capabilities"]["FMS"] = (data[offset] & 0x08) >> 3;
				content["Extended Capabilities"]["Proxy ARP Service"] = (data[offset] & 0x10) >> 4;
				content["Extended Capabilities"]["Collocated Interference Reporting"] = (data[offset] & 0x20) >> 5;
				content["Extended Capabilities"]["Civic Location"] = (data[offset] & 0x40) >> 6;
				content["Extended Capabilities"]["Geospatial Location"] = (data[offset] & 0x80) >> 7;
			}

			offset++;
			if(offset<data.size()) {
				content["Extended Capabilities"]["TFS"] = (data[offset] & 0x01) >> 0;
				content["Extended Capabilities"]["WNM-Sleep Mode"] = (data[offset] & 0x02) >> 1;
				content["Extended Capabilities"]["TIM Broadcast"] = (data[offset] & 0x04) >> 2;
				content["Extended Capabilities"]["BSS Transition"] = (data[offset] & 0x08) >> 3;
				content["Extended Capabilities"]["QoS Traffic Capability"] = (data[offset] & 0x10) >> 4;
				content["Extended Capabilities"]["AC Station Count"] = (data[offset] & 0x20) >> 5;
				content["Extended Capabilities"]["Multiple BSSID"] = (data[offset] & 0x40) >> 6;
				content["Extended Capabilities"]["Timing Measurement"] = (data[offset] & 0x80) >> 7;
			}

			offset++;
			if(offset<data.size()) {
				content["Extended Capabilities"]["Channel Usage"] = (data[offset] & 0x01) >> 0;
				content["Extended Capabilities"]["SSID List"] = (data[offset] & 0x02) >> 1;
				content["Extended Capabilities"]["DMS"] = (data[offset] & 0x04) >> 2;
				content["Extended Capabilities"]["UTC TSF Offset"] = (data[offset] & 0x08) >> 3;
				content["Extended Capabilities"]["Peer U-APSD Buffer STA Support"] = (data[offset] & 0x10) >> 4;
				content["Extended Capabilities"]["TDLS Peer PSM Support"] = (data[offset] & 0x20) >> 5;
				content["Extended Capabilities"]["TDLS channel switching"] = (data[offset] & 0x40) >> 6;
				content["Extended Capabilities"]["Interworking"] = (data[offset] & 0x80) >> 7;
			}

			offset++;
			if(offset<data.size()) {
				content["Extended Capabilities"]["QoS Map"] = (data[offset] & 0x01) >> 0;
				content["Extended Capabilities"]["EBR"] = (data[offset] & 0x02) >> 1;
				content["Extended Capabilities"]["SSPN Interface"] = (data[offset] & 0x04) >> 2;
				content["Extended Capabilities"]["Reserved"] = (data[offset] & 0x08) >> 3;
				content["Extended Capabilities"]["MSGCF Capability"] = (data[offset] & 0x10) >> 4;
				content["Extended Capabilities"]["TDLS support"] = (data[offset] & 0x20) >> 5;
				content["Extended Capabilities"]["TDLS Prohibited"] = (data[offset] & 0x40) >> 6;
				content["Extended Capabilities"]["TDLS Channel Switching Prohibited"] = (data[offset] & 0x80) >> 7;
			}

			offset++;
			if(offset<data.size()) {
				content["Extended Capabilities"]["Reject Unadmitted Frame"] = (data[offset] & 0x01) >> 0;
				content["Extended Capabilities"]["Service Interval Granularity"] = VALS(service_interval_granularity_vals,(data[offset] & 0x0e) >> 1);
				content["Extended Capabilities"]["Identifier Location"] = (data[offset] & 0x10) >> 4;
				content["Extended Capabilities"]["U-APSD Coexistence"] = (data[offset] & 0x20) >> 5;
				content["Extended Capabilities"]["WNM-Notification"] = (data[offset] & 0x40) >> 6;
				content["Extended Capabilities"]["Reserved"] = (data[offset] & 0x80) >> 7;
			}

			offset++;
			if(offset<data.size()) {
				content["Extended Capabilities"]["UTF-8 SSID"] = (data[offset] & 0x01) >> 0;
			}
		}

		new_ie["name"]="Extended Capabilities";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_EXT_CAPABILITY;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_RSN(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;

		// 01 00 00 0f ac 04 01 00 00 0f ac 04 02 00 00 0f ac 02 00 0f ac 06 8c 00

		uint offset = 0 ;
		content["RSN Version"] = GetUInt16(&data[0],offset);
		auto RSNOUI = GetUInt24Big(&data[0],offset);
		content["Group Cipher Suite OUI"] = BufferToHex(&data[offset-3],3,':');
		if(RSNOUI==OUI_RSN) {
			content["Group Cipher Suite type"] = VALS(ieee80211_rsn_cipher_vals,data[offset++]);
		} else {
			content["Group Cipher Suite type"] = BufferToHex(&data[offset++],1);
		}
		if(offset<data.size()) {
			auto pcsc = GetUInt16(&data[0], offset);
			content["Pairwise Cipher Suite Count"] = pcsc;
			if (offset + pcsc * 4 <= data.size()) {
				nlohmann::json suites = nlohmann::json::array();
				while (pcsc) {
					nlohmann::json entry;
					RSNOUI = GetUInt24Big(&data[0], offset);
					entry["Group Cipher Suite OUI"] = BufferToHex(&data[offset - 3], 3, ':');
					if (RSNOUI == OUI_RSN) {
						entry["Group Cipher Suite type"] =
							VALS(ieee80211_rsn_cipher_vals, data[offset++]);
					} else {
						entry["Group Cipher Suite type"] = BufferToHex(&data[offset++], 1);
					}
					suites.push_back(entry);
					pcsc--;
				}
				content["Pairwise Cipher Suite List"] = suites;
			}
		}

		if(offset<data.size()) {
			auto akms_count = GetUInt16(&data[0],offset);
			content["Auth Key Management (AKM) Suite Count"] = akms_count;
			if(offset+akms_count*4<=data.size()) {
				nlohmann::json suites=nlohmann::json::array();
				while(akms_count) {
					nlohmann::json entry;
					RSNOUI = GetUInt24Big(&data[0],offset);
					entry["Auth Key Management (AKM) OUI"] = BufferToHex(&data[offset-3],3,':');
					if(RSNOUI==OUI_RSN) {
						entry["Auth Key Management (AKM) type"] = VALS(ieee80211_rsn_keymgmt_vals,data[offset++]);
					} else {
						entry["Auth Key Management (AKM) type"] = BufferToHex(&data[offset++],1);
					}
					suites.push_back(entry);
					akms_count--;
				}
				content["Auth Key Management (AKM) List"]=suites;
			}
		}

		if(offset+2<=data.size()) {
			auto rsn_cap = GetUInt16(&data[0], offset);
			content["RSN Capabilities"]["RSN Pre-Auth capabilities"] = (rsn_cap & 0x0001) >> 0;
			content["RSN Capabilities"]["RSN No Pairwise capabilities"] = (rsn_cap & 0x0002) >> 1;

			content["RSN Capabilities"]["RSN PTKSA Replay Counter capabilities"] =
				VALS(rsn_cap_replay_counter, (rsn_cap & 0x000c) >> 2);
			content["RSN Capabilities"]["RSN GTKSA Replay Counter capabilities"] =
				VALS(rsn_cap_replay_counter, (rsn_cap & 0x0030) >> 4);
			content["RSN Capabilities"]["Management Frame Protection Required"] =
				(rsn_cap & 0x0040) >> 6;
			content["RSN Capabilities"]["Management Frame Protection Capable"] =
				(rsn_cap & 0x0080) >> 7;
			content["RSN Capabilities"]["PeerKey Enabled"] = (rsn_cap & 0x0200) >> 9;
		}

		if(offset+2<=data.size()) {
			auto pmkid_count = GetUInt16(&data[0],offset);
			content["PMKID Count"] = pmkid_count;
			if(offset+pmkid_count*16<=data.size()) {
				nlohmann::json list=nlohmann::json::array();
				while(pmkid_count) {
					nlohmann::json entry;
					entry["PMKID"] = BufferToHex(&data[offset],16);
					list.push_back(entry);
					pmkid_count--;
					offset+=16;
				}
				content["PMKID List"]=list;
			}
		}

		if(offset+4<=data.size()) {
			RSNOUI = GetUInt24Big(&data[0],offset);
			content["Group Management Cipher Suite"]["Group Management Cipher Suite OUI"] = BufferToHex(&data[offset-3],3,':');
			if(RSNOUI==OUI_RSN) {
				content["Group Management Cipher Suite"]["Group Management Cipher Suite type"] = VALS(ieee80211_rsn_cipher_vals,data[offset++]);
			} else {
				content["Group Management Cipher Suite"]["Group Management Cipher Suite type"] = BufferToHex(&data[offset++],1);
			}
		}

		new_ie["name"]="RSN";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_RSN;
		return new_ie;
	}

	inline nlohmann::json
	dissect_qos_info(const unsigned char *b,uint l)
	{
		nlohmann::json content;

		if(l<1)
			return content;

		auto ftype = MGT_PROBE_REQ;

		switch (ftype) {
			case MGT_ASSOC_REQ:
			case MGT_PROBE_REQ:
			case MGT_REASSOC_REQ:
			{
				/* To AP so decode as per WMM standard Figure 7 QoS Info field when sent from WMM STA*/
				content["WME QoS Info"]["Max SP Length"] = VALS(ieee802111_wfa_ie_wme_qos_info_sta_max_sp_length_vals,(*b & 0x60) >> 5);
				content["WME QoS Info"]["AC_BE"] = (*b & 0x08) >> 3;
				content["WME QoS Info"]["AC_BK"] = (*b & 0x04) >> 2;
				content["WME QoS Info"]["AC_VI"] = (*b & 0x02) >> 1;
				content["WME QoS Info"]["AC_VO"] = (*b & 0x01) >> 0;
				break;
			}
			case MGT_BEACON:
			case MGT_PROBE_RESP:
			case MGT_ASSOC_RESP:
			case MGT_REASSOC_RESP:
			{
				/* From AP so decode as per WMM standard Figure 6 QoS Info field when sent from WMM AP */
				content["WME QoS Info"]["U-APSD"] = (*b & 0x80) >> 4;
				content["WME QoS Info"]["Parameter Set Count"] = (*b & 0x0f) >> 0;
				break;
			}
			default:
				content["WME QoS Info"]["invalid"] = true;
				break;
		}
		return content;
	}

	inline nlohmann::json dissect_vendor_ie_wpawme(const unsigned char *b, uint l) {
		nlohmann::json ie;
		uint offset=0;

		// ie["dump"] = BufferToHex(b,l);

		ie["vendor"] = "Wi-Fi : WPA / WME";
		auto type = b[0];
		ie["type"] = VALS(ieee802111_wfa_ie_type_vals,type);
		offset++;

		if(offset<l) {
			switch (type) {
			case 1: {
				ie["WPA Version"] = GetUInt16(b, offset);
				auto OUI = GetUInt24Big(b, offset);
				if (OUI == OUI_WPAWME)
					ie["Multicast Cipher Suite type"] =
						VALS(ieee80211_wfa_ie_wpa_cipher_vals, b[offset++]);
				else
					ie["Multicast Cipher Suite type"] = BufferToHex(&b[offset++], 1);
				auto ucs_count = GetUInt16(b, offset);
				ie["Unicast Cipher Suite Count"] = ucs_count;
				nlohmann::json list = nlohmann::json::array();
				while (ucs_count) {
					OUI = GetUInt24Big(b, offset);
					nlohmann::json entry;
					entry["Unicast Cipher Suite OUI"] = BufferToHex(&b[offset - 3], 3);
					if (OUI == OUI_WPAWME)
						entry["Unicast Cipher Suite type"] =
							VALS(ieee80211_wfa_ie_wpa_cipher_vals, b[offset++]);
					else
						entry["Unicast Cipher Suite type"] = BufferToHex(&b[offset++], 1);
					list.push_back(entry);
					ucs_count--;
				}
				ie["Unicast Cipher Suite List"] = list;

				auto akms_count = GetUInt16(b, offset);
				ie["Auth Key Management (AKM) Suite Count"] = akms_count;
				nlohmann::json list2 = nlohmann::json::array();
				while (akms_count) {
					OUI = GetUInt24Big(b, offset);
					nlohmann::json entry;
					entry["Auth Key Management (AKM) OUI"] = BufferToHex(&b[offset - 3], 3);
					if (OUI == OUI_WPAWME)
						entry["Auth Key Management (AKM) type"] =
							VALS(ieee80211_wfa_ie_wpa_keymgmt_vals, b[offset++]);
					else
						entry["Auth Key Management (AKM) type"] = BufferToHex(&b[offset++], 1);
					list2.push_back(entry);
					akms_count--;
				}
				ie["Unicast Cipher Suite List"] = list2;
			} break;
			case 2: {
				auto sub_type = b[offset++];
				ie["WME Subtype"] = VALS(ieee802111_wfa_ie_wme_type,sub_type);
				ie["WME Version"] = (uint) b[offset++];

				switch(sub_type) {
					case 0: {
							ie["WME QoS Info"] = dissect_qos_info(&b[offset],l-offset);
						}
						break;
					case 1: {
						ie["WME QoS Info"] = dissect_qos_info(&b[offset],l-offset);
						offset++;
						offset++;		// skip reserved...
						nlohmann::json list=nlohmann::json::array();
						for(uint i=0;i<4;i++) {
							nlohmann::json entry;
							entry["ACI"] = VALS(ieee80211_wfa_ie_wme_acs_vals, (b[offset] & 0x60)>>5);
							entry["Admission Control Mandatory"] = (b[offset] & 0x10) >> 4;
							entry["AIFSN"] = (b[offset] & 0x0f)>>5;
							offset++;
							entry["ECW Max"] = (b[offset] & 0xf0)>>4;
							entry["ECW Min"] = (b[offset] & 0x0f)>>0;
							offset++;
							entry["TXOP Limit"] = b[offset] + b[offset+1]*256;
							offset+=2;
							list.push_back(entry);
						}
						ie["Ac Parameters"]["ACI / AIFSN Field"] = list;
						}
						break;
					case 2: {
						auto tid = GetUInt24(b,offset);
							ie["TS Info"]["TID"] =  (tid & 0x00001E) >> 1;
							ie["TS Info"]["Direction"] = VALS(ieee80211_wfa_ie_wme_tspec_tsinfo_direction_vals,(tid & 0x000060) >> 5);
							ie["TS Info"]["PSB"] =  VALS(ieee80211_wfa_ie_wme_tspec_tsinfo_psb_vals, (tid & 0x000400) >> 10);
							ie["TS Info"]["UP"] =  VALS(ieee80211_wfa_ie_wme_tspec_tsinfo_up_vals,(tid & 0x003800) >> 11);
							ie["TS Info"]["Normal MSDU Size"] = GetUInt16(b,offset);
							ie["TS Info"]["Maximum MSDU Size"] = GetUInt16(b,offset);
							ie["TS Info"]["Minimum Service Interval"] = GetUInt32(b,offset);
							ie["TS Info"]["Maximum Service Interval"] = GetUInt32(b,offset);
							ie["TS Info"]["Inactivity Interval"] = GetUInt32(b,offset);
							ie["TS Info"]["Suspension Interval"] = GetUInt32(b,offset);
							ie["TS Info"]["Service Start Time"] = GetUInt32(b,offset);
							ie["TS Info"]["Minimum Data Rate"] = GetUInt32(b,offset);
							ie["TS Info"]["Mean Data Rate"] = GetUInt32(b,offset);
							ie["TS Info"]["Peak Data Rate"] = GetUInt32(b,offset);
							ie["TS Info"]["Burst Size"] = GetUInt32(b,offset);
							ie["TS Info"]["Delay Bound"] = GetUInt32(b,offset);
							ie["TS Info"]["Minimum PHY Rate"] = GetUInt32(b,offset);
							ie["TS Info"]["Surplus Bandwidth Allowance"] = GetUInt16(b,offset);
							ie["TS Info"]["Medium Time"] = GetUInt16(b,offset);
						}
						break;
					default:
						break;
				}

			} break;
			case 4: {
				ie["TLV Block"] = BufferToHex(&b[offset],l-offset);
			} break;
			default:
				ie["Data"] = BufferToHex(&b[1],l-1);
				break;
			}
		}

		return ie;
	}

	inline nlohmann::json dissect_vendor_ie_rsn(const unsigned char *b, uint l) {
		nlohmann::json ie;
		ie["vendor"] = "Wi-Fi : RSN";

		switch(b[0]) {
		case 4: {
			ie["RSN PMKID"] = BufferToHex(&b[1],16);
		}
			break;
		default:
			ie["RSN Unknown"] = BufferToHex(&b[1],l-1);
			break;
		}
		return ie;
	}

	inline nlohmann::json dissect_vendor_ie_ht(const unsigned char *b, uint l) {
		nlohmann::json ie;
		ie["vendor"] = "Wi-Fi : 802.11 Pre-N";
		--l;
		auto type = b[0];
		switch(type) {
		case 51: {
			dissect_ht_capability_ie(&b[1],l-1,ie);
		}
			break;
		case 52: {
			dissect_ht_info_ie_1_0(&b[1],l-1,ie);
		}
			break;
		default:
			ie["802.11n (Pre) Unknown Data"] = BufferToHex(&b[1],l-1);
			break;
		}
		return ie;
	}

	inline nlohmann::json dissect_vendor_ie_wfa(const unsigned char *b, uint l) {
		nlohmann::json ie;
		ie["vendor"] = "Wi-Fi Alliance";
		ie["Data"] = BufferToHex(&b[1],l-1);
		b++;l++;
		return ie;
	}

	inline nlohmann::json dissect_vendor_ie_aironet(const unsigned char *b, uint l) {
		nlohmann::json ie;
		ie["vendor"] = "Cisco Wireless (Aironet)";
		ie["Data"] = BufferToHex(&b[1],l-1);
		b++;l++;
		return ie;
	}

	inline nlohmann::json dissect_vendor_ie_marvell(const unsigned char *b, uint l) {
		nlohmann::json ie;
		ie["vendor"] = "Marvell Semiconductor";
		ie["Data"] = BufferToHex(&b[1],l-1);
		b++;l++;
		return ie;
	}

	inline nlohmann::json dissect_vendor_ie_atheros(const unsigned char *b, uint l) {
		nlohmann::json ie;
		ie["vendor"] = "Atheros Communications";
		ie["Data"] = BufferToHex(&b[1],l-1);
		b++;l++;
		return ie;
	}

	inline nlohmann::json dissect_vendor_ie_aruba(const unsigned char *b, uint l) {
		nlohmann::json ie;
		ie["vendor"] = "Aruba Networks";
		ie["Data"] = BufferToHex(&b[1],l-1);
		b++;l++;
		return ie;
	}

	inline nlohmann::json dissect_vendor_ie_nintendo(const unsigned char *b, uint l) {
		nlohmann::json ie;
		ie["vendor"] = "Nintendo";
		ie["Data"] = BufferToHex(&b[1],l-1);
		b++;l++;
		return ie;
	}

	inline nlohmann::json WFS_WLAN_EID_VENDOR_SPECIFIC(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;

		if(data.size()>=3) {
			uint offset=0;
			auto RSNOUI = GetUInt24Big(&data[0],offset);

			switch(RSNOUI) {
			case OUI_WPAWME:
				content = dissect_vendor_ie_wpawme(&data[offset], data.size()-offset);
				break;
			case OUI_RSN:
				content = dissect_vendor_ie_rsn(&data[offset], data.size()-offset);
				break;
			case OUI_PRE11N:
				content = dissect_vendor_ie_ht(&data[offset], data.size()-offset);
				break;
			case OUI_WFA:
				content = dissect_vendor_ie_wfa(&data[offset], data.size()-offset);
				break;
			case OUI_CISCOWL:
				content = dissect_vendor_ie_aironet(&data[offset], data.size()-offset);
				break;
			case OUI_MARVELL:
				content = dissect_vendor_ie_marvell(&data[offset], data.size()-offset);
				break;
			case OUI_ATHEROS:
				content = dissect_vendor_ie_atheros(&data[offset], data.size()-offset);
				break;
			case OUI_ARUBA:
				content = dissect_vendor_ie_aruba(&data[offset], data.size()-offset);
				break;
			case OUI_NINTENDO:
				content = dissect_vendor_ie_nintendo(&data[offset], data.size()-offset);
				break;
			default:
				content["content"] = BufferToHex(&data[0],data.size());
				break;
			}

		}

		new_ie["name"]="Vendor Specific";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_VENDOR_SPECIFIC;
		return new_ie;
	}

	inline nlohmann::json WFS_WLAN_EID_EXTENSION(const std::vector<unsigned char> &data) {
		nlohmann::json 	new_ie;
		nlohmann::json 	content;

		// std::cout << BufferToHex(&data[0],data.size()) << std::endl;
		uint offset=0;
		auto sub_ie = data[offset++];
		switch (sub_ie) {
		case WLAN_EID_EXT_HE_CAPABILITY: {
			content["Extension EID"] = BufferToHex(&data[0],1);
			content["Block"] = BufferToHex(&data[1],data.size()-1);
		} break;
		case WLAN_EID_EXT_HE_OPERATION: {
			content["Extension EID"] = BufferToHex(&data[0],1);
			content["Block"] = BufferToHex(&data[1],data.size()-1);
		} break;
		case WLAN_EID_EXT_HE_MU_EDCA: {
			content["Extension EID"] = BufferToHex(&data[0],1);
			content["Block"] = BufferToHex(&data[1],data.size()-1);
		} break;
		default:
			content["Extension EID"] = BufferToHex(&data[0],1);
			content["Block"] = BufferToHex(&data[1],data.size()-1);
			break;
		}

		new_ie["name"]="EI Extensions";
		new_ie["content"]=content;
		new_ie["type"]=WLAN_EID_EXTENSION;
		return new_ie;
	}

	inline bool ParseWifiScan(Poco::JSON::Object::Ptr &Obj, std::stringstream &Result, Poco::Logger &Logger) {
		std::ostringstream	ofs;
		Obj->stringify(ofs);

		try {
			nlohmann::json D = nlohmann::json::parse(ofs.str());
			// std::cout << "Start of parsing wifi" << std::endl;
			if (D.contains("status")) {
				auto Status = D["status"];
				if (Status.contains("scan") && Status["scan"].is_array()) {
					nlohmann::json ScanArray = Status["scan"];
					nlohmann::json ParsedScan = nlohmann::json::array();
					for (auto &scan_entry : ScanArray) {
						if (scan_entry.contains("ies") && scan_entry["ies"].is_array()) {
							auto ies = scan_entry["ies"];
							nlohmann::json new_ies = nlohmann::json::array();
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
										} else if (ie_type ==
												   ieee80211_eid::WLAN_EID_PWR_CONSTRAINT) {
											new_ies.push_back(WFS_WLAN_EID_PWR_CONSTRAINT(data));
										} else if (ie_type == ieee80211_eid::WLAN_EID_ERP_INFO) {
											new_ies.push_back(WFS_WLAN_EID_ERP_INFO(data));
										} else if (ie_type ==
												   ieee80211_eid::
													   WLAN_EID_SUPPORTED_REGULATORY_CLASSES) {
											new_ies.push_back(
												WFS_WLAN_EID_SUPPORTED_REGULATORY_CLASSES(data));
										} else if (ie_type ==
												   ieee80211_eid::WLAN_EID_HT_CAPABILITY) {
											new_ies.push_back(WFS_WLAN_EID_HT_CAPABILITY(data));
										} else if (ie_type ==
												   ieee80211_eid::WLAN_EID_EXT_SUPP_RATES) {
											new_ies.push_back(WFS_WLAN_EID_EXT_SUPP_RATES(data));
										} else if (ie_type ==
												   ieee80211_eid::WLAN_EID_TX_POWER_ENVELOPE) {
											new_ies.push_back(WFS_WLAN_EID_TX_POWER_ENVELOPE(data));
										} else if (ie_type ==
												   ieee80211_eid::WLAN_EID_VHT_CAPABILITY) {
											new_ies.push_back(WFS_WLAN_EID_VHT_CAPABILITY(data));
										} else if (ie_type ==
												   ieee80211_eid::
													   WLAN_EID_RRM_ENABLED_CAPABILITIES) {
											new_ies.push_back(
												WFS_WLAN_EID_RRM_ENABLED_CAPABILITIES(data));
										} else if (ie_type ==
												   ieee80211_eid::WLAN_EID_EXT_CAPABILITY) {
											new_ies.push_back(WFS_WLAN_EID_EXT_CAPABILITY(data));
										} else if (ie_type == ieee80211_eid::WLAN_EID_TPC_REPORT) {
											new_ies.push_back(WFS_WLAN_EID_TPC_REPORT(data));
										} else if (ie_type == ieee80211_eid::WLAN_EID_RSN) {
											new_ies.push_back(WFS_WLAN_EID_RSN(data));
										} else if (ie_type ==
												   ieee80211_eid::WLAN_EID_VENDOR_SPECIFIC) {
											new_ies.push_back(WFS_WLAN_EID_VENDOR_SPECIFIC(data));
										} else if (ie_type == ieee80211_eid::WLAN_EID_EXTENSION) {
											new_ies.push_back(WFS_WLAN_EID_EXTENSION(data));
										} else {
											// std::cout
											//	<< "Skipping IE: no parsing available: " << ie_type
											//	<< std::endl;
											new_ies.push_back(ie);
										}
									} else {
										// std::cout << "Skipping IE: no data and type" << std::endl;
										new_ies.push_back(ie);
									}
								} catch (...) {
									// std::cout << "Skipping IE: exception" << std::endl;
									Logger.information(fmt::format("Error parsing IEs"));
									new_ies.push_back(ie);
								}
							}
							scan_entry["ies"] = new_ies;
							ParsedScan.push_back(scan_entry);
						} else {
							// std::cout << "Skipping scan" << std::endl;
							ParsedScan.push_back(scan_entry);
						}
					}
					Status["scan"] = ParsedScan;
					D["status"] = Status;
				}
			}
			Result << to_string(D);
			// std::cout << "End of parsing wifi" << std::endl;
			return true;
		} catch (const Poco::Exception &E) {
			Logger.log(E);
			Logger.error(fmt::format("Failure to parse WifiScan."));
		} catch (...) {
			Logger.error(fmt::format("Failure to parse WifiScan."));
		}
		return false;
	}

} // namespace OpenWifi

