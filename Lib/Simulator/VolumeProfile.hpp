#ifndef VOLUME_PROFILE_HPP
#define VOLUME_PROFILE_HPP
#include "Utils/Utils.hpp"

namespace Simulator {
using namespace Utils;

enum class VolumeInterpolationStrategy { FLAT, LINEAR, EXPONENTIAL, POWER_LAW, NULL_VOLUME_INTERPOLATION_STRATEGY };
enum class VolumeExtrapolationStrategy { FLAT, LINEAR, EXPONENTIAL, POWER_LAW, NULL_VOLUME_EXTRAPOLATION_STRATEGY };

class IVolumeInterpolator {
public:
    IVolumeInterpolator(const uint32_t interpDistStart, const uint32_t interpDistEnd) :
        myInterpDistanceStart(interpDistStart), myInterpDistanceEnd(interpDistEnd) {
        if (interpDistEnd < interpDistStart)
            Error::LIB_THROW("[IVolumeInterpolator] Interpolation end distance must be greater than start distance.");
    }
    virtual ~IVolumeInterpolator() = default;
    uint32_t getInterpDistanceStart() const { return myInterpDistanceStart; }
    uint32_t getInterpDistanceEnd() const { return myInterpDistanceEnd; }
    uint32_t operator()(const uint32_t dist) const { return volumeAt(dist); }
    virtual uint32_t volumeAt(const uint32_t dist) const noexcept = 0;
private:
    uint32_t myInterpDistanceStart;
    uint32_t myInterpDistanceEnd;
};

class FlatVolumeInterpolator final : public IVolumeInterpolator {
public:
    FlatVolumeInterpolator(const uint32_t interpDistStart, const uint32_t interpDistEnd, const uint32_t flatVolume) :
        IVolumeInterpolator(interpDistStart, interpDistEnd), myFlatVolume(flatVolume) {}
    uint32_t volumeAt(const uint32_t /* dist */) const noexcept override {
        return myFlatVolume;
    }
private:
    uint32_t myFlatVolume;
};

class LinearVolumeInterpolator final : public IVolumeInterpolator {
    LinearVolumeInterpolator(const uint32_t interpDistStart, const uint32_t interpDistEnd, const uint32_t volumeStart, const uint32_t volumeEnd) :
        IVolumeInterpolator(interpDistStart, interpDistEnd), myVolumeStart(volumeStart), myVolumeEnd(volumeEnd) {
        myVolumeSlope = double(myVolumeEnd - myVolumeStart) / double(interpDistEnd - interpDistStart);
    }
    uint32_t volumeAt(const uint32_t dist) const noexcept override;
private:
    double myVolumeSlope;
    uint32_t myVolumeStart;
    uint32_t myVolumeEnd;
};

class CustomInputVolumeInterpolator final : public IVolumeInterpolator {
public:
    CustomInputVolumeInterpolator(std::map<uint32_t, uint32_t> inputVolumes) :
        IVolumeInterpolator(inputVolumes.begin()->first, inputVolumes.rbegin()->first), myInputVolumes(std::move(inputVolumes)) {}
    uint32_t volumeAt(const uint32_t dist) const noexcept override {
        return myInputVolumes.find(dist) != myInputVolumes.end() ? myInputVolumes.at(dist) : 0;
    }
private:
    std::map<uint32_t, uint32_t> myInputVolumes;
};

class PiecewiseConstantVolumeInterpolator final : public IVolumeInterpolator {
public:
    PiecewiseConstantVolumeInterpolator(std::map<uint32_t, uint32_t> knots) :
        IVolumeInterpolator(knots.begin()->first, knots.rbegin()->first), myKnots(std::move(knots)) {}
    uint32_t volumeAt(uint32_t dist) const noexcept override;
private:
    std::map<uint32_t, uint32_t> myKnots;
};

class PiecewiseLinearVolumeInterpolator final : public IVolumeInterpolator {
public:
    PiecewiseLinearVolumeInterpolator(std::map<uint32_t, uint32_t> knots) :
        IVolumeInterpolator(knots.begin()->first, knots.rbegin()->first), myKnots(std::move(knots)) {}
    uint32_t volumeAt(uint32_t dist) const noexcept override;
private:
    std::map<uint32_t, uint32_t> myKnots;
};

class IVolumeExtrapolator {
public:
    IVolumeExtrapolator(const uint32_t extrapDist) : myExtrapDistance(extrapDist) {}
    virtual ~IVolumeExtrapolator() = default;
    uint32_t getExtrapDistance() const { return myExtrapDistance; }
    uint32_t operator()(const uint32_t dist) const { return volumeAt(dist); }
    virtual uint32_t volumeAt(const uint32_t dist) const noexcept = 0;
private:
    uint32_t myExtrapDistance;
};

class FlatVolumeExtrapolator final : public IVolumeExtrapolator {
public:
    FlatVolumeExtrapolator(const uint32_t extrapDist, const uint32_t flatVolume) :
        IVolumeExtrapolator(extrapDist), myFlatVolume(flatVolume) {}
    uint32_t volumeAt(const uint32_t /* dist */) const noexcept override {
        return myFlatVolume;
    }
private:
    uint32_t myFlatVolume;
};

class VolumeProfile {
public:
    VolumeProfile(std::unique_ptr<IVolumeInterpolator> interp, std::unique_ptr<IVolumeExtrapolator> extrap, const uint32_t interpEnd) :
        myInterpolator(std::move(interp)), myExtrapolator(std::move(extrap)), myInterpEnd(interpEnd) {}
    uint32_t operator()(const uint32_t dist) const {
        return dist <= myInterpEnd ? (*myInterpolator)(dist) : (*myExtrapolator)(dist);
    }
private:
    std::unique_ptr<IVolumeInterpolator> myInterpolator;
    std::unique_ptr<IVolumeExtrapolator> myExtrapolator;
    uint32_t myInterpEnd;
};
}

#endif
