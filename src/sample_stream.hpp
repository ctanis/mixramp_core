#ifndef _SAMPLE_STREAM_H_
#define _SAMPLE_STREAM_H_

#include <SDL2/SDL.h>
#include <string>

// base sample converter for both reading and writing
class sample_stream
{
public:
    sample_stream(Uint8* rawbuff) : rawbuff(rawbuff)
    {}

    // get the next sample scaled to range [-1,1]
    virtual float	read_sample()=0;

    // write a sample in [-1,1] automatically converted to scale
    // required of audio hardware
    virtual void	write_sample(float f)=0;

    // if this returns true, read/write are undefined
    virtual bool	at_end()=0;

    virtual std::string type()=0;
    
    //length in samples...
    virtual long length() = 0;
    
    virtual ~sample_stream()
    {
        SDL_FreeWAV(rawbuff);
        rawbuff = NULL;
    }

    virtual void set_pos(long p)=0;
    

protected:
    Uint8* rawbuff;
    

};


// call this:
sample_stream* ss_wrap  (SDL_AudioSpec spec, Uint8* buff, int len);




// templated subclass for automatic sample conversion

template<typename T>
class XintX : public sample_stream
{
public:
    XintX(Uint8* buff, int len, std::string name) : sample_stream(buff),
                                                    buff((T*)buff),
                                                    len(len/sizeof(T)),
                                                    pos(0),
                                                    name(name)
    { }

    float read_sample()
    {
        T out = buff[pos];
        pos++;

        return float_cast(out);
    }

    void write_sample(float f)
    {
        T in = T_cast(f);

        buff[pos]=in;
        pos++;
    }

    bool at_end()
    {
        return pos >= len;
    }

    void set_pos(long pos)
    {
        this->pos = pos;
    }
    
    std::string type()
    {
        return name;
    }

    long length()               // in samples
    {
        return len;
    }


protected:
    T*	buff;
    long len;
    long pos;
    std::string name;
    T	T_cast(float f);
    float float_cast(T t);

};




// KLUGE Alert!  I have manually calculated all these
// conversions.. there's probably a more clever way.

template<>
Uint8 XintX<Uint8>::T_cast(float f)
{
    return (f + 1.0) * 255 / 2.0;

}

template<>
Uint16 XintX<Uint16>::T_cast(float f)
{
    return (f + 1.0) * 65535 / 2.0;
}

template<>
Sint8 XintX<Sint8>::T_cast(float f)
{
    return f * 127;
}

template<>
Sint16 XintX<Sint16>::T_cast(float f)
{
    return f * 32767;
}



template<>
float XintX<Uint8>::float_cast(Uint8 t)
{
    return (t/255.0) - 1.0;

}

template<>
float XintX<Uint16>::float_cast(Uint16 t)
{
    return (t/65535.0) - 1.0;

}

template<>
float XintX<Sint8>::float_cast(Sint8 t)
{
    return t/127.0;
}

template<>
float XintX<Sint16>::float_cast(Sint16 t)
{
    return t/32767.0;
}



// return appropriate subclass for this AudioSpec -- make sure to
// delete this!

// inline sample_stream* ss_wrap(SDL_AudioSpec spec, Uint8* buff, int len)
// {
//     switch(spec.format)
//     {
//      case AUDIO_U8: return new XintX<Uint8>(buff, len);
//      case AUDIO_U16LSB: return new XintX<Uint16>(buff, len);
//      case AUDIO_S8: return new XintX<Sint8>(buff, len);
//      case AUDIO_S16LSB: return new XintX<Sint16>(buff, len);
//      default:
//          return NULL;
//     }
// }


#endif


