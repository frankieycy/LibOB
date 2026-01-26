#ifndef VOLUME_PROFILE_CPP
#define VOLUME_PROFILE_CPP
#include "Utils/Utils.hpp"
#include "Simulator/VolumeProfile.hpp"

namespace Simulator {
uint32_t LinearVolumeInterpolator::volumeAt(const uint32_t dist) const {
    if (dist < getInterpDistanceStart())
        return 0;
    else if (dist == getInterpDistanceStart())
        return myVolumeStart;
    else if (dist >= getInterpDistanceEnd())
        return myVolumeEnd;
    return static_cast<uint32_t>(std::round(myVolumeStart + myVolumeSlope * (dist - getInterpDistanceStart())));
}

uint32_t PiecewiseConstantVolumeInterpolator::volumeAt(uint32_t dist) const {
    auto hi = myKnots.lower_bound(dist);
    if (hi == myKnots.begin())
        return dist < hi->second ? 0 : hi->second;
    else if (hi == myKnots.end())
        return std::prev(hi)->second;
    auto lo = std::prev(hi);
    return lo->second;
}

uint32_t PiecewiseLinearVolumeInterpolator::volumeAt(uint32_t dist) const {
    auto hi = myKnots.lower_bound(dist);
    if (hi == myKnots.begin())
        return dist < hi->second ? 0 : hi->second;
    else if (hi == myKnots.end())
        return std::prev(hi)->second;
    auto lo = std::prev(hi);
    double slope = double(hi->second - lo->second) / double(hi->first - lo->first);
    return static_cast<uint32_t>(lo->second + slope * (dist - lo->first));
}
}

#endif
