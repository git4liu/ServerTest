#pragma once
#include <vector>
#include "ModelServer.h"

class Util
{
public:
	Util(void);
	~Util(void);


public:
	static void loadConfig(std::vector<ModelServer> * servers);
	
	template <typename T>
	static std::string anyToString(T input);
};

