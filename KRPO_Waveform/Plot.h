#pragma once

#include <vector>
#include <string>

class Plot
{
public:
	std::string name;
	std::string xLabel = "x";
	std::string yLabel = "y";

	std::vector <double> x;
	std::vector <double> y;

	explicit operator bool() const {
		return !x.empty() && x.size() == y.size();
	}
};