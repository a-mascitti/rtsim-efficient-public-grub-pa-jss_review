#ifndef __TRACE_POWER_HPP__
#define __TRACE_POWER_HPP__

#include <cpu.hpp>

namespace RTSim {
    class CPU;
    class CPU_BL;
    /**
     * \ingroup server
     *
     * This class exports a periodic trace of the power saved by the CPU.
     * The trace is saved on a file called "power.txt" every 10 msec.
     */
    class TracePowerConsumption:
        public PeriodicTimer, public TraceAscii
    {
    protected:
        CPU* cpu;
        unsigned long long int counter;
        long double totalPowerSaved;
        long double totalPowerConsumed;
    public:
        TracePowerConsumption(CPU* c, Tick period=10, const std::string &filename="power.txt");
        ~TracePowerConsumption();
        long double getAveragePowerSaving();
        long double getAveragePowerConsumption();
	
        /// Periodically updates the variables and writes some values in the file
        virtual void action();
    };

    /**
     * \ingroup server
     *
     * This class exports a periodic trace of the power consumed by the CPU (every tick/simul step).
     * The trace is saved on a file called "power.txt" when the CPU changes frequency.
     * It has been thought for very long runs (e.g., 60.000 sim. steps) and was born with CPU_BL (big LITTLE).
     */
    class TracePowerConsumptionOnChanged :
        public TracePowerConsumption
    {
    private:
        string _formerWorkload = "idle";
        unsigned int _formerFreq = 0u;
    public:
        TracePowerConsumptionOnChanged(CPU* c, const std::string &filename="power.txt");
    
        /// Periodically updates the variables and writes some values in the file, but only if either freq or wl changed
        virtual void action();
    };

}

#endif
