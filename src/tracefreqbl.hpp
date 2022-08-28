#ifndef __TRACE_FREQ_BL_HPP__
#define __TRACE_FREQ_BL_HPP__

#include <cpu.hpp>

namespace RTSim {
    class Island_BL;
    /**
     * \ingroup server
     *
     * This class exports a periodic trace of the frequency saved by the CPU.
     * The trace is saved on a file called "freq.txt" every 1 msec by default.
     * Only the frequency at the end of the simulation step is traced.
     *
     * I've tried to make it easily reusable. For example, on BLMedium, Island might be
     * even the middle island
     */
    class TraceFreqBL:
        public PeriodicTimer, public TraceAscii
    {
        vector<Island_BL*> islands;
        vector<unsigned int> _formerFreq;
    public:
        TraceFreqBL(vector<Island_BL*> isl, Tick period=1, const std::string &filename="freq.txt");
        ~TraceFreqBL();
	
        /// Periodically updates the variables and writes some values in the file
        void action();
    };



    /**
     * \ingroup server
     *
     * Synchronous version of frequency tracer, directly called by the island bl when changing freq.
     * The trace is saved on a file called "freq.txt" only when the island notifies a freq change.
     *
     * The aim of the class is to be fast and to reduce disk accesses and posted tracing events.
     *
     * I've tried to make it easily reusable. For example, on BLMedium, Island might be
     * even the middle island
     */
    class TraceFreqBLSynch : public TraceAscii {
    private:
        Island_BL* _island;
        string _islandType;
        unsigned int _formerFreq = 0u;
    public:
        TraceFreqBLSynch(Island_BL* island, const string &filename);
    
        void onFreqChanged(bool isForced=false);
    };

}

#endif
