//  -*- mode: c++; mode: flymake; -*- 
#include <mixramp.hpp>
#include <algorithm>



mixramp::mixer::~mixer()
{
    MIXRAMP_DEBUG("deleting mixer");

    for (unsigned int t=0; t<_ins.size(); t++)
    {
        delete _ins[t];
    }
}


// assumes the first input track is stereo
int mixramp::mixer::add_input(mixramp::node* in)
{
    if (_ins.size() == 0)
    {
        _sr = in->sample_rate();
    }
    else
    {
        if (in->sample_rate() != _sr)
        {
            MIXRAMP_LOG("sample rate mismatch in mixer");
            return -1;
        }
    }

    _ins.push_back(in);
    _ok = in->ok();
    return _ins.size()-1;
}


// assumes the first input track is stereo (buff1)
// assumes volume nodes are directly attached to mixer
float* mixramp::mixer::get_buffer(long start, int len)
{
    float* buff1 = _ins[0]->get_buffer(start, len);
    int ni=0;

    if ( _ins[0]->is_type("vol") && ((mixramp::volume*)_ins[0])->is_muted())
    {
	std::fill_n(buff1, len*_ins[0]->num_channels(), 0.0);
	ni++;
    }

    for (unsigned int t=1; t<_ins.size(); t++)
    {
	if (! _ins[t]->is_type("vol") || ! ((mixramp::volume*)_ins[t])->is_muted())
	{
	    float* tbuff = _ins[t]->get_buffer(start, len);
	    ni++;
        
	    if (_ins[t]->num_channels() == 1)
	    {
		// mono track -> stereo mixer
		for (int s=0; s<len; s++)
		{
		    buff1[2*s] += tbuff[s]/ni;
		    buff1[2*s+1] += tbuff[s]/ni;
		}

	    }
	    else
	    {
		// stereo track -> stereo mixer
		for (int s=0; s<len; s++)
		{
		    buff1[2*s] += tbuff[2*s]/ni;
		    buff1[2*s+1] += tbuff[2*s+1]/ni;
		}
	    }
	}
    }
    return buff1;
}
