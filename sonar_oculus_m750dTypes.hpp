#ifndef sonar_oculus_m750d_TYPES_HPP
#define sonar_oculus_m750d_TYPES_HPP

#include <cstdint>

namespace sonar_oculus_m750d {
    enum Mode {
        // is flexi mode, needs full fire message
        MODE_0,
        // 750kHz, 256 beams, 120º aperture
        MODE_1,
        // 1.5MHz, 128 beams, 60º aperture
        MODE_2,
        // 1.8MHz, 256 beams, 50º aperture
        MODE_3
    };
    struct Configuration
    {
        Mode mode;
        double range;
        double gain;
        double speed_of_sound;
        double salinity;
        bool gain_assist;
        uint8_t gamma;
        uint8_t net_speed_limit;
    };
    
}

#endif
