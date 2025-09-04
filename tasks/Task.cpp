/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <iodrivers_base/ConfigureGuard.hpp>
#include <sonar_oculus_m750d/Driver.hpp>

using namespace sonar_oculus_m750d;
using namespace iodrivers_base;

sonar_oculus_m750d::Task::Task(std::string const& name)
    : TaskBase(name)
{
}

sonar_oculus_m750d::Task::~Task()
{
}

bool sonar_oculus_m750d::Task::configureHook()
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
    return true;
}

bool sonar_oculus_m750d::Task::startHook()
{
    if (!TaskBase::startHook())
        return false;
    sonar_oculus_m750d::Driver* driver =
        static_cast<sonar_oculus_m750d::Driver*>(mDriver);
    driver->fireSonar(m_fire_config);
    return true;
}

void sonar_oculus_m750d::Task::updateHook()
{
    TaskBase::updateHook();
}

void sonar_oculus_m750d::Task::errorHook()
{
    TaskBase::errorHook();
}

void sonar_oculus_m750d::Task::stopHook()
{
    TaskBase::stopHook();
}

void sonar_oculus_m750d::Task::cleanupHook()
{
    TaskBase::cleanupHook();
}

void sonar_oculus_m750d::Task::processIO()
{
    sonar_oculus_m750d::Driver* driver =
        static_cast<sonar_oculus_m750d::Driver*>(mDriver);
    auto sonar = driver->processOne();
    if (sonar) {
        _sonar.write(*sonar);
    }
    driver->fireSonar(m_fire_config);
}