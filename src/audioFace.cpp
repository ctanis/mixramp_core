#include <mixramp.hpp>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <map>
#include <fstream>

#include <cstdlib>
#include <algorithm>
#include <iterator>

namespace mixramp {

    audioFace::audioFace()
    {
	_type = "audio face";

	mixer* mix = new mixer();
	mix->set_name(0);
	node_map[mix->get_name()] = mix;

	volume* mv = new volume(); //Master volume
	mv->set_input(mix);
	node_map[mv->get_name()] = mv; 

	pan* pa = new pan();
	pa->set_input(mv);
	node_map[pa->get_name()] = pa;

	set_input(pa);
	t_count = 0;
    }

    void audioFace::add(std::string track_name, long offset)
    {
	track_file* tf = new track_file(track_name,offset);
        tf->set_name(t_count++);
	node_map[tf->get_name()] = tf; 

	volume* vol = new volume();
	vol->set_input(tf);
	node_map[vol->get_name()] = vol;

	pan* _pan = new pan();
	_pan->set_input(vol);
	node_map[_pan->get_name()] = _pan; 

        track_monitor* moni = new track_monitor();
        moni->set_input(_pan);
        node_map[moni->get_name()] = moni;

	node* mx = find("mix0");
	if (mx != NULL)	
	    ((mixer *)mx)->add_input(moni);
    }
    
    bool audioFace::load(std::string dir_name)
    {
	DIR *dir;
	struct dirent *entry;
    
	dir = opendir(dir_name.c_str());
	if(!dir)
	{
	    MIXRAMP_ERROR("Directory was not found");
	    return false;
	}
	int c=0;
	while ( (entry = readdir(dir)) != NULL)
	{
	    c++;
	    std::string d_name = entry->d_name;
	    if( d_name != "." && d_name != ".."  && d_name.substr(d_name.size()-4) == ".wav")
        {
            std::string path = dir_name + "/" + d_name;
            add(path);
        }
    }
	closedir(dir);

	return (c != 0);
    }
    

    
    void audioFace::loadfile(std::string dir_name)
    {
        std::string t_name;
        std::ifstream infile(dir_name+"/Manifest.txt");
        if(!infile)
        {
            MIXRAMP_ERROR("Project Not Found");
        }
        else
        {
            while(std::getline (infile,t_name))
            {
                std::istringstream iss(t_name);

                std::vector<std::string>tokens
                {
                    std::istream_iterator<std::string>{iss},
                        std::istream_iterator<std::string>{}
                };

                add(dir_name+"/"+tokens[0],atol(tokens[1].c_str()));
            }
        }
    }
    
    void audioFace::get_track_levels(std::vector<float> &levels)
    {
        for (int i=0; i<t_count;i++)
        {
            std::string name = "track"+std::to_string(i)+"/moni";
            node* node = find(name);
            if (node != NULL)
                levels[i]=((track_monitor*)node)->get_level();
        }
    }
    
    int audioFace::num_tracks()
    {
        return t_count;
    }
    

    void audioFace::getNodes()
    {
	for (auto it=node_map.begin(); it!=node_map.end(); ++it)
	{
//	mixramp::node* no = it->second;
	    std::cout << it->first << std::endl;
	}
    }

    bool audioFace::set(std::string &name, std::string &param)
    {
	node* node = find(name);
	if (node != NULL)
	    return node->set(param);
	else
	    return false;
    }

    void audioFace::mute(std::string &name)
    {
	if (find(name) != NULL)
	{
	    mixramp::node* vol_node = find(name + "/vol");
	    if (vol_node != NULL)
		((volume*)vol_node)->mute();
	}
    }

    inline
    mixramp::node* audioFace::find(std::string node_name)
    {
	std::map<std::string, node*>::iterator iter = node_map.find(node_name);
    
	if (iter != node_map.end())
	    return iter->second;
	else 
	{
	    MIXRAMP_ERROR("Cannot find " << node_name << " node.");
	    return NULL;
	}
    }
}
