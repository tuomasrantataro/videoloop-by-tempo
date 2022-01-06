#ifndef TYPES_H
#define TYPES_H

#include <vector>

namespace MyTypes {

typedef std::vector<uint8_t> AudioData;

typedef struct TempoData {
    std::vector<float> ticks; // beat ticks, [s] from data start
    float confidence; // confidence in beat detection. >3.5 is excellent
    float BPM;  // bpm estimate
    std::vector<float> BPMEstimates; // array of bpm estimates starting from most likely
    std::vector<float> BPMIntervals; // list of beats intervals [s]
} TempoData;

enum AudioBufferType {
    rolling,
    track
};

};  //namespace

Q_DECLARE_METATYPE(MyTypes::TempoData)
Q_DECLARE_METATYPE(MyTypes::AudioData)


#endif