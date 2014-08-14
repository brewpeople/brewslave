#pragma once

namespace brewmeister
{
    enum COMMAND
    {
        READ = 0xf0,
        WRITE = 0xf1
    };

    enum INSTRUMENT
    {
        TEMP = 0xf1,
        HEAT = 0xf2,
        STIR = 0xf3,
        HEAT_CONTROL = 0xf4
    };

}



