#include "tracepower.hpp"

namespace RTSim {
    using namespace std;
    
    TracePowerConsumption::TracePowerConsumption(CPU* c, Tick period, const string &filename) : 
        PeriodicTimer(period), TraceAscii(filename), counter(0),totalPowerSaved(0),totalPowerConsumed(0) 
    {
        cpu = c;
    }   
  
    TracePowerConsumption::~TracePowerConsumption()
    {
        cpu = NULL;
    }
  
    long double TracePowerConsumption::getAveragePowerSaving() 
    {
        long double TPS = (totalPowerSaved) / ((long double) counter ); 
        if (counter > 0) return TPS;
        return 0;
    }
  
    long double TracePowerConsumption::getAveragePowerConsumption() 
    {
        long double TPC = (totalPowerConsumed) / ((long double) counter ); 
        if (counter > 0)
            return TPC/(cpu->getMaxPowerConsumption());
        return 0;
    }

    void TracePowerConsumption::action()
    {
        /* It periodically updates the variables: */
        double currentPowerConsumption = cpu->getCurrentPowerConsumption();
        totalPowerConsumed += currentPowerConsumption;
        counter++;
    
        long double TPC = getAveragePowerConsumption();
        //record("Average Normalized Power Consumption:");
        //record(TPC);
        record("Current Power Consumption t=" + to_string(int(SIMUL.getTime())) +":");
        record(currentPowerConsumption);
    }




    TracePowerConsumptionOnChanged::TracePowerConsumptionOnChanged(CPU* c, const std::string &filename) : 
        TracePowerConsumption(c, Tick(1), filename)
    {
        assert (c->getWorkload() == "idle");
        _formerFreq = c->getFrequency();
    }


    void TracePowerConsumptionOnChanged::action() {
        const string curWorkload = cpu->getWorkload();
        const unsigned int curFrequency = cpu->getFrequency();
        if (curWorkload != _formerWorkload || curFrequency != _formerFreq) {
            _formerWorkload = curWorkload;
            _formerFreq     = curFrequency;
            double currentPowerConsumption = cpu->getCurrentPowerConsumption();
            string stringa = "Cur W=" + to_string(currentPowerConsumption) + " " + curWorkload + " t=";
	    record(stringa);
	    record(double(SIMUL.getTime()));
        }
    }

}
