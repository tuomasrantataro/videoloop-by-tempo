#ifndef TYPES_H
#define TYPES_H

#include <vector>

namespace MyTypes {

typedef std::vector<uint8_t> AudioData;

typedef struct TempoData {
    std::vector<float> ticks; // beat ticks, [s] from data start
    float confidence; // confidence in beat detection. >3.5 is excellent
    float BPM;  // bpm estimate
    float peak1;
    float power1;
    float spread1;
    float peak2;
    float power2;
    float spread2;
    std::vector<float> BPMEstimates; // array of bpm estimates starting from most likely
    std::vector<float> BPMIntervals; // list of beats intervals [s]
} TempoData;

typedef struct TrackData {
    QString trackId = "";
    float BPM = 0.0;
    float confidence = 0.0;
    float peak1, power1, spread1 = 0.0; // first histogram peak
    float peak2, power2, spread2 = 0.0; // second histogram peak
    QString artist = "";
    QString title = "";

    TrackData() {};
    TrackData(const TempoData &data, const QString &trackId_, const QString &artist_, const QString &track_) {
        BPM = data.BPM;
        confidence = data.confidence;
        peak1 = data.peak1;
        power1 = data.power1;
        spread1 = data.spread1;
        peak2 = data.peak2;
        power2 = data.power2;
        spread2 = data.spread2;
        trackId = trackId_;
        artist = artist_;
        title = track_;
    }
} TrackData;

enum AudioBufferType {
    rolling,
    track
};

};  //namespace

Q_DECLARE_METATYPE(MyTypes::TempoData)
Q_DECLARE_METATYPE(MyTypes::AudioData)


#endif