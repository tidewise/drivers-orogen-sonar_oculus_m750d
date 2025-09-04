/* Generated from orogen/lib/orogen/templates/tasks/Task.hpp */

#ifndef SONAR_OCULUS_M750D_TASK_TASK_HPP
#define SONAR_OCULUS_M750D_TASK_TASK_HPP

#include "sonar_oculus_m750d/TaskBase.hpp"

namespace sonar_oculus_m750d{
    class Task : public TaskBase
    {
	friend class TaskBase;
    protected:
        M750DConfiguration m_fire_config;

    public:
        Task(std::string const& name = "sonar_oculus_m750d::Task");
	    ~Task();
        bool configureHook();
        bool startHook();
        void updateHook();
        void errorHook();
        void stopHook();
        void cleanupHook();
        void processIO();
    };
}

#endif

