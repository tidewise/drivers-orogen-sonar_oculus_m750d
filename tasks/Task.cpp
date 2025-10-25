/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <iodrivers_base/ConfigureGuard.hpp>
#include <sonar_oculus_m750d/Driver.hpp>

using namespace sonar_oculus_m750d;

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
    m_pressure_data_timeout_period = _pressure_data_timeout.get();
    return true;
}

bool Task::startHook()
{
    if (!TaskBase::startHook())
        return false;

    m_safe_to_work = base::isNaN(m_safe_working_pressure.toPa());
    m_pressure_data_timeout = base::Timeout(m_pressure_data_timeout_period);

    safeFireSonar();

    return true;
}

void Task::safeFireSonar()
{
    if (!m_safe_to_work) {
        return;
    }
    m_driver->fireSonar(m_fire_config);
}

void Task::updateHook()
{
    TaskBase::updateHook();

    if (base::isNaN(m_safe_working_pressure.toPa())) {
        return;
    }

    if(!m_safe_to_work && safeToWork()) { // unsafe -> safe transition
        m_safe_to_work = true;
        safeFireSonar();
    }

    m_safe_to_work = safeToWork();
    if (!m_safe_to_work) {
        return error(UNSAFE_WORKING_PRESSURE);
    }
}

bool Task::safeToWork()
{
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

void Task::error(States error_state)
{
    if (state() != error_state) {
        TaskBase::error(error_state);
    }
}

void Task::errorHook()
{
    TaskBase::errorHook();
    if (state() == UNSAFE_WORKING_PRESSURE && safeToWork()) {
        recover();
    }
}

void Task::stopHook()
{
    TaskBase::stopHook();
}

void Task::cleanupHook()
{
    TaskBase::cleanupHook();
}

void Task::processIO()
{
    auto sonar = m_driver->processOne();
    if (sonar) {
        _sonar.write(*sonar);
        safeFireSonar();
    }
}