//  -*- mode: c++; mode: flymake; -*- 
#include <mixramp.hpp>
#include "sample_stream.hpp"


// return appropriate subclass for this AudioSpec -- make sure to
// delete this!

sample_stream* ss_wrap(SDL_AudioSpec spec, Uint8* buff, int len)
{
    switch(spec.format)
    {
     case AUDIO_U8: return new XintX<Uint8>(buff, len, "Uint8");
     case AUDIO_U16LSB: return new XintX<Uint16>(buff, len, "Uint16");
     case AUDIO_S8: return new XintX<Sint8>(buff, len, "Sint8");
     case AUDIO_S16LSB: return new XintX<Sint16>(buff, len, "Sint16");
     default:
         return NULL;
    }
}




// track_file
mixramp::track_file::track_file(std::string path, long offset)
    : node(),
      _path(path),
      _offset(offset),
      _ss(NULL),
      _buffer(NULL)
{

    _type = "track";


    SDL_AudioSpec filespec;
    Uint8* wavedata;
    Uint32 wavelen;

    if (SDL_LoadWAV(path.c_str(), &filespec, &wavedata, &wavelen) == NULL)
    {
        MIXRAMP_ERROR("error loading " << path << " -- " << SDL_GetError());
        return;
    }

    _num_channels = filespec.channels;
    _sr = filespec.freq;

    _ss = ss_wrap(filespec, wavedata, wavelen);

    if (_ss == NULL)
    {
        MIXRAMP_ERROR("unsupported file format: " << filespec.format);
        return;
    }

    _length = static_cast<sample_stream*>(_ss)->length() / _num_channels; // number of samples in time
    _buffer = new float[_num_channels * MAX_BUFFER];

    MIXRAMP_LOG("successfully opened " << path <<
                "; type " << static_cast<sample_stream*>(_ss)->type());

    _ok = true;
}


mixramp::track_file::~track_file()
{
    MIXRAMP_DEBUG("deleting track_file");

    if (_ss != NULL)
    {
        delete static_cast<sample_stream*>(_ss);
    }

    if (_buffer != NULL)
    {
        delete _buffer;
    }

}


long mixramp::track_file::track_end()
{
    return _length+_offset;
}



// fetch len samples (per channel) starting at start'th sample.  we have to
// explicitly zero the buffer if we're not playing audio
float* mixramp::track_file::get_buffer(long start, int len)
{
    sample_stream* ss =  static_cast<sample_stream*>(_ss);
    assert(len <= MAX_BUFFER);

    long actual = start-_offset;


    if (actual > 0)
    {
        ss->set_pos(actual*_num_channels);
    
        for (int b=0; b<len*_num_channels; b++)
        {
            if (ss->at_end())
            {
                _buffer[b] = 0; // empty buffer
            }
	    else
            {
                _buffer[b]=ss->read_sample();
            }
       }

    }
    else
    {
        for (int b=0; b<len*_num_channels; b++)
        {
            _buffer[b] = 0;     // empty buffer
        }
    }
    
    return _buffer;
}
