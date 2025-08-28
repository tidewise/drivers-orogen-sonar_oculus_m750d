/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <memory>

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
    if (!TaskBase::configureHook())
        return false;
    m_config = _configuration.get();
    m_client = std::make_unique<OsClientCtrl>(m_config.ip_addr, m_config.netmask);
    m_client->Connect();
    return true;
}
bool Task::startHook()
{
    if (!TaskBase::startHook())
        return false;
    m_client->Fire(m_config.mode,
        m_config.range,
        m_config.gain,
        m_config.speed_of_sound,
        m_config.salinity,
        m_config.gain_assist,
        m_config.gamma,
        m_config.net_speed_limit);
    m_client->m_readData.socketConnect();
    return true;
}
void Task::updateHook()
{
    TaskBase::updateHook();
    m_client->m_readData.singleRun();
    if (m_client->hasNewImage()) {
        auto sonar_data = m_client->m_readData.m_osBuffer->getSonarData();
    }
    base::samples::Sonar sonar;
    // TODO: fill sonar
    _sonar.write(sonar);
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
