#!/bin/bash
source ./autobuild/lede-build-sanity.sh
#get the brach_name
temp=${0%/*}
branch_name=${temp##*/}
backport_new=0

#step1 clean
#clean

#do prepare stuff
prepare

#hack mt7622 config-5.4
echo "CONFIG_NETFILTER=y" >> ./target/linux/mediatek/mt7622/config-5.4
echo "CONFIG_NETFILTER_ADVANCED=y" >> ./target/linux/mediatek/mt7622/config-5.4

prepare_flowoffload

prepare_mac80211 ${backport_new}

prepare_final ${branch_name}

#step2 build
build ${branch_name} -j1 || [ "$LOCAL" != "1" ]
