#include "AVFactory.h"

#ifdef VPVM_ENABLE_LIBAV
#include "video/AudioDecoder.h"
#include "video/VideoEncoder.h"
#endif

namespace vpvm
{

AVFactory::AVFactory(QObject *parent)
    : QObject(parent),
      m_parent(parent)
{
}

bool AVFactory::isSupported() const
{
#ifdef VPVM_ENABLE_LIBAV
    return true;
#else
    return false;
#endif
}

IAudioDecoder *AVFactory::createAudioDecoder() const
{
#ifdef VPVM_ENABLE_LIBAV
    return new AudioDecoder(m_parent);
#else
    return 0;
#endif
}

IVideoEncoder *AVFactory::createVideoEncoder() const
{
#ifdef VPVM_ENABLE_LIBAV
    return new VideoEncoder(m_parent);
#else
    return 0;
#endif
}

} /* namespace vpvm */
