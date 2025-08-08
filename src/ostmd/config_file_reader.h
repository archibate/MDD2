/*!
 * \file config_file_reader.h
 * \date 2023/07/25 11:10
 *
 * \author zhangxuebing
 * Contact: xuebing.zhang@orientfutures.com
 *
 * \brief ≈‰÷√∂¡»°¿‡
 */
#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <algorithm>

#include "efvi_receive_depend.h"

class ConfigFileReader {
public:
	bool LoadConfigFile(const std::string& filePath) {
		std::ifstream file(filePath);
		if (!file) {
			std::cerr << "Unable to open config file: " << filePath << std::endl;
			return false;
		}

		std::string line;
		std::string currentSection;

		while (std::getline(file, line)) {
			line = line.substr(0, line.find("#"));  

			if (line.empty()) continue;

			if (line[0] == '[') { 
				currentSection = line.substr(1, line.find("]") - 1);
			}
			else {  
				auto delimiterPos = line.find("=");
				auto name = line.substr(0, delimiterPos);
				auto value = line.substr(delimiterPos + 1);
				m_config_map[currentSection][name] = trim(value);
			}
		}

		return true;
	}

	std::string GetValueString(const std::string& section, const std::string& name) const {
		auto iter = m_config_map.find(section);
		if (iter != m_config_map.end()) {
			auto innerIter = iter->second.find(name);
			if (innerIter != iter->second.end()) {
				return innerIter->second;
			}
		}
		return "";
	}
	int GetValueInt(const std::string& section, const std::string& name) const {
		std::string valueString = GetValueString(section, name);
		if (valueString == "")
			return 0;
		return std::stoi(valueString);
	}

	bool GetSockUdpParam(sock_udp_param& quote_param, const char* section) {
		int enable = GetValueInt(section, "enable");
		quote_param.m_open = (bool)(enable);

		quote_param.m_cpu_id = GetValueInt(section, "cpu_id");

		quote_param.m_local_ip = GetValueString(section, "local_ip");
		quote_param.m_local_port = GetValueInt(section, "local_port");

		quote_param.m_multicast_ip = GetValueString(section, "multicast_ip");
		quote_param.m_multicast_port = GetValueInt(section, "multicast_port");

		quote_param.m_eth_name = GetValueString(section, "eth_name");
		return true;
	}
private:
	std::string trim(const std::string& str) {
		size_t first = str.find_first_not_of(" \t'\""); 
		if (std::string::npos == first)
			return str;
		size_t last = str.find_last_not_of(" \t'\""); 
		return str.substr(first, (last - first + 1));
	}
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_config_map;
};
