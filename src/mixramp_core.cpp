//  -*- mode: c++; mode: flymake; -*- 
#include <mixramp.hpp>
#include <cstdlib>
#include <cmath>

mixramp::node::~node()
{
    
}

bool mixramp::node::set(std::string &params)
{
    MIXRAMP_ERROR("Attempting to set the " << _name << " node, which does not support that action.");
    return 0;
}

bool mixramp::node::set_name(int n)
{
    if ( _type == "")
    {
	MIXRAMP_ERROR("Cannot make name without type set.");
	_name = "";
	return false;
    }
    std::stringstream ss;
    ss << _type << n++;
    _name = ss.str();
    return true;
}

// chained node
mixramp::chained_node::~chained_node()
{
    MIXRAMP_DEBUG("deleting chained_node");

    if (_input)
    {
	delete _input;
    }
}



mixramp::node* mixramp::chained_node::set_input(mixramp::node* input)
{
    _input = input;
    _num_channels = input->num_channels();
    _sr = input->sample_rate();
    _ok = input->ok();

    chained_node *cnode = dynamic_cast<chained_node*>(input);
    _base = (cnode != NULL) ? cnode->_base : input->get_name();
    _name = _base + "/" + _type;

    return this;
}


float* mixramp::chained_node::get_buffer(long start, int len)
{
    if (_input != NULL)
    {
	return _input->get_buffer(start, len);
    }
    else
    {
	return NULL;
    }
    
}

// volume node
mixramp::volume::volume(float a)
    : _amp(a), _muted(false)
{
    _type = "vol";
}

void mixramp::volume::set_amp(float amp)
{
    _amp = amp;
}

float* mixramp::volume::get_buffer(long start, int len)
{
    float* buff = chained_node::get_buffer(start, len);

    for (int s=0; s<len*_num_channels; s++)
	buff[s] *= _amp * !_muted;

    return buff;
}


bool mixramp::volume::set(std::string &params)
{
//What happens if params in not valid float?
    float a = ::strtod(params.c_str(), 0);

    if (a < -2.9)
    {
	_amp = 0;
	MIXRAMP_ERROR("Attempting to set vol out of (higher than) -3 to 2 range.");
	return false;
    }
    else if (a > 2)
    {
	_amp = pow(10,.5);
	MIXRAMP_ERROR("Attempting to set vol out of (lower than) -3 to 2 range.");
	return false;
    }
    _amp = pow(10,(a-1)/2);
    return true;
}

bool mixramp::volume::mute()
{
    return _muted = !_muted;
}

//track_monitor
mixramp::track_monitor::track_monitor()
{
    _type = "moni";
}


float* mixramp::track_monitor::get_buffer(long start, int len)
{
    float* buff = chained_node::get_buffer(start, len);
    level = abs(buff[0]);
    for (int s=1; s<len*_num_channels; s++)
    {
        float value = buff[s];
        if (value > level) level=value;
    }
    return buff;
}


float mixramp::track_monitor::get_level()
{

     return level;
}


mixramp::pan::pan(float l)
    : _level(l)
{
    _type = "pan";
    _mono_buff = new float[2 * MAX_BUFFER];
}

float* mixramp::pan::get_buffer(long start, int len)
{
    float* buff = chained_node::get_buffer(start, len);

    float* ret_buff; 
    int small, large;

    float per = fabs(_level);

    small = (_level < 0) ? 1 : 0;
    large = !small;

    bool mono;

    if (_num_channels == 1)
    {
	ret_buff = _mono_buff;
	mono = true;
    }
    else if (_num_channels == 2)
    {
	ret_buff = buff;
	mono = false;
    }
    else //output error message, pass along buffer unmodified
    {
	MIXRAMP_ERROR("Unsupported # of channels: " << _num_channels);
	return buff;
    }

    int i, j;
    for (i=0; i<len*2; i+=2)
    {
	if (mono)
	{j = i/2; //Position in mono stream
	    ret_buff[i+small] = buff[j]*(.75*(1-per));
	    ret_buff[i+large] = buff[j]*(.75+.25*per);
	}
	else
	{
	    ret_buff[i+small] *= (1-per);
	    ret_buff[i+large] = (ret_buff[i+large] + per*ret_buff[i+small]); // removed: /(1+per)
	}
    }
    return ret_buff;
}

bool mixramp::pan::set(std::string &params)
{
    float l = ::strtod(params.c_str(), 0);

    if (l > 1)
    {
	_level = 1;
	MIXRAMP_ERROR("Attempting to set pan level out of (higher than) 1 to -1 range.");
	return false;
    }
    else if (l < -1)
    {
	_level = -1;
	MIXRAMP_ERROR("Attempting to set pan level out of (lower than) 1 to -1 range.");
	return false;
    }

    _level = l;
    return true;
}

mixramp::pan::~pan()
{
    delete[] _mono_buff;
}
