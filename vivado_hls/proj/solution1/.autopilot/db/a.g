#!/bin/sh
lli=${LLVMINTERP-lli}
exec $lli \
    /data/varuns/CMSPhase2HLS/APx_Gen0_Algo/VivadoHls/GCT_algo_unpacked_Dev/vivado_hls/proj/solution1/.autopilot/db/a.g.bc ${1+"$@"}