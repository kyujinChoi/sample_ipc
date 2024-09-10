#ifndef XML_H_
#define XML_H_

#include "Parameter/Parameter.h"
#include <libxml/parser.h>
#include <libxml/tree.h>


class Xml : public Parameter
{
public:
    Xml(){};
    ~Xml(){};
    bool Open();
    bool Read();
    bool Write();
    bool Close();
private:
    bool readChildren(std::string name, xmlNode *node);
private:
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
};
#include "Xml.hpp"
#endif