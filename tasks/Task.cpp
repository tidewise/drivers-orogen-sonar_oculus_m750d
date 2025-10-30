/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <iodrivers_base/ConfigureGuard.hpp>
#include <sonar_oculus_m750d/Driver.hpp>
#include <chrono>
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
    return true;
}

bool Task::startHook()
{
    if (!TaskBase::startHook())
        return false;

    m_pressure_data_timeout.restart();
    m_init_unsafe_working_pressure = false;
    while (!safeToWork()) {
        if (m_pressure_data_timeout.elapsed()) {
            // RTT does not let us transition from startHook to a runtime error
            // state directly. This flag is used to pass the information
            // to updateHook
            m_init_unsafe_working_pressure = true;
            return true;
        }

        this_thread::sleep_for(100ms);
    }

    fireSonar();
    return true;
}

void Task::fireSonar() {
    m_recover_minimum_time.restart();
    m_driver->fireSonar(m_fire_config);
}

void Task::updateHook()
{
    // RTT does not let us transition from startHook to a runtime error state
    // directly. This flag is used to pass the information
    if (m_init_unsafe_working_pressure) {
        m_init_unsafe_working_pressure = false;
        return error(UNSAFE_WORKING_PRESSURE);
    }

    // Call updateHook after the check for m_init_unsafe_working_pressure to not
    // call processIO
    TaskBase::updateHook();
}

void Task::processIO()
{
    auto sonar = m_driver->processOne();
    if (sonar) {
        _sonar.write(*sonar);

        if (!safeToWork()) {
            return error(UNSAFE_WORKING_PRESSURE);
        }
        fireSonar();
    }
}

void Task::errorHook()
{
    TaskBase::errorHook();

    // Make sure we ignore any pending I/O or the FD activity will call us
    // in an inifinite loop
    m_driver->clear();

    // The sonar does not support receiving a fire command while one is already
    // in flight. The recover_minimum_time here is meant to make sure we do not
    // have any in flight anymore.
    if (safeToWork() && m_recover_minimum_time.elapsed()) {
        fireSonar();
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
}

void Task::cleanupHook()
{
    TaskBase::cleanupHook();
}
