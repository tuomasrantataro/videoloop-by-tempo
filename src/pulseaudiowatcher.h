#ifndef PULSEAUDIOWATCHER_H
#define PULSEAUDIOWATCHER_H

#include <stdio.h>

#include <QtCore>

#include <pulse/pulseaudio.h>

// Structures for getting data from pulseaudio C API
typedef struct pa_sinkinput_c {
    char name[256];
    uint32_t client;
    int corked;
    int muted;
} pa_sinkinput_c_t;

typedef struct pa_sinkinputlist_c {
    pa_sinkinput_c_t* inputs = NULL;
    size_t size = 0;
} pa_sinkinputlist_c_t;

typedef struct pa_client_c {
    uint32_t index;
    char name[256];
} pa_client_c_t;

typedef struct pa_clientlist_c {
    pa_client_c_t* clients = NULL;
    size_t size = 0;
} pa_clientlist_c_t;


class PulseaudioWatcher : public QObject
{
    Q_OBJECT
public:
    PulseaudioWatcher(QString targetProgram, QStringList ignorePrograms);
    ~PulseaudioWatcher();

public:

    void startPolling(int interval_ms);
    void stopPolling();

signals:
    void invalidateData(QString reason);

private:
    QString m_targetProgram;
    QStringList m_ignorePrograms;
    QTimer *m_checkPulseSinkInputs;

    // For PulseAudio API use
    int pa_get_clients(pa_sinkinputlist_c_t *sinkinputs, pa_clientlist_c_t *clients);

private slots:
    void checkPulseAudioData();
};

// Callbacks for PulseAudio C API use
void pa_state_cb(pa_context *c, void *userdata);
void pa_sinkinputlist_cb(pa_context *c, const pa_sink_input_info *l, int eol, void *userdata);
void pa_clientlist_cb(pa_context *c, const pa_client_info *l, int eol, void *userdata);

#endif