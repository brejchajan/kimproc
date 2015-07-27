/*	ArgumentParser C++: easy and lightweight header library
    Copyright (C) 2015  Jan Brejcha

    OPEN SOURCE LICENCE VUT V BRNĚ
	Verze 1.
	Copyright (c) 2010, Vysoké učení technické v Brně, Antonínská 548/1, PSČ 601 90
	-------------------------------------------------------------------------------
*/

#ifndef ARGUMENTPARSER_H
#define ARGUMENTPARSER_H

//local headers
#include "argument.h"
#include "argumentstore.h"
#include "parameter.h"

//stl headers
#include <typeinfo>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

//other headers

using namespace std;

namespace argpar {

class ArgumentParser
{
public:
    /**
     * @brief ArgumentParser    Constructor of ArgumentParser.
     * @param appName           Name of the application to be used in the
     *                          generated help string.
     * @param appDescription    Description to be used in the generated help
     *                          string.
     */
    ArgumentParser(string _appName, string _appDescription = "", bool _printUsage = true)
        :appName(_appName), appDescription(_appDescription), parsed(false),
          printUsageFlag(_printUsage)
    {

    }

    /**
     * @brief addArgument    Adds argument to be parsed.
     *                          T is the type template of the argument, where two
     *                          are possible - bool and string.

     *                      Default value is false.
     */
    void addArgument(Argument argument)
    {
        args.add(argument);
    }

    /**
     * @brief addArgument   Directly creates the argument ad adds it to the
     *                      arguments to be parsed. It is equivalent to create
     *                      the Argument object first and then use the second
     *                      version of this method - addArgument(Argument argument).
    * @param _shorname      Short name of the argument without hyphen (e. g. f for -f)
    * @param _name          Full name of the argument without double hyphen(e. g. file for --file)
    * @param _paramCount    Count of the parameters following this argument.
    *                      (e.g. -f <param1> <param2> implies paramCount = 2);
    *                      In case of 0 params, only presence of the argument
    *                      is evaluated).
    * @param _description   Description to be used in generated help.
    * @param _optional      Indicates whether the argument is optional or obligatory.
    *                      Set to true, if this argument is optional, false otherwise.
    *                      Default value is false.
    */
    void addArgument(string _shortname,
                     string _name,
                     vector<Parameter> _params,
                     string _description = "",
                     bool _optional = false)
    {
        Argument newArgument(_shortname, _name, _params,
                             _description, _optional);
        args.add(newArgument);
    }

    /**
     * @brief parse     Parses the arguments. In case any non-optional
     *                  (obligatory) argument was not set, the help string
     *                  is printed and false is returned.
     * @param argc      Argument count, obtained by the main function.
     * @param argv      Arguments to be parsed, obtained by the main function.
     * @return          True if all non-optional (obligatory) arguments were
     *                  found, false otherwise.
     */
    bool parse(int argc, const char *argv[])
    {
        parsed = false;
        vector<string> qArguments = argvToStringList(argc, argv);

        for (int i = 0; i < args.size(); ++i)
        {
            Argument &arg = args[i];
            string shortArg = arg.getShortArg();
            string fullArg = arg.getFullArg();
            vector<string>::iterator shortArgIt = find(qArguments.begin(), qArguments.end(), shortArg);
            vector<string>::iterator fullArgIt = find(qArguments.begin(), qArguments.end(), fullArg);
            vector<string>::iterator argIt = min(shortArgIt, fullArgIt);
            if (argIt != qArguments.end())
            {
                //given argument list contains defined argument
                arg.setExists(true);
                int pc = arg.getParamCount();
                if (argc > distance(qArguments.begin(), pc + argIt))
                {
                    for (int j = 1; j <= pc; ++j)
                    {
                        arg.addResult(*(argIt + j));
                    }
                }
                else
                {
                    //we have less arguments in argument list than needed
                    printUsage();
                    return false;
                }
            }
        }

        if (args.verifyAllArguments())
        {
            //all obligatory arguments found
            parsed = true;
            return true;
        }
        else
        {
            printUsage();
            return false;
        }
    }

    /**
     * @brief resultByName  Obtains the result of an parsed argument identified
     *                      by full name of the argument.
     * @param argName       Full name of the argument.
     * @return              The parsed result of the argument. Runtime error
     *                      is thrown in case the parse() method has not been
     *                      called yet.
     */
    vector<string> resultByName(string argName)
    {
        if (!parsed)
        {
            throw std::runtime_error("Unable to get result, parse() method was\
                                     not called yet.");
        }
        Argument *a = args.findByName(argName);
        if (a == NULL)
        {
            throw std::runtime_error("Unable to find argument with name: " + argName);
        }
        return a->getResult();
    }


    /**
     * @brief resultByShortname     Obtains the result of an parsed argument
     *                      identified by the shortname of the argument.
     * @param argName       Full name of the argument.
     * @return              The parsed result of the argument. Runtime error
     *                      is thrown in case the parse() method has not been
     *                      called yet.
     */
    vector<string> resultByShortname(string argName)
    {
        if (!parsed)
        {
            throw std::runtime_error("Unable to get result, parse() method was\
                                     not called yet.");
        }
        Argument *a = args.findByShortname(argName);
        if (a == NULL)
        {
            throw std::runtime_error("Unable to find argument with name: " + argName);
        }
        return a->getResult();
    }

    Argument *argumentByShortname(string shortname)
    {
        if (!parsed)
        {
            throw std::runtime_error("Unable to get result, parse() method was\
                                     not called yet.");
        }
        return args.findByShortname(shortname);
    }

    Argument *argumentByName(string name)
    {
        if (!parsed)
        {
            throw std::runtime_error("Unable to get result, parse() method was\
                                     not called yet.");
        }
        return args.findByName(name);
    }





private:

    string appName;

    string appDescription;

    ArgumentStore args;

    //true if parse method was called already, false otherwise
    bool parsed;
    
	bool printUsageFlag;


    vector<string> argvToStringList(int argc, const char *argv[])
    {
        vector<string> qArguments;
        for (int i = 0; i < argc; ++i)
        {
            qArguments.push_back(string(argv[i]));
        }
        return qArguments;
    }

    void printUsage()
    {
        if (printUsageFlag)
        {
            cout << "SYNOPSIS: " << endl;
            cout << "./" << appName << " ";
            for (int i = 0; i < args.size(); ++i)
            {
                Argument &arg = args[i];
                if (arg.isOptional())
                    cout << "[";
                cout << arg.getShortArg();
                for (int j = 0; j < arg.getParamCount(); ++j)
                {
                    cout << " <" << arg.getParam(j).name << ">";
                }
                if (arg.isOptional())
                    cout << "]";
                cout << " ";
            }
            cout << endl << endl;
            //detailed descriptions
            cout << "DESCRIPTION:" << endl;
            cout << appDescription << endl << endl;
            for (int i = 0; i < args.size(); ++i)
            {
                Argument &arg = args[i];
                cout << "\t";
                cout << arg.getShortArg() << ", " << arg.getFullArg() << ": " <<
                        arg.getDescription();
                for (int j = 0; j < arg.getParamCount(); ++j)
                {
                    Parameter p = arg.getParam(j);
                    cout << endl;
                    cout << "\t\t <" << p.name << "> " << p.description;
                }
                cout << " " << endl;
            }

            flush(cout);
        }
    }
};

}

#endif // ARGUMENTPARSER_H
