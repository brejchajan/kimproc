/*	ArgumentParser C++: easy and lightweight header library
    Copyright (C) 2015  Jan Brejcha

    OPEN SOURCE LICENCE VUT V BRNĚ
	Verze 1.
	Copyright (c) 2010, Vysoké učení technické v Brně, Antonínská 548/1, PSČ 601 90
	-------------------------------------------------------------------------------
*/

#ifndef ARGUMENTSTORE_H
#define ARGUMENTSTORE_H

#include "argument.h"

#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace argpar {

class ArgumentStore{
public:
    Argument *findByShortname(string shortname)
    {
        for (Argument &arg : args)
        {
            if (arg.getShortname() == shortname)
            {
                return &arg;
            }
        }
        return NULL;
    }

    Argument *findByName(string name)
    {
        for (Argument &arg : args)
        {
            if (arg.getName() == name)
            {
                return &arg;
            }
        }
        return NULL;
    }

    void add(Argument arg)
    {
        if (findByName(arg.getName()) != NULL)
        {
            throw std::runtime_error("Argument with this name already exists.");
        }
        if (findByShortname(arg.getShortname()) != NULL)
        {
            throw std::runtime_error("Argument with this shortname already exists.");
        }

        args.push_back(arg);
    }

    /**
     * @brief verifyAllArguments
     * @return true, if all obligatory arguments in this ArgumentStore were
     *         found in argument list, false otherwise.
     */
    bool verifyAllArguments()
    {
        for (Argument &arg : args)
        {
            if (!arg.exists() && !arg.isOptional())
                return false;
        }

        return true;
    }

    int size()
    {
        return args.size();
    }

    Argument &operator[] (int i)
    {
        return args[i];
    }

private:
    vector<Argument> args;
};

}

#endif // ARGUMENTSTORE_H
