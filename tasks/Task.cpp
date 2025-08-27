/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"
#include <memory>
#include <rtt/extras/ReadOnlyPointer.hpp>

using namespace sonar_oculus_m750d;
using namespace base::samples::frame;

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
    return true;
}
void Task::updateHook()
{
    TaskBase::updateHook();
    m_client->m_readData.run();
    auto image_and_range = m_client->m_readData.m_osBuffer->getImageAndRange();
    std::unique_ptr<Frame> frame(new Frame(image_and_range.width,
        image_and_range.height,
        8U,
        base::samples::frame::frame_mode_t::MODE_GRAYSCALE,
        0,
        0));
    frame->setImage(image_and_range.data, image_and_range.data_size);
    frame->setStatus(STATUS_VALID);
    frame->received_time = base::Time::now();
    RTT::extras::ReadOnlyPointer out_frame_ptr(frame.release());
    _out_frame.write(out_frame_ptr);
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
