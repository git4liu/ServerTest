#pragma once
#include <string>

class ModelRuntimeConfig
{
public:
	ModelRuntimeConfig(void);
	~ModelRuntimeConfig(void);

public:
	long timer;
	std::string fetion_id;
	std::string fetion_pswd;
	std::string fetion_recv;
	std::string fileDir;
	std::string fetion_shell;

};

