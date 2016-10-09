#include "sound.h"
#include "math.h"
//#include "iostream"

BufferDesc DEFAULT_BUFF_DESC = {44100, 8, 1, 44100*1.5};

void init_wave(WAVEFORMATEX &wfx, BufferDesc &desc) {
    memset(&wfx, 0, sizeof(WAVEFORMATEX));
    wfx.wFormatTag          = WAVE_FORMAT_PCM;
    wfx.nChannels           = desc.Channels;
    wfx.nSamplesPerSec      = desc.SamplesPerSec;
    wfx.wBitsPerSample      = desc.BitsPerSample;
    wfx.nBlockAlign         = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec     = wfx.nSamplesPerSec * wfx.nBlockAlign;
}

// --------------------------------------------------------------------- DXSound ----//
DXSound::DXSound(HWND hwnd) : sound(0) {
    HRESULT ret = DirectSoundCreate8(NULL, &sound, NULL);
    if (FAILED(ret)) {
        throw "create dsound failed.";
    }

    if (hwnd == 0) {
        hwnd = GetConsoleWindow();
    }

    ret = sound->SetCooperativeLevel(hwnd, DSSCL_NORMAL);
    if (FAILED(ret)) {
        sound->Release();
        sound = NULL;
        throw "not set level.";
    }
}

DXSound::~DXSound() {
    if (sound) {
        sound->Release();
        sound = NULL;
    }
}

// ------------------------------------------------------------------ DXChannel ----//
DXChannel::DXChannel(DXSound &ds, BufferDesc desc) : dxs(ds), bdesc(desc) {
    init_desc(desc);

    HRESULT ret = ds.sound->CreateSoundBuffer(&bufdesc, &buffer, NULL);

    if (FAILED(ret)) {
        printf("Not create dxbuffer 0x%x", ret);
        throw ret;
    }
}

void DXChannel::init_desc(BufferDesc &desc) {
    init_wave(wfx, desc);

    memset(&bufdesc, 0, sizeof(DSBUFFERDESC));
    bufdesc.dwSize          = sizeof(DSBUFFERDESC);
    //DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
    bufdesc.dwFlags         = DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME; 
    bufdesc.dwBufferBytes   = desc.BufferBytes ? desc.BufferBytes : 3 * wfx.nAvgBytesPerSec;
    bufdesc.lpwfxFormat     = &wfx;
}

void DXChannel::setFormat(BufferDesc &d) {
    init_wave(wfx, d);
    buffer->SetFormat(&wfx);
}

void DXChannel::free() {
    if (buffer) {
        buffer->Release();
        buffer = NULL;
    }
}

DXChannel::~DXChannel() {
    free();
}

void DXChannel::lockBuffer(CreateSample &cs) {
    if (DS_OK != buffer->Lock(
        0, // Offset at which to start lock.
        0, // Size of lock; ignored because of flag.
        &cs.lpvWrite, // Gets address of first part of lock.
        &cs.dwLength, // Gets size of first part of lock.
        NULL, // Address of wraparound not needed.
        NULL, // Size of wraparound not needed.
        DSBLOCK_ENTIREBUFFER) ) {

        //memcpy(lpvWrite, pbData, dwLength);
        throw "lock buffer fail.";
    }
    cs.dwLength /= sizeof(NoteType);
}

void DXChannel::freeBuffer(CreateSample &cs) {
    buffer->Unlock(
        cs.lpvWrite, // Address of lock start.
        cs.dwLength, // Size of lock.
        NULL, // No wraparound portion.
        0); // No wraparound size.
}

void DXChannel::play(DWORD flag) {
    // buffer->SetCurrentPosition(0);
    buffer->Play(0, 0, flag);
}

void DXChannel::stop() {
    buffer->Stop();
}

void DXChannel::setFrequency(DWORD f) {
    buffer->SetFrequency(f);
}

void DXChannel::setVolume(double per) {
    buffer->SetVolume(int((-DSBVOLUME_MIN) * per) + DSBVOLUME_MIN);
}

// ------------------------------------------------------------------- ChannelPool ----//
ChannelPool::ChannelPool(DXSound &b, int s) : base(b), size(s), chs_idx(0) {
    chs = new DXChannel*[size];
    for (int i=0; i<size; ++i) {
        chs[i] = new DXChannel(base);
    }
}

ChannelPool::~ChannelPool() {
    for (int i=0; i<size; ++i) {
        delete chs[i];
    }
    delete [] chs;
    chs = NULL;
}

void ChannelPool::stop() {
    for (int i=0; i<size; ++i) {
        chs[i]->stop();
    }
}

void noise(CreateSample &cs) {
    for (int i=0; i<cs.dwLength; ++i) {
        cs[i] = rand();
    }
}

void osc::operator()(CreateSample &cs) {
    int z = (cs.desc.nSamplesPerSec / hz);
    double PI_2 = M_PI * 2;

    for (int i=0; i<cs.dwLength; ++i) {
        cs[i] = NoteType(( sin(PI_2 * (i%z) / z) + 1 ) * cs.maxval);
    }
}

void triangle::operator()(CreateSample &cs) {
    int z = (cs.desc.nSamplesPerSec / hz);
    double x;

    for (int i=0; i<cs.dwLength; ++i) {
        x = double(i % z) / z;
        if (x > 0.5) x -= 0.5;
        cs[i] = cs.maxval * 2 * x;
    }
}

void square::operator()(CreateSample &cs) {
    int z = (cs.desc.nSamplesPerSec / hz);
    int b = z/2;

    for (int i=0; i<cs.dwLength; ++i) {
        cs[i] = (i%z > b) ? cs.maxval: 1;
        // printf("S %d %d %d\n", i, cs[i], z);
    }
}

CreateSample::CreateSample(DXChannel &ds, void *x)
    : desc(ds.getWfx()), xdata(x),
      lpvWrite(0), dwLength(0),
      maxval((1 << ds.getWfx().wBitsPerSample) -1) {
}

CreateSample::CreateSample(BufferDesc &bd, void *x) 
        : lpvWrite(0), dwLength(0), desc({0}), xdata(x) {
    init_wave(desc, bd);
    maxval = (1 << desc.wBitsPerSample) -1;
}
