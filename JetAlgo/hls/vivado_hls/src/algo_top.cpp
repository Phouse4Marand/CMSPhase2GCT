#include "algo_top_parameters.h"
#include "algo_top.h"
#include <algorithm>
#include <utility>

#include "../../../../include/objects.h"
#include "JetObjects.h"
using namespace std;
using namespace algo;


void unpackInputLink(hls::stream<algo::axiword576> &ilink, Tower towers[TOWERS_IN_ETA/2]) {
#pragma HLS PIPELINE II=N_WORDS_PER_FRAME
#pragma HLS ARRAY_PARTITION variable=towers complete dim=0
#pragma HLS INTERFACE axis port=ilink
#pragma HLS INLINE

  ap_uint<576> word_576b_;

#ifndef __SYNTHESIS__
  // Avoid simulation warnings
  if (ilink.empty()) return;
#endif

  word_576b_ = ilink.read().data;

  towers[0]  = Tower(word_576b_( 31,   0));
  towers[1]  = Tower(word_576b_( 63,  32));
  towers[2]  = Tower(word_576b_( 95,  64));
  towers[3]  = Tower(word_576b_(127,  96));
  towers[4]  = Tower(word_576b_(159, 128));
  towers[5]  = Tower(word_576b_(191, 160));
  towers[6]  = Tower(word_576b_(223, 192));
  towers[7]  = Tower(word_576b_(255, 224));
  towers[8]  = Tower(word_576b_(287, 256));
  towers[9]  = Tower(word_576b_(319, 288));
  towers[10] = Tower(word_576b_(351, 320));
  towers[11] = Tower(word_576b_(383, 352));
  towers[12] = Tower(word_576b_(415, 384));
  towers[13] = Tower(word_576b_(447, 416));
  towers[14] = Tower(word_576b_(479, 448));

  return;
}

void packOutput(Region region[10], hls::stream<algo::axiword576> &olink){
#pragma HLS PIPELINE II=N_OUTPUT_WORDS_PER_FRAME
#pragma HLS ARRAY_PARTITION variable=region complete dim=0
#pragma HLS INTERFACE axis port=olink
#pragma HLS INLINE

  ap_uint<576> word_576b_;

  word_576b_( 37,   0) = (ap_uint<38>) region[0].data;
  word_576b_( 75,  38) = (ap_uint<38>) region[1].data;
  word_576b_(113,  76) = (ap_uint<38>) region[2].data;
  word_576b_(151, 114) = (ap_uint<38>) region[3].data;
  word_576b_(189, 152) = (ap_uint<38>) region[4].data;
  word_576b_(227, 190) = (ap_uint<38>) region[5].data;
  word_576b_(265, 228) = (ap_uint<38>) region[6].data;
  word_576b_(303, 266) = (ap_uint<38>) region[7].data;
  word_576b_(341, 304) = (ap_uint<38>) region[8].data;
  word_576b_(379, 342) = (ap_uint<38>) region[9].data;
  word_576b_(575, 380) = 0;

  axiword576 r; r.last = 0; r.user = 0;
  r.data = word_576b_;
  
  olink.write(r);

  return ;
}


void algo_top(hls::stream<axiword576> link_in[N_INPUT_LINKS], hls::stream<axiword576> link_out[N_OUTPUT_LINKS]) {
#pragma HLS INTERFACE axis port=link_in
#pragma HLS INTERFACE axis port=link_out
#pragma HLS PIPELINE II=N_WORDS_PER_FRAME

#pragma HLS ARRAY_PARTITION variable=link_in complete dim=0
#pragma HLS ARRAY_PARTITION variable=link_out complete dim=0


  // Step 1: Unpack links
  // Input is 60 links carrying 30phix30eta towers
  Tower towers[TOWERS_IN_PHI][TOWERS_IN_ETA];
#pragma HLS ARRAY_PARTITION variable=towers complete dim=0
     
  for (size_t ilink = 0; ilink < N_INPUT_LINKS/2; ilink++) {
#pragma LOOP UNROLL
#pragma HLS latency min=1
    size_t iPosEta = ilink;
    size_t iNegEta = ilink+N_INPUT_LINKS/2;
    unpackInputLink(link_in[iNegEta], &towers[ilink][0]);
    unpackInputLink(link_in[iPosEta], &towers[ilink][TOWERS_IN_ETA/2]);
  }


  // Step 2: Jet Algo goes here
  Region reg3x3[100];
  size_t index=0;
  for(size_t ieta = 1; ieta<TOWERS_IN_ETA-1; ieta+=3){
#pragma LOOP UNROLL
    for(size_t iphi = 1; iphi<TOWERS_IN_PHI-1 && index < 100; iphi+=3, index+=1){
#pragma LOOP UNROLL

      ap_uint<10> seed_et = towers[iphi][ieta].tower_et();
      ap_uint<6> tphi = iphi;
      ap_uint<5> teta = ieta;
      ap_uint<3> time = towers[iphi][ieta].peak_time();
      ap_uint<14> region_et = get3x3Sum(
	  towers[iphi-1][ieta+1], towers[iphi][ieta+1], towers[iphi+1][ieta+1],
	  towers[iphi-1][ieta], towers[iphi][ieta], towers[iphi+1][ieta],
	  towers[iphi-1][ieta-1], towers[iphi][ieta-1], towers[iphi+1][ieta-1]);

      reg3x3[index] = Region(seed_et, region_et, tphi, teta, time);

    }
  }

  // Step 3: Pack the outputs
  for(size_t olink=0; olink<10; olink++){
#pragma LOOP UNROLL
   packOutput(&reg3x3[olink*10], link_out[olink]); 
  }
//-  for (size_t olink = 0; olink < N_OUTPUT_LINKS/2; olink++) {
//-#pragma LOOP UNROLL
//-#pragma HLS latency min=1
//-    size_t iPosEta = olink;              
//-    size_t iNegEta = olink + (N_OUTPUT_LINKS/2);
//-    packOutput(&towers[olink][0], link_out[iNegEta]);
//-    packOutput(&towers[olink][TOWERS_IN_ETA/2], link_out[iPosEta]);
//-  }
}
