#ifndef __FILE_IMPORTER_H__
#define __FILE_IMPORTER_H__

#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <random>
#include <map>

namespace RTSim {
    // using namespace MetaSim;
    using namespace std;

    class PeriodicTask;
    class CBServerCallingEMRTKernel;
    class EnergyMRTKernel;

    class StaffordImporter {
    public:

      // static const string& FOLDER_ROOT() { static string c = "./"; return c; } // sorry, rush...

      /**
          Generate periodic tasks with Stafford algorithm, with given period and utilization.
          If randomPeriod = true, then periods are random
        */
      static string generate(unsigned int period, double utilization, unsigned int periodGran=500u, int64_t maxLCM = -1, bool randomPeriod = false, unsigned int nCPUs=6, unsigned int nTasks=4);

      /// Imports tasks from file. Returns CPU/set number -> (WCET, Period = Deadline) for all CPU, tasks.
      static map<unsigned int, pair<unsigned int, unsigned int>> importFromFile(const string& filename);

      static vector<PeriodicTask*> getPeriodicTasks(const string& filename, const int experiment_no);

      static vector<CBServerCallingEMRTKernel*> getEnvelopedPeriodcTasks(const string& filename, EnergyMRTKernel *kern, const int experiment_no = -1, unsigned int nCPUs = 8, unsigned int nTasksPerCore = 3);

      static string getLastGenerated();

      /// Stafford alg. can generate tasks with WCET 0, which is useless
      static bool check(const string& filename);
      
    private:
      static bool _isFileExisting(const string& absFilename);

      static int _getRandom(unsigned int min, unsigned int max);

      static void saveLastGenerated(const string& filename);

    };

}


#endif
