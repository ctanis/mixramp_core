#ifndef _MIXRAMP_H_
#define _MIXRAMP_H_

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <map>
#include <stdio.h>


#define MIXRAMP_LOG(msg) std::cerr << "LOG: " << msg << std::endl;
#define MIXRAMP_DEBUG(msg) std::cerr << "DEBUG: " << msg << std::endl;
#define MIXRAMP_ERROR(msg) std::cerr << "ERROR: " << msg << std::endl;


namespace mixramp
{
    static const int MAX_BUFFER=100000;


    /*! superclass of all mix nodes
     */
    class node
    {
    public:

	node() : _ok(false)
	{}

	virtual ~node();

	/*! \returns number of audio channels output by this node
	 */
	int num_channels() { return _num_channels; }

	/*! \returns intended sample rate for audio output
	 */
	int sample_rate()  { return _sr; }

	/*! \returns whether or not this node is /good to go/
	 */
	bool ok()          { return _ok; }
                
	/*! Get a node's audio buffer.  Results are undefined if `ok()` returns `false`
	  \param start sample offset for desired buffer
	  \param len length in samples for desired buffer
	  \returns internally allocated float array of len*num_channels() floats, normalized  [-1,1]
	*/          
	virtual float* get_buffer(long start, int len) = 0;



	friend std::ostream& operator<<(std::ostream& os, const node* nd)
	{
	    return os << nd->_name;
	}

	std::string get_name() {return _name;};

	inline bool is_type(std::string type)
	{return _type == type;}

        virtual bool set(std::string &params);

        bool set_name(int);

    protected:
	int     _num_channels;
	int     _sr;
	bool    _ok;
	std::string _type, _name;
    };
    
    /*! superclass of mix nodes that act as an audio pipe
     */
    class chained_node : public node
    {
    public:
	virtual ~chained_node();
        

	/*! set the node that inputs into this one
	  \param input the input node
	*/
	node* set_input(node* input);

	virtual float* get_buffer(long start, int len);


    protected:
	node* _input;
	std::string _base;
    };
    


    /*! SDL-based file player -- one clip per track
     */
    class track_file : public node
    {
    public:
	track_file(std::string path, long offset=0L);

        
	virtual ~track_file();

	/*! calculate end of this track
	  \returns time in samples for end of track
	*/
	long track_end();

	float* get_buffer(long start, int len);

    private:
	std:: string _path;
	long _offset;
	long _length;

	void* _ss;
	float* _buffer;
    };



    /*! chained_node that scales input amplitude
     */
    class volume : public chained_node
    {
    public:
	volume(float a = 1.0);

	void set_amp(float amp);

	float* get_buffer(long start, int len);

	bool mute();

	inline
	bool is_muted() {return _muted;}

	virtual bool set(std::string &params);

    protected:
	float   _amp;
	bool _muted;
    };
    
    class track_monitor : public chained_node
    {
    public:
        track_monitor();
        
        float* get_buffer(long start, int len);
        float get_level();
     
    protected:
        float level;
        
    };
    

    /*! node that mixes multiple sources together
     */
    class mixer : public node
    {
    public:
	virtual ~mixer();


	/*! construct a mixer -- always in stereo */
	mixer()
	{
	    _type = "mix";
	    _num_channels=2;
	}

	/*! add another source input to mixer--
	  ODDITY TO FIX: first input track must be stereo
	  \param in the new input
	  \returns input id for new input
	*/
	int add_input(node* in);

	/*! \returns number of inputs
	 */
	int num_inputs() { return _ins.size(); }

	virtual float* get_buffer(long start, int len);

    protected:
	std::vector<node*> _ins;
    };

    class audioFace : public chained_node
    {
        
    public:
        audioFace();

        void add(std::string trackname, long offset = 0);
        
        void get_track_levels(std::vector<float> &levels);
        
        void getNodes();

        bool set(std::string &name, std::string &param);

        void mute(std::string &name);

        bool load(std::string dir_ns);
        
        void loadfile(std::string dir);

        void get(std::string* command);
        
        int num_tracks();
    private:
        std::map<std::string, mixramp::node*> node_map;
        int t_count;
        mixramp::node* find(std::string node_name);
    };

    /*
      template<typename T> T parse(const string& str) 
      {
      T t;
      std::istringstream sstr(str);
      sstr >> t;
      return t;
      }
    */


    class commands
    {
    public:
        mixramp::audioFace* face;
    
        commands()
        {
            face = new mixramp::audioFace();
        }
    
        ~commands()
        {
            delete face;
        }
        
        enum projectCommands
        {
            unknown, set,get,mute,add,remove,getNodes,load,loadfile
        };
    
        projectCommands hashit (std::string const&inString)
        {
      
            if(inString == "set") return set;
            if(inString == "get") return get;
            if(inString == "mute") return mute;
            if(inString == "add") return add;
            if(inString == "getNodes") return getNodes;
            if(inString == "load") return load;
            if(inString == "loadfile") return loadfile;

            return unknown;
        }
    
        bool cmd(std::string command)
        {
            std::string buf;
            std::stringstream ss(command);
        
            std::vector<std::string> tokens;
        
            while (ss >> buf)
            {
                tokens.push_back(buf);
            }
	    
            switch (hashit(tokens[0]))
            {
             case getNodes:
                 face->getNodes();
                 break;

                 if (tokens.size() <= 1) 
                 {
                     MIXRAMP_ERROR("Incomplete command"); return false;
                 }

             case load:
                 MIXRAMP_DEBUG("loading " << tokens[1]);
		 
                 return face->load(tokens[1]);
                 //face->loadVolume(tokens[2]);
                 break;
             case mute:
                 face->mute(tokens[1]);
                 break;
             case set:

		 if (tokens.size() < 3)
		 {
                     MIXRAMP_ERROR("Incomplete command");
                     return false;
                 }
                 face->set(tokens[1], tokens[2]);
                 break;
             case add:
                 std::cout <<tokens[0]+" "+tokens[1] << std::endl;
                 face->add(tokens[1]);
                 break;
             case get:
                 break;
             case remove:
                 break;
             case loadfile:
                 MIXRAMP_DEBUG("loading: --" << tokens[1] << "--");
                 face->loadfile(tokens[1]);
                 break;
             default:
                 MIXRAMP_ERROR("unknown command: " << command);
                 return false;
                 
            }

            return true;
        }

        float* get_buffer(long position, int buff)
        {
            return face->get_buffer(position, buff);
        }
        
        int num_channels()
        {
            return face->num_channels();
        }

        int sample_rate()
        {
            return face->sample_rate();
        }

        int num_tracks()
        {
            return face->num_tracks();
        }
        
        void get_track_levels(std::vector<float> &levels)
        {
            face->get_track_levels(levels);
        }

        
    };

    class pan : public chained_node
    {
    public:
	pan(float l = 0);

	virtual ~pan();

	float* get_buffer(long start, int len);

	virtual bool set(std::string &params);

    private:
	float _level;
	float* _mono_buff;
    };
}
    

#endif /* _MIXRAMP_H_ */

