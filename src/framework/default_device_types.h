//
// Created by stephane bourque on 2023-04-19.
//

#pragma once

#include <vector>
#include <string>

#include "ow_constants.h"

namespace OpenWifi {
	inline  const std::vector<std::pair<std::string, std::string>> DefaultDeviceTypeList{
		{"actiontec_web7200", Platforms::AP},
		{"cig_wf186w", Platforms::AP},
		{"cig_wf188n", Platforms::AP},
		{"cig_wf194c4", Platforms::AP},
		{"cig_wf196", Platforms::AP},
		{"cig_wf196-ca", Platforms::AP},
		{"cig_wf196-ca-ath12", Platforms::AP},
		{"cig_wf196-us", Platforms::AP},
		{"cig_wf610d", Platforms::AP},
		{"cig_wf660a", Platforms::AP},
		{"cig_wf808", Platforms::AP},
		{"cybertan_eww622-a1", Platforms::AP},
		{"edgecore_eap101", Platforms::AP},
		{"edgecore_eap101-ath12", Platforms::AP},
		{"edgecore_eap102", Platforms::AP},
		{"edgecore_eap104", Platforms::AP},
		{"edgecore_eap104-ath12", Platforms::AP},
		{"edgecore_ecs4100-12ph", Platforms::AP},
		{"edgecore_ecw5211", Platforms::AP},
		{"edgecore_ecw5410", Platforms::AP},
		{"edgecore_oap100", Platforms::AP},
		{"edgecore_spw2ac1200", Platforms::SWITCH},
		{"edgecore_spw2ac1200-lan-poe", Platforms::SWITCH},
		{"edgecore_ssw2ac2600", Platforms::SWITCH},
		{"hfcl_ion4", Platforms::AP},
		{"hfcl_ion4x", Platforms::AP},
		{"hfcl_ion4x_2", Platforms::AP},
		{"hfcl_ion4xe", Platforms::AP},
		{"hfcl_ion4xi", Platforms::AP},
		{"indio_um-305ac", Platforms::AP},
		{"indio_um-305ax", Platforms::AP},
		{"indio_um-310ax-v1", Platforms::AP},
		{"indio_um-325ac", Platforms::AP},
		{"indio_um-510ac-v3", Platforms::AP},
		{"indio_um-510axm-v1", Platforms::AP},
		{"indio_um-510axp-v1", Platforms::AP},
		{"indio_um-550ac", Platforms::AP},
		{"linksys_e8450-ubi", Platforms::AP},
		{"linksys_ea6350-v4", Platforms::AP},
		{"linksys_ea8300", Platforms::AP},
		{"liteon_wpx8324", Platforms::AP},
		{"meshpp_s618_cp01", Platforms::AP},
		{"meshpp_s618_cp03", Platforms::AP},
		{"udaya_a5-id2", Platforms::AP},
		{"wallys_dr40x9", Platforms::AP},
		{"wallys_dr6018", Platforms::AP},
		{"wallys_dr6018_v4", Platforms::AP},
		{"x64_vm", Platforms::AP},
		{"yuncore_ax840", Platforms::AP},
		{"yuncore_fap640", Platforms::AP},
		{"yuncore_fap650", Platforms::AP}};
}
