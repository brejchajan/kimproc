/*	ArgumentParser C++: easy and lightweight header library
    Copyright (C) 2015  Jan Brejcha

    OPEN SOURCE LICENCE VUT V BRNĚ
	Verze 1.
	Copyright (c) 2010, Vysoké učení technické v Brně, Antonínská 548/1, PSČ 601 90
	-------------------------------------------------------------------------------
*/

#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <string>
#include <vector>

#include <parameter.h>

using namespace std;

namespace argpar {

class Argument
{
public:
    /**
    * @brief Argument       Constructor of the Argument class.
    * @param _shorname      Short name of the argument without hyphen (e. g. f for -f)
    * @param _name          Full name of the argument without double hyphen(e. g. file for --file)
    * @param _params        Vector of parameters of this argument. Each argument
    *                       can have zero or more parameters.
    * @param _description   Description to be used in generated help.
    * @param _optional      Indicates whether the argument is optional or obligatory.
    *                       Set to true, if this argument is optional, false otherwise.
    *                       Default value is false.
    */
    Argument(string _shortname,
                string _name,
                vector<Parameter> _params,
                string _description = "",
                bool _optional = false)
        : shortname(_shortname), name(_name), params(_params),
          description(_description), optional(_optional), exist(false)
    {
    }

    //getters and setters
    string getName()
    {
        return name;
    }

    string getShortname()
    {
        return shortname;
    }

    bool isOptional()
    {
        return optional;
    }

    string getDescription()
    {
        return description;
    }

    string getShortArg()
    {
        return "-" + shortname;
    }

    string getFullArg()
    {
        return "--" + name;
    }

    /**
     * @brief exists indicates the existance of an argument in an argument list.
     * It is useful for testing whether the optional argument has been set.
     * @return true if the argument has been found in argument list.
     */
    bool exists()
    {
        return exist;
    }

    void setExists(bool e)
    {
        exist = e;
    }

    vector<string> getResult()
    {
        return result;
    }

    void addResult(string res)
    {
        result.push_back(res);
    }

    int getParamCount()
    {
        return params.size();
    }

    Parameter &getParam(int i)
    {
        return params[i];
    }


private:
    string shortname;
    string name;
    vector<Parameter> params;
    string description;
    bool optional;

    vector<string> result;

    //set to true if the argument was found in the argument list.
    bool exist;
};

}
#endif // ARGUMENT_H
