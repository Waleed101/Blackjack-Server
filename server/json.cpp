#include <iostream>
#include <jsoncpp/json/json.h>

int main() {
	Json::Value root;
	
	root["name"] = "John";
	
	std::string jsonString = root.toStyledString();
	
	std::cout << jsonString << std::endl;
	
	return 0;
}
