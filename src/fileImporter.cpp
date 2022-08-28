/***************************************************************************
begin                : Thu Apr 24 15:54:58 CEST 2003
copyright            : (C) 2003 by Giuseppe Lipari
email                : lipari@sssup.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <fileImporter.hpp>
#include <energyMRTKernel.hpp>
#include <inttypes.h>

static char s[1024]        = "";
static char filename[512]  = "";

namespace RTSim
{
    // using namespace std;

    string StaffordImporter::generate(unsigned int period, double utilization, unsigned int periodGran/*=500u*/, int64_t maxLCM /*=-1*/, bool randomPeriod /*= false*/, unsigned int nCPUs/*=6*/, unsigned int nTasks/*=4*/) {
        cout << "StaffordImporter::" << __func__ << "()" << endl;
        
        string absPath      = "";

        time_t n            = time(0);
        struct tm* _tm      = localtime(&n);
        char now[50]        = "";
        sprintf(now, "%d_%d_%d_%d_%d_%d", _tm->tm_mday, _tm->tm_mon + 1, _tm->tm_year + 1900, _tm->tm_hour, _tm->tm_min, _tm->tm_sec);

        if (randomPeriod) { // periods in [1000000-100000000] us
          sprintf(filename, "taskset_generator/%s_Pmin_%u_Pmax_%u_Pgran_%u_u_%f.txt", now, 1000000u, 100000000u, periodGran, utilization);
          sprintf(s, "python3 taskset_generator/taskgen3.py -s %u -n %u --period-min %u --period-max %u --period-gran %u --max-lcm %" PRId64 " -u %f --round-C -d 235 > %s", nCPUs, nTasks, 1000000u, 100000000u, periodGran, maxLCM, utilization, filename);
        }
        else {
          sprintf(filename, "taskset_generator/%s_P_%u_u_%f.txt", now, period, utilization);
          sprintf(s, "python3 taskset_generator/taskgen3.py -s %u -n %u -p %u -u %f --round-C > %s", nCPUs, nTasks, period, utilization, filename);
        }

        if (std::ifstream(filename)) {
          remove(filename);
        }

        cout << "executing: " << endl << s << endl;

        if (system(s) < 0) { cerr << "Error while creating tasket." << endl; abort(); }
        cout << "file made" << endl;
        saveLastGenerated(filename);
        return string(filename);
    }

    /// Imports tasks from file. Returns CPU/set number -> (WCET, Period = Deadline) for all CPU, tasks.
    map<unsigned int, pair<unsigned int, unsigned int>> StaffordImporter::importFromFile(const string& filename) {
       cout << "StaffordImporter::" << __func__ << "() " << filename << endl;
       assert (_isFileExisting(filename));

       ifstream fd(filename.c_str());
       double trash, util, wcet, period;
       map<unsigned int, pair<unsigned int, unsigned int>> tasks;
       int i = 0;

       while (fd >> trash >> util >> wcet >> period) {
          tasks[i] = make_pair((unsigned int) wcet, (unsigned int) period);
          i++;
       }

       for (const auto &elem : tasks) {
          double u = double(elem.second.first) / double(elem.second.second);
          cout << "\t" << elem.first << ")\twcet " << elem.second.first << " period " << elem.second.second << endl;
       }

       fd.close();
       cout << "\timported " << tasks.size() << endl;
       return tasks;
    }

    vector<PeriodicTask*> StaffordImporter::getPeriodicTasks(const string& filename, const int experiment_no) {
        cout << "StaffordImporter::" << __func__ << "()" << endl;

        map<unsigned int, pair<unsigned int, unsigned int>> tasks = importFromFile(filename);
        vector<PeriodicTask*> res;
        char instr[60] = "";

        for (const auto &elem : tasks) {
            PeriodicTask* t = new PeriodicTask((int)elem.second.second, (int)elem.second.second, 0, "t_" + to_string(experiment_no) + "_" + to_string(elem.first));
            sprintf(instr, "fixed(%d, %s);", elem.second.first, "bzip2");
            t->insertCode(instr);
	    t->setIndex(elem.first);
            if (t->getWCET(1.0) == 0.0) // stafford may generate tasks with WCET 0
              throw std::invalid_argument("Creating task with WCET 0");
            res.push_back(t);
        }

        cout << "\t" << __func__ << "(). read " << res.size() << endl;

        return res;
    }

    vector<CBServerCallingEMRTKernel*> StaffordImporter::getEnvelopedPeriodcTasks(const string& filename, EnergyMRTKernel *kern, const int experiment_no /*= -1*/, unsigned int nCPUs /*=8*/, unsigned int nTasksPerCore/*=3*/) {
        assert (filename != ""); assert (kern != NULL);
        cout << "StaffordImporter::" << __func__ << "()" << endl;

        vector<CBServerCallingEMRTKernel*> ets;
        vector<PeriodicTask*> tasks = getPeriodicTasks(filename, experiment_no);
        cout << "received " << tasks.size() << endl;

        for (PeriodicTask* t : tasks) {
          ets.push_back(kern->addTaskAndEnvelope(t, ""));
        }

        for (CBServerCallingEMRTKernel* e : ets)
          assert ( double(e->getBudget()) > e->getAllTasks().at(0)->getWCET(1.0) );

        return ets;
    }


    string StaffordImporter::getLastGenerated() {
        string filename = "";

        ifstream in("taskset_generator/saved.conf");
        in >> filename;
        in.close();

        assert(filename != "");

        return filename;
    }

    /// Stafford alg. can generate tasks with WCET 0, which is useless
    bool StaffordImporter::check(const string& filename) {
      assert (filename != "");
      cout << "StaffordImporter::" << __func__ << "(filename=" << filename << ")" << endl;
      bool res = true;

      ifstream fd(filename.c_str());
      double trash, util, wcet, period;
      while (fd >> trash >> util >> wcet >> period) {
        if (wcet == 0.0) {
          res = false;
          break;
        }
      }

      fd.close();
      return res;
    }
    
    bool StaffordImporter::_isFileExisting(const string& absFilename) {
        std::ifstream ifile(absFilename.c_str());
        return (bool) ifile;
    }

    int StaffordImporter::_getRandom(unsigned int min, unsigned int max) // range : [min, max)
    {
        // from https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
        random_device rd;  // obtain a random number from hardware
        mt19937 eng(rd()); // seed the generator
        uniform_int_distribution<> distr(min, max); // define the range

        return distr(eng);
    }

    void StaffordImporter::saveLastGenerated(const string& filename) {
        assert (filename != "");
        cout << "StaffordImporter::" << __func__ << "()" << endl;

        ofstream out("taskset_generator/saved.conf");
        out << filename;
        out.close();
    }

}
