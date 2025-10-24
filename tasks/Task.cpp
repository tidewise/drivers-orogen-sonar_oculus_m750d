/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <iodrivers_base/ConfigureGuard.hpp>
#include <sonar_oculus_m750d/Driver.hpp>

using namespace sonar_oculus_m750d;

Task::Task(std::string const& name)
    : TaskBase(name)
{
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

    return true;
}

bool Task::startHook()
{
    if (!TaskBase::startHook())
        return false;
    m_driver->fireSonar(m_fire_config);
    return true;
}

void Task::updateHook()
{
    TaskBase::updateHook();
}

void Task::errorHook()
{
    TaskBase::errorHook();
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
        m_driver->fireSonar(m_fire_config);
    }
}