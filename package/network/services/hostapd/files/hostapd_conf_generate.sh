#!/bin/sh

usage()
{
	echo " "
	echo "USAGE: hostapd_generate.sh [interface: ra0|rai0] [security option: open|wpapsk|wpa2psk|wpawpa2mixedpsk|wepopen|wepshare|EnterpriseAP|wpsopen|wpswpa2psk|sae|suiteB]"
}

if [ $# -lt 2 ]; then
	echo "Wrong Input!!!"
	usage
	exit 1
fi

wif_if=$1;
sec_type=$2;
if [ $wif_if == "ra0" ]; then
	band=5G;
else
	band=2G;
fi

if [ $sec_type == "EnterpriseAP" ]; then
	conf_file=hostapd_${wif_if}_wpa2$sec_type.conf;
else
	conf_file=hostapd_${wif_if}_$sec_type.conf;
fi
cp /etc/hostapd_common.conf $conf_file

# Interface name update
sed -i 's/^interface=.*/interface='${wif_if}'/g' $conf_file

if [ $sec_type == "open" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_OPEN/g' $conf_file
elif [ $sec_type == "wpapsk" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_WPAPSK/g' $conf_file
	sed -i 's/^.*wpa=.*/wpa=1/g' $conf_file
	sed -i 's/^.*wpa_passphrase=.*/wpa_passphrase=12345678/g' $conf_file
	sed -i 's/^.*wpa_key_mgmt=.*/wpa_key_mgmt=WPA-PSK/g' $conf_file
	sed -i 's/^.*wpa_pairwise=.*/wpa_pairwise=TKIP/g' $conf_file
elif [ $sec_type == "wpa2psk" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_WPA2PSK/g' $conf_file
	sed -i 's/^.*wpa=.*/wpa=2/g' $conf_file
	sed -i 's/^.*wpa_passphrase=.*/wpa_passphrase=12345678/g' $conf_file
	sed -i 's/^.*wpa_key_mgmt=.*/wpa_key_mgmt=WPA-PSK/g' $conf_file
	sed -i 's/^.*rsn_pairwise=.*/rsn_pairwise=CCMP/g' $conf_file
elif [ $sec_type == "wpawpa2mixedpsk" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_WPAWPA2MixedPSK/g' $conf_file
	sed -i 's/^.*wpa=.*/wpa=3/g' $conf_file
	sed -i 's/^.*wpa_passphrase=.*/wpa_passphrase=12345678/g' $conf_file
	sed -i 's/^.*wpa_key_mgmt=.*/wpa_key_mgmt=WPA-PSK/g' $conf_file
	sed -i 's/^.*wpa_pairwise=.*/wpa_pairwise=TKIP CCMP/g' $conf_file
elif [ $sec_type == "wepopen" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_WEP_OPEN/g' $conf_file
	sed -i 's/^.*wep_key0=.*/wep_key0=1234567890/g' $conf_file
	sed -i 's/^.*wep_default_key=.*/wep_default_key=0/g' $conf_file
elif [ $sec_type == "wepshare" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_WEP_SHARED/g' $conf_file
	sed -i 's/^.*wep_key0=.*/wep_key0=1234567890/g' $conf_file
	sed -i 's/^.*wep_default_key=.*/wep_default_key=0/g' $conf_file
	sed -i 's/^.*auth_algs=.*/auth_algs=2/g' $conf_file
elif [ $sec_type == "EnterpriseAP" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_WPA2ENTERPRISE/g' $conf_file
	sed -i 's/^.*auth_algs=.*/auth_algs=3/g' $conf_file
	sed -i 's/^.*wpa=.*/wpa=2/g' $conf_file
	sed -i 's/^.*wpa_key_mgmt=.*/wpa_key_mgmt=WPA-EAP/g' $conf_file
	sed -i 's/^.*rsn_pairwise=.*/rsn_pairwise=CCMP/g' $conf_file
	sed -i 's/^.*ieee8021x=.*/ieee8021x=1/g' $conf_file
	sed -i 's/^.*own_ip_addr=.*/own_ip_addr=192.168.1.1/g' $conf_file
	sed -i 's/^.*auth_server_addr=.*/auth_server_addr=192.168.1.15/g' $conf_file
	sed -i 's/^.*auth_server_port=.*/auth_server_port=1812/g' $conf_file
	sed -i 's/^.*auth_server_shared_secret=.*/auth_server_shared_secret=12345678/g' $conf_file
	sed -i 's/^.*rsn_preauth=.*/rsn_preauth=1/g' $conf_file
	sed -i 's/^.*rsn_preauth_interfaces=.*/rsn_preauth_interfaces=br-lan/g' $conf_file
elif [ $sec_type == "wpsopen" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_WPSOPEN/g' $conf_file
	sed -i 's/^.*eap_server=.*/eap_server=1/g' $conf_file
	sed -i 's/^.*wps_state=.*/wps_state=2/g' $conf_file
	sed -i 's/^.*wps_independent=.*/wps_independent=1/g' $conf_file
	sed -i 's/^.*device_name=/device_name=/g' $conf_file
	sed -i 's/^.*manufacturer=/manufacturer=/g' $conf_file
	sed -i 's/^.*model_name=.*/model_name=WAP/g' $conf_file
	sed -i 's/^.*model_number=.*/model_number=123/g' $conf_file
	sed -i 's/^.*serial_number=.*/serial_number=12345/g' $conf_file
	sed -i 's/^.*device_type=.*/device_type=6-0050F204-1/g' $conf_file
	sed -i 's/^.*config_methods=/config_methods=/g' $conf_file
elif [ $sec_type == "wpswpa2psk" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_WPSWPA2PSK/g' $conf_file
	sed -i 's/^.*wpa=.*/wpa=2/g' $conf_file
	sed -i 's/^.*wpa_passphrase=.*/wpa_passphrase=12345678/g' $conf_file
	sed -i 's/^.*wpa_key_mgmt=.*/wpa_key_mgmt=WPA-PSK/g' $conf_file
	sed -i 's/^.*rsn_pairwise=.*/rsn_pairwise=CCMP/g' $conf_file
	sed -i 's/^.*eapol_version=.*/eapol_version=2/g' $conf_file
	sed -i 's/^.*eap_server=.*/eap_server=1/g' $conf_file
	sed -i 's/^.*wps_state=.*/wps_state=2/g' $conf_file
	sed -i 's/^.*wps_independent=.*/wps_independent=1/g' $conf_file
	sed -i 's/^.*device_name=/device_name=/g' $conf_file
	sed -i 's/^.*manufacturer=/manufacturer=/g' $conf_file
	sed -i 's/^.*model_name=.*/model_name=WAP/g' $conf_file
	sed -i 's/^.*model_number=.*/model_number=123/g' $conf_file
	sed -i 's/^.*serial_number=.*/serial_number=12345/g' $conf_file
	sed -i 's/^.*device_type=.*/device_type=6-0050F204-1/g' $conf_file
	sed -i 's/^.*config_methods=/config_methods=/g' $conf_file
elif [ $sec_type == "sae" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_SAE/g' $conf_file
	sed -i 's/^.*wpa=.*/wpa=2/g' $conf_file
	sed -i 's/^.*wpa_passphrase=.*/wpa_passphrase=12345678/g' $conf_file
	sed -i 's/^.*wpa_key_mgmt=.*/wpa_key_mgmt=SAE/g' $conf_file
	sed -i 's/^.*rsn_pairwise=.*/rsn_pairwise=CCMP/g' $conf_file
	sed -i 's/^.*eapol_version=.*/eapol_version=2/g' $conf_file
	sed -i 's/^.*ieee80211w=.*/ieee80211w=2/g' $conf_file
elif [ $sec_type == "suiteB" ]; then
	sed -i 's/^ssid=.*/ssid='${band}'_SuiteB/g' $conf_file
	sed -i 's/^.*auth_algs=.*/auth_algs=3/g' $conf_file
	sed -i 's/^.*ctrl_interface_group=.*/ctrl_interface_group=0/g' $conf_file
	sed -i 's/^.*wpa=.*/wpa=2/g' $conf_file
	sed -i 's/^.*wpa_passphrase=.*/wpa_passphrase=12345678/g' $conf_file
	sed -i 's/^.*wpa_key_mgmt=.*/wpa_key_mgmt=WPA-EAP-SUITE-B-192/g' $conf_file
	sed -i 's/^.*wpa_group_rekey=.*/wpa_group_rekey=30000/g' $conf_file
	sed -i 's/^.*ieee8021x=.*/ieee8021x=1/g' $conf_file
	sed -i 's/^.*ieee80211w=.*/ieee80211w=2/g' $conf_file
	sed -i 's/^.*own_ip_addr=.*/own_ip_addr=192.168.1.1/g' $conf_file
	sed -i 's/^.*auth_server_addr=.*/auth_server_addr=192.168.1.15/g' $conf_file
	sed -i 's/^.*auth_server_port=.*/auth_server_port=1812/g' $conf_file
	sed -i 's/^.*auth_server_shared_secret=.*/auth_server_shared_secret=1234567890123456789012345678901234567890123456789012345678901234/g' $conf_file
	sed -i 's/^.*rsn_preauth=.*/rsn_preauth=1/g' $conf_file
	sed -i 's/^.*rsn_preauth_interfaces=.*/rsn_preauth_interfaces=br-lan/g' $conf_file
	sed -i 's/^.*rsn_pairwise=.*/rsn_pairwise=GCMP-256/g' $conf_file
	sed -i 's/^.*group_mgmt_cipher=.*/group_mgmt_cipher=BIP-GMAC-256/g' $conf_file
else
	echo $sec_type" is not supported!!!"
	rm -rf $conf_file
	usage
	exit 1;
fi

echo $wif_if"("$band") "$sec_type" mode ==> File generated: "$conf_file
