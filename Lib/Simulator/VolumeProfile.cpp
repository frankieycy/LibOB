#ifndef VOLUME_PROFILE_CPP
#define VOLUME_PROFILE_CPP
#include "Utils/Utils.hpp"
#include "Simulator/VolumeProfile.hpp"

namespace Simulator {
std::string toString(const VolumeInterpolationStrategy strategy) {
    switch (strategy) {
        case VolumeInterpolationStrategy::FLAT:                 return "Flat";
        case VolumeInterpolationStrategy::LINEAR:               return "Linear";
        case VolumeInterpolationStrategy::EXPONENTIAL:          return "Exponential";
        case VolumeInterpolationStrategy::POWER_LAW:            return "PowerLaw";
        case VolumeInterpolationStrategy::CUSTOM_INPUT:         return "CustomInput";
        case VolumeInterpolationStrategy::PIECEWISE_CONSTANT:   return "PiecewiseConstant";
        case VolumeInterpolationStrategy::PIECEWISE_LINEAR:     return "PiecewiseLinear";
        default:                                                return "None";
    }
}

std::string toString(const VolumeExtrapolationStrategy strategy) {
    switch (strategy) {
        case VolumeExtrapolationStrategy::FLAT:         return "Flat";
        case VolumeExtrapolationStrategy::LINEAR:       return "Linear";
        case VolumeExtrapolationStrategy::EXPONENTIAL:  return "Exponential";
        case VolumeExtrapolationStrategy::POWER_LAW:    return "PowerLaw";
        default:                                        return "None";
    }
}

std::ostream& operator<<(std::ostream& out, const VolumeInterpolationStrategy strategy) { return out << toString(strategy); }

std::ostream& operator<<(std::ostream& out, const VolumeExtrapolationStrategy strategy) { return out << toString(strategy); }

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
