#ifndef __TRACE_UTIL_WL_BL_HPP__
#define __TRACE_UTIL_WL_BL_HPP__

#include <trace.hpp>
#include <vector>

namespace RTSim {
    class CPU_BL;
    class Island_BL;

    using namespace std;

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
    class TraceUtilWlSynch : public TraceAscii  {
    private:
        vector<string> _formerWorkloads;
        vector<double> _formerUtils;
        Island_BL* _island;

    public:
        TraceUtilWlSynch(Island_BL* isl, const std::string &filename="util.txt");
	    ~TraceUtilWlSynch();

        void onUtilOrWlChanged(bool isForced = false);
    };

}

#endif
