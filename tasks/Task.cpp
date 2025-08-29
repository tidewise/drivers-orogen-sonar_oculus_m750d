/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <memory>

using namespace sonar_oculus_m750d;
using namespace base::samples;

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
    m_timeout = _timeout.get();
    m_client = std::make_unique<OsClientCtrl>(m_config.ip_addr, m_config.netmask);
    m_client->Connect();
    return true;
}
bool Task::startHook()
{
    if (!TaskBase::startHook())
        return false;
    m_client->m_readData.socketConnect();
    return true;
}

std::vector<float> toBeamFirst(uchar* bin_first, uint16_t beam_count, uint16_t bin_count)
{
    std::vector<float> beam_first(beam_count * bin_count);
    for (uint16_t b = 0; b < beam_count; b++) {
        for (uint16_t r = 0; r < bin_count; r++) {
            beam_first[b * bin_count + r] =
                static_cast<float>(bin_first[r * beam_count + b]);
        }
    }
    return beam_first;
}

base::samples::Sonar fillSonar(base::Time const& time, SonarData const& sonar_data)
{
    base::samples::Sonar sonar(time,
        base::Time::fromSeconds(0),
        sonar_data.bin_count,
        base::Angle::fromDeg(20),
        base::Angle::fromDeg(20),
        sonar_data.beam_count,
        false);
    auto beam_first =
        toBeamFirst(sonar_data.data, sonar_data.beam_count, sonar_data.bin_count);
    sonar.setBeamBins(0, beam_first);
    return sonar;
}

void Task::updateHook()
{
    TaskBase::updateHook();
    fireSonar();
    m_client->m_readData.singleRun();
    if (m_client->hasNewImage()) {
        auto sonar_data = m_client->m_readData.m_osBuffer->getSonarData();
        base::samples::Sonar sonar = fillSonar(base::Time::now(), sonar_data);
        _sonar.write(sonar);
    }
}

void Task::fireSonar()
{
    m_client->Fire(m_config.mode,
        m_config.range,
        m_config.gain,
        m_config.speed_of_sound,
        m_config.salinity,
        m_config.gain_assist,
        m_config.gamma,
        m_config.net_speed_limit);
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
