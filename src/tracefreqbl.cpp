#include "tracefreqbl.hpp"

namespace RTSim {
    using namespace std;
    
    TraceFreqBL::TraceFreqBL(vector<Island_BL*> isl, Tick period, const string &filename) : 
        PeriodicTimer(period), TraceAscii(filename)
    {
        assert (isl.size() == 2);
        islands = isl;
        _formerFreq.push_back(islands.at(0)->getFrequency());
        _formerFreq.push_back(islands.at(1)->getFrequency());
    }
  
    TraceFreqBL::~TraceFreqBL()
    {
        
    }

    void TraceFreqBL::action()
    {
        /* It periodically updates the variables: */
        for (Island_BL* i : islands) {
          double freq = i->getFrequency();
          IslandType islandType = i->getIslandType();
          if (_formerFreq[islandType] != freq) {
            _formerFreq[islandType] = freq;
            string type = (islandType == IslandType::BIG ? "big" : "LITTLE");
            record(to_string(int64_t(SIMUL.getTime())) + " " + type + " ");
            record(freq);
          }
        }
    }




    TraceFreqBLSynch::TraceFreqBLSynch(Island_BL* island, const string &filename) : 
        TraceAscii(filename)
    {
        _island = island;
        _formerFreq = island->getFrequency();
        _islandType = (island->getIslandType() == IslandType::BIG ? "big" : "LITTLE");
	onFreqChanged(true);
    }
    
    void TraceFreqBLSynch::onFreqChanged(bool isForced)
    {
        double freq = _island->getFrequency();
        if (isForced || _formerFreq != freq) {  // just a double check to save disk accesses
            _formerFreq = freq;
            record(to_string(int64_t(SIMUL.getTime())) + " " + _islandType + " ");
            record(freq);
        }
    }

}
