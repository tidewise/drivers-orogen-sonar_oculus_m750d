/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <chrono>
#include <iodrivers_base/ConfigureGuard.hpp>
#include <sonar_oculus_m750d/Driver.hpp>
#include <thread>

using namespace sonar_oculus_m750d;
using namespace std;
using namespace base;
using namespace chrono_literals;

Task::Task(std::string const& name)
    : TaskBase(name)
{
    _safe_working_pressure.set(base::Pressure::fromPascal(120000));
    _pressure_data_timeout.set(base::Time::fromMilliseconds(300));
}

Task::~Task()
{
}

bool Task::configureHook()
{
    auto beam_width = _beam_width.get();
    auto beam_height = _beam_height.get();
    m_update_rate = _update_rate.get();
    std::unique_ptr<sonar_oculus_m750d::Driver> driver(
        new sonar_oculus_m750d::Driver(beam_width, beam_height));
    iodrivers_base::ConfigureGuard guard(this);
    if (!_io_port.get().empty())
        driver->openURI(_io_port.get());
    setDriver(driver.get());
    if (!TaskBase::configureHook()) {
        return false;
    }
    guard.commit();
    m_fire_config = _fire_configuration.get();
    m_driver = move(driver);

    m_safe_working_pressure = _safe_working_pressure.get();
    m_pressure_data_timeout = Timeout(_pressure_data_timeout.get());
    m_recover_minimum_time = Timeout(Time::fromSeconds(3));
    stopSonar();
    return true;
}

bool Task::startHook()
{
    if (!TaskBase::startHook())
        return false;

    m_pressure_data_timeout.restart();

    if (base::isUnset(m_safe_working_pressure.toPa())) {
        startSonar();
        return true;
    }

    while (!m_pressure_data_timeout.elapsed()) {
        base::samples::Pressure pressure;
        auto flow = _pressure.read(pressure);
        if (flow != RTT::NewData) {
            continue;
        }

        m_pressure_data_timeout.restart();
        bool safe = safeToWork();
        m_init_unsafe_working_pressure = !safe;

        if (safe) {
            startSonar();
        }
        return true;
    }
    m_init_unsafe_working_pressure = true;
    return true;
}

void Task::startSonar()
{
    m_recover_minimum_time.restart();
    m_driver->fireSonar(m_fire_config, m_update_rate);
}

void Task::stopSonar()
{
    m_recover_minimum_time.restart();
    m_driver->fireSonar(m_fire_config, UPDATE_STANDBY);
}

void Task::updateHook()
{
    // RTT does not let us transition from startHook to a runtime error state
    // directly. This flag is used to pass the information
    if (m_init_unsafe_working_pressure) {
        m_init_unsafe_working_pressure = false;
        stopSonar();
        return error(UNSAFE_WORKING_PRESSURE);
    }

    if (!safeToWork()) {
        stopSonar();
        return error(UNSAFE_WORKING_PRESSURE);
    }

    // Call updateHook after the pressure safety checks to make sure it is already done
    // in processIO
    TaskBase::updateHook();
}

void Task::processIO()
{
    auto sonar = m_driver->processOne();
    if (!sonar) {
        return;
    }

    _sonar.write(*sonar);
}

void Task::errorHook()
{
    TaskBase::errorHook();

    // The sonar does not support receiving a fire command while one is already
    // in flight. The recover_minimum_time here is meant to make sure we do not
    // have any in flight anymore.
    if (safeToWork() && m_recover_minimum_time.elapsed()) {
        startSonar();
        recover();
    }
}

bool Task::safeToWork()
{
    if (base::isUnset(m_safe_working_pressure.toPa())) {
        return true;
    }

    base::samples::Pressure pressure;
    auto flow = _pressure.read(pressure);
    if (flow == RTT::NoData ||
        (flow == RTT::OldData && m_pressure_data_timeout.elapsed())) {
        return false;
    }

    if (flow == RTT::NewData) {
        m_pressure_data_timeout.restart();
    }

    return pressure.toPa() > m_safe_working_pressure.toPa();
}

void Task::stopHook()
{
    TaskBase::stopHook();
    stopSonar();
}

void Task::cleanupHook()
{
    TaskBase::cleanupHook();
}
