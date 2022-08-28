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
#ifndef __PSTRACE_HPP__
#define __PSTRACE_HPP__

#include <fstream>
#include <iostream>
#include <string>

#include <baseexc.hpp>
#include <basetype.hpp>
#include <event.hpp>
#include <particle.hpp>
#include <trace.hpp>

#include <rttask.hpp>
#include <taskevt.hpp>

namespace RTSim {
    class CBServerCallingEMRTKernel;
    class Island_BL;

    class PSTrace {
    protected:
        std::ofstream fd;
        bool first_event;

        /// Events will be stored into file since this tick (to save disk accesses and exec times)
        Tick startingTick = Tick(0);

        void writeTaskEvent(const Task &tt, const std::string &evt_name, TaskEvt* evt);

    public:
        PSTrace(const std::string& name);
        ~PSTrace();

        void probe(ArrEvt& e);
        void probe(EndEvt& e);
        void probe(SchedEvt& e);
        void probe(DeschedEvt& e);
        void probe(DeadEvt& e);
        void attachToTask(Task& t);

        void writeTasks(vector<CBServer*> tasks, vector<AbsRTTask*> tasksinside, string filename);
        void writeCPUs (vector<Island_BL*> islands, string filename);

        /// write whatever you want into file
        void write(const AbsTask *tt, const std::string &str, bool isTimeCheckEnabled = true);

        void setStartingTick(Tick &tick) { startingTick = tick; }
    };
}

#endif

