#ifndef __TRACE_POWER_BL_HPP__
#define __TRACE_POWER_BL_HPP__

#include <trace.hpp>

namespace RTSim {
    class CPU_BL;
    /**
     * \ingroup server
     *
     * Synchronous version of power tracer, directly called by the CPU_BL when changing freq or wl.
     * The trace is saved on a file called "power.txt" only when the island notifies a freq change.
     *
     * The aim of the class is to be fast and to reduce disk accesses and posted tracing events.
     *
     * I've tried to make it easily reusable. For example, on BLMedium, Island might be
     * even the middle island
     */
    class TracePowerConsumptionSynch : public TraceAscii  {
    private:
        CPU_BL* _cpu = NULL;

        Island_BL* _island = NULL;
        vector<unsigned int> _formerFreqs;
        vector<string> _formerWorkloads;

	    // stringstream _ss;
    public:
        TracePowerConsumptionSynch(CPU_BL* cpu, const std::string &filename="power.txt");
	    TracePowerConsumptionSynch(Island_BL* island, const std::string &filename="power.txt");
	    ~TracePowerConsumptionSynch();

        // void setOutputFilename(const string& filename="power.txt"); 
    
        void onFreqOrWlChanged();
    };

}

#endif
