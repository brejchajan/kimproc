/*	ArgumentParser C++: easy and lightweight header library
    Copyright (C) 2015  Jan Brejcha

    OPEN SOURCE LICENCE VUT V BRNĚ
	Verze 1.
	Copyright (c) 2010, Vysoké učení technické v Brně, Antonínská 548/1, PSČ 601 90
	-------------------------------------------------------------------------------
*/

#ifndef PARAMETER
#define PARAMETER

#include <string>

using namespace std;

namespace argpar {

struct Parameter {
    string name;
    string description;

    Parameter(string _name, string _description)
        :name(_name), description(_description)
    {
    }
};

}

#endif // PARAMETER

