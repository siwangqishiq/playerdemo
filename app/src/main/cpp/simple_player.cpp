#include "simple_player.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

static pthread_mutex_t  audioEngineLock = PTHREAD_MUTEX_INITIALIZER;

static SLObjectItf engineObject = nullptr;
static SLEngineItf engineEngine;

static SLmilliHertz bqPlayerSampleRate = 0;

// output mix interfaces
static SLObjectItf outputMixObject = nullptr;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = nullptr;

// file descriptor player interfaces
static SLObjectItf fdPlayerObject = nullptr;
static SLPlayItf fdPlayerPlay;
static SLSeekItf fdPlayerSeek;
static SLMuteSoloItf fdPlayerMuteSolo;
static SLVolumeItf fdPlayerVolume;

extern "C" void freeFdPlayerObject();

extern "C" JNIEXPORT
jstring JNICALL
Java_panyi_xyz_playerdemo_SimplePlayerBridge_sayHello(JNIEnv* env, jclass clazz , jint code){
    const char *str = "Hello World from native 潘易";
    jstring  result = env->NewStringUTF(str);
    return result;
}


extern "C"
JNIEXPORT void JNICALL
Java_panyi_xyz_playerdemo_SimplePlayerBridge_createEngine(JNIEnv *env, jclass clazz) {
    SLresult  result;

    result = slCreateEngine(&engineObject , 0 , nullptr , 0 , nullptr , nullptr);
    assert(result == SL_RESULT_SUCCESS);

    result =  (*engineObject)->Realize(engineObject , SL_BOOLEAN_FALSE);
    assert(result == SL_RESULT_SUCCESS);

    result = (*engineObject)->GetInterface(engineObject , SL_IID_ENGINE , &engineEngine);
    assert(result == SL_RESULT_SUCCESS);

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

}

extern "C"
JNIEXPORT void JNICALL
Java_panyi_xyz_playerdemo_SimplePlayerBridge_createBufferQueueAudioPlayer(JNIEnv *env, jclass clazz,
                                                                          jint sample_rate,
                                                                          jint samples_per_buf) {
    SLresult result;
    if (sample_rate >= 0 && samples_per_buf >= 0 ) {
        bqPlayerSampleRate = sample_rate * 1000;
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm ={SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                  SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                  SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};

    if(bqPlayerSampleRate) {
        format_pcm.samplesPerSec = bqPlayerSampleRate;       //sample rate in mili second
    }
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, nullptr};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_panyi_xyz_playerdemo_SimplePlayerBridge_createAssetAudioPlayer(JNIEnv *env, jclass clazz,
                                                                    jstring filename , jlong size) {
    freeFdPlayerObject();
    SLresult result;

    // convert Java string to UTF-8
    const char *utf8 = (env)->GetStringUTFChars(filename, nullptr);
    assert(nullptr != utf8);

    // use asset manager to open asset by filename
//    AAssetManager* mgr = AAssetManager_fromJava(env, asset_manager);
//    assert(nullptr != mgr);
    //AAsset* asset = AAssetManager_open(mgr, utf8, AASSET_MODE_UNKNOWN);

    // the asset might not be found
//    if (nullptr == asset) {
//        return JNI_FALSE;
//    }

    // open asset as file descriptor
//    off_t start, length;
//    int fd = AAsset_openFileDescriptor(asset, &start, &length);
//    assert(0 <= fd);
//    AAsset_close(asset);

    // configure audio source
//    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
//    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
//    SLDataSource audioSrc = {&loc_fd, &format_mime};

    SLDataLocator_URI uri = {SL_DATALOCATOR_URI , (SLchar *)utf8};
//    SLDataLocator_Address address = {SL_DATALOCATOR_ADDRESS , (void *)utf8 , (SLuint32)size};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&uri, &format_mime};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &fdPlayerObject, &audioSrc, &audioSnk,
                                                3, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*fdPlayerObject)->Realize(fdPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the play interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_PLAY, &fdPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the seek interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_SEEK, &fdPlayerSeek);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the mute/solo interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_MUTESOLO, &fdPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the volume interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_VOLUME, &fdPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // enable whole file looping
    result = (*fdPlayerSeek)->SetLoop(fdPlayerSeek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);
    assert(SL_RESULT_SUCCESS == result);

    // release the Java string and UTF-8
    (env)->ReleaseStringUTFChars(filename, utf8);

    return JNI_TRUE;
}

void freeFdPlayerObject(){
    // destroy file descriptor audio player object, and invalidate all associated interfaces
    if (fdPlayerObject != nullptr) {
        (*fdPlayerObject)->Destroy(fdPlayerObject);
        fdPlayerObject = nullptr;
        fdPlayerPlay = nullptr;
        fdPlayerSeek = nullptr;
        fdPlayerMuteSolo = nullptr;
        fdPlayerVolume = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_panyi_xyz_playerdemo_SimplePlayerBridge_shutdown(JNIEnv *env, jclass clazz) {
    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != nullptr) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = nullptr;
        outputMixEnvironmentalReverb = nullptr;
    }

    freeFdPlayerObject();
    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != nullptr) {
        (*engineObject)->Destroy(engineObject);
        engineObject = nullptr;
        engineEngine = nullptr;
    }

    pthread_mutex_destroy(&audioEngineLock);
}


extern "C"
JNIEXPORT void JNICALL
Java_panyi_xyz_playerdemo_SimplePlayerBridge_setPlayingAssetAudioPlayer(JNIEnv *env, jclass clazz,
                                                                        jboolean is_playing) {
    SLresult result;

    // make sure the asset audio player was created
    if (nullptr != fdPlayerPlay) {
        // set the player's state
        result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, is_playing ?SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_PAUSED);
        assert(SL_RESULT_SUCCESS == result);
    }
}