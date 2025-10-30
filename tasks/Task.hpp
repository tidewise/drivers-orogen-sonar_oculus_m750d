/* Generated from orogen/lib/orogen/templates/tasks/Task.hpp */

#ifndef SONAR_OCULUS_M750D_TASK_TASK_HPP
#define SONAR_OCULUS_M750D_TASK_TASK_HPP

#include "sonar_oculus_m750d/Driver.hpp"
#include "sonar_oculus_m750d/TaskBase.hpp"
#include <base/Timeout.hpp>

namespace sonar_oculus_m750d {
    class Task : public TaskBase {
        friend class TaskBase;

    protected:
        std::unique_ptr<sonar_oculus_m750d::Driver> m_driver;
        M750DConfiguration m_fire_config;

        base::Pressure m_safe_working_pressure;
        base::Timeout m_pressure_data_timeout;
        bool m_init_unsafe_working_pressure = false;

        base::Timeout m_recover_minimum_time;

        void processIO();

        // checks for working safety based on pressure input and set m_safe_to_work state
        // accordingly
        bool safeToWork();
        void fireSonar();

    public:
        Task(std::string const& name = "sonar_oculus_m750d::Task");
        ~Task();
        bool configureHook();
        bool startHook();
        void updateHook();
        void errorHook();
        void stopHook();
        void cleanupHook();
    };
}

#endif
