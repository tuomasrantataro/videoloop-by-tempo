
#include "pulseaudiowatcher.h"

#include <pulse/pulseaudio.h>

// Callback forward declatations for PulseAudio C API use
void pa_state_cb(pa_context *c, void *userdata);
void pa_sinkinputlist_cb(pa_context *c, const pa_sink_input_info *l, int eol, void *userdata);
void pa_clientlist_cb(pa_context *c, const pa_client_info *l, int eol, void *userdata);



PulseaudioWatcher::PulseaudioWatcher(QString targetProgram, QStringList ignorePrograms) :
    m_targetProgram(targetProgram),
    m_ignorePrograms(ignorePrograms)
{
    m_checkPulseSinkInputs = new QTimer(this);
    connect(m_checkPulseSinkInputs, &QTimer::timeout, this, &PulseaudioWatcher::checkPulseAudioData);
}

PulseaudioWatcher::~PulseaudioWatcher()
{
    
}

void PulseaudioWatcher::startPolling(int interval)
{
    m_checkPulseSinkInputs->start(interval);
}

void PulseaudioWatcher::stopPolling()
{
    m_checkPulseSinkInputs->stop();
}

void PulseaudioWatcher::checkPulseAudioData()
{
    pa_clientlist_c_t pa_client_list;
    pa_client_list.clients = NULL;      // Will be reallocated in pa_clientlist_cb
    pa_sinkinputlist_c_t pa_sinkinput_list;
    pa_sinkinput_list.inputs = NULL;    // Will be reallocated in pa_sinkinputlist_cb

    if (pa_get_clients(&pa_sinkinput_list, &pa_client_list) < 0) {
        qWarning() << "Failed to get sink inputs and/or their clients";
        free(pa_client_list.clients);
        free(pa_sinkinput_list.inputs);
        return;
    }

    QMap<int, QString> pa_clients;
    QStringList audioOutputers;

    // Generate a dictionary mapping PulseAudio's client indexes to their names
    for (size_t i = 0; i < pa_client_list.size; i++) {
        pa_client_c item = pa_client_list.clients[i];
        pa_clients.insert(item.index, QString(item.name));
    }

    // Generate list of PulseAudio's clients which may output audio at this moment
    for (size_t i = 0; i < pa_sinkinput_list.size; i++) {
        pa_sinkinput_c_t item = pa_sinkinput_list.inputs[i];

        // If the sink-input can produce audio output
        if (!item.corked && !item.muted) {
            // Add the client (application) name to vector
            audioOutputers.push_back(pa_clients[item.client]);

        }
    }

    for (auto item : m_ignorePrograms) {
        audioOutputers.removeAll(item);
    }

    // If there is other software outputting audio or the wanted application isn't,
    // send signal to indicate that the audio data is not what is wanted
    if (audioOutputers.size() != 1) {
        QString error = QString::number(audioOutputers.size()) + " PulseAudio sink-inputs found: ";
        error.append(audioOutputers.join(", "));
        emit invalidateData(error);
    }
    else if (!audioOutputers.contains(m_targetProgram)) {
        QString error = "Target program " + m_targetProgram + " not found in PulseAudio sink-inputs.";
        emit invalidateData(error);
    }



    // Free the memory allocated in callbacks
    free(pa_client_list.clients);
    free(pa_sinkinput_list.inputs);
}

int PulseaudioWatcher::pa_get_clients(pa_sinkinputlist_c_t *sinkinputs, pa_clientlist_c_t *clients) {

    pa_mainloop *pa_ml;
    pa_mainloop_api *pa_mlapi;
    pa_operation *pa_op;
    pa_context *pa_ctx;

    int state = 0;
    int pa_ready = 0;

    pa_ml = pa_mainloop_new();
    pa_mlapi = pa_mainloop_get_api(pa_ml);
    pa_ctx = pa_context_new(pa_mlapi, "probe sink-inputs");

    pa_context_connect(pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

    pa_context_set_state_callback(pa_ctx, pa_state_cb, &pa_ready);

    // Loop forever until we get the data wanted
    for (;;) {
        // PulseAudio not yet ready
        if (pa_ready == 0) {
            pa_mainloop_iterate(pa_ml, 1, NULL);
            continue;
        }

        // PulseAudio connection failed or terminated
        if (pa_ready == 2) {
            pa_context_disconnect(pa_ctx);
            pa_context_unref(pa_ctx);
            pa_mainloop_free(pa_ml);
            return -1;
        }

        switch (state) {
            case 0:
                // Get all sink-inputs
                pa_op = pa_context_get_sink_input_info_list(
                    pa_ctx,
                    pa_sinkinputlist_cb,
                    sinkinputs
                    );
                state++;
                break;
            case 1:
                // Wait for previous operation to finish
                if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {
                    pa_operation_unref(pa_op);

                    // Get all PulseAudio server's clients
                    pa_op = pa_context_get_client_info_list(
                        pa_ctx,
                        pa_clientlist_cb,
                        clients
                    );
                state++;
                    
                }
                break;
            case 2:
                // Got all the data wanted, free the resources
                if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {
                    pa_operation_unref(pa_op);
                    pa_context_disconnect(pa_ctx);
                    pa_context_unref(pa_ctx);
                    pa_mainloop_free(pa_ml);
                    return 0;
                }
                break;
            default:
                // We should never see this state
                qDebug() << "in state: " << state;
                return -1;
        }

        pa_mainloop_iterate(pa_ml, 1, NULL);
    }
}

void pa_state_cb(pa_context *c, void *userdata) {
        pa_context_state_t state;

        int *pa_ready = (int*)userdata;

        state = pa_context_get_state(c);
        switch  (state) {
                // There are just here for reference
                case PA_CONTEXT_UNCONNECTED:
                case PA_CONTEXT_CONNECTING:
                case PA_CONTEXT_AUTHORIZING:
                case PA_CONTEXT_SETTING_NAME:
                default:
                        break;
                case PA_CONTEXT_FAILED:
                case PA_CONTEXT_TERMINATED:
                        *pa_ready = 2;
                        break;
                case PA_CONTEXT_READY:
                        *pa_ready = 1;
                        break;
        }
}

void pa_sinkinputlist_cb(pa_context *c, const pa_sink_input_info *l, int eol, void *userdata) {
    pa_sinkinputlist_c_t *pa_sinkinput_list = (pa_sinkinputlist_c_t*)userdata;

    if (eol > 0) {
        return;
    }

    // Allocate memory for a new item in inputs array
    pa_sinkinput_list->inputs = (pa_sinkinput_c_t*)
                        realloc(pa_sinkinput_list->inputs,
                        (pa_sinkinput_list->size+1) * sizeof(pa_sinkinput_c_t)
                        );
    pa_sinkinput_list->size++;

    // Copy data to the newly allocated memory
    int idx = pa_sinkinput_list->size - 1;
    strncpy(pa_sinkinput_list->inputs[idx].name, l->name, 255);
    pa_sinkinput_list->inputs[idx].client = l->client;
    pa_sinkinput_list->inputs[idx].corked = l->corked;
    pa_sinkinput_list->inputs[idx].muted = l->mute;
}

void pa_clientlist_cb(pa_context *c, const pa_client_info *l, int eol, void *userdata) {
    pa_clientlist_c_t *pa_client_list = (pa_clientlist_c_t*)userdata;

    if (eol > 0) {
        return;
    }

    pa_client_list->clients = (pa_client_c_t*)
                        realloc(pa_client_list->clients,
                        (pa_client_list->size+1) * sizeof(pa_client_c_t)
                        );
    pa_client_list->size++;

    int idx = pa_client_list->size - 1;
    strncpy(pa_client_list->clients[idx].name, l->name, 255);
    pa_client_list->clients[idx].index = l->index;
}