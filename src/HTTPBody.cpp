#include "HTTPBody.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/algorithm/string.hpp>

typedef map<string, EnumHttpParaType> HttpParaTypeMap;
static HttpParaTypeMap _supportedParas;
typedef vector<string> split_vector_type;

using namespace boost;

static void _init_supported_paras()
{
	if(_supportedParas.size() > 0)
		return;

	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_OP, HTTP_PARA_TYPE_INT));
	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_ID, HTTP_PARA_TYPE_STRING));
	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_CFG_FLG, HTTP_PARA_TYPE_INT));
	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_MEM, HTTP_PARA_TYPE_STRING));
	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_CPU, HTTP_PARA_TYPE_INT));
	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_SYS_VER, HTTP_PARA_TYPE_STRING));
	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_MAC_CNT, HTTP_PARA_TYPE_INT));
	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_ID_CNT, HTTP_PARA_TYPE_INT));
	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_STIME, HTTP_PARA_TYPE_STRING));
	_supportedParas.insert(HttpParaTypeMap::value_type(PARA_ETIME, HTTP_PARA_TYPE_STRING));
	_supportedParas.insert(HttpParaTypeMap::value_type("invl", HTTP_PARA_TYPE_INT));
}

HttpPara *HttpPara::CreatePara(string para)
{
	split_vector_type para_pair;
	string name, value;
	_init_supported_paras();

	split(para_pair, para, is_any_of("="), token_compress_on);
	if(para_pair.size() != 2)
		return NULL;

	name = para_pair[0];
	value = para_pair[1];
	trim(name);
	trim(value);

	if((name.length() <= 0) || (value.length() <= 0))
		return NULL;

	if(_supportedParas.count(name) < 1)
		return NULL;

	EnumHttpParaType type = (EnumHttpParaType)_supportedParas[name];
	return new HttpPara(name, type, value);
}

int HttpPara::Parse(string value)
{
	switch(m_type)
	{
		case HTTP_PARA_TYPE_INT:
			m_value = HttpParaValue(stringToNum<int>(value));
			break;
		case HTTP_PARA_TYPE_FLOAT:
			m_value = HttpParaValue(stringToNum<float>(value));
			break;
		case HTTP_PARA_TYPE_STRING:
			m_value = value;
			break;
	}

	return 0;
}

void HttpOper::AddPara(HttpPara *para)
{
	if(!para)
		return;
	m_paras.insert(HttpParaMap::value_type(para->GetName(), para));
}

void HttpOper::AddPara(string name, int value)
{
	HttpPara *para = new HttpPara(name, HTTP_PARA_TYPE_INT, value);
	if(!para)
		return;

	m_paras.insert(HttpParaMap::value_type(name, para));
}

void HttpOper::AddPara(string name, float value)
{
	HttpPara *para = new HttpPara(name, HTTP_PARA_TYPE_FLOAT, value);
	if(!para)
		return;

	m_paras.insert(HttpParaMap::value_type(name, para));
}

void HttpOper::AddPara(string name, string value)
{
	HttpPara *para = new HttpPara(name, HTTP_PARA_TYPE_STRING, value);
	if(!para)
		return;

	m_paras.insert(HttpParaMap::value_type(name, para));
}

void HttpOper::Clear()
{
	HttpParaMap::iterator itr = m_paras.begin();

	while(itr != m_paras.end())
	{
		delete (HttpPara *)itr->second;
		m_paras.erase(itr);
	}
}

string HttpOper::ToString()
{
	string result;

	result = numToString<int>(m_oper);

	HttpParaMap::iterator itr = m_paras.begin();

	while(itr != m_paras.end())
	{
		HttpPara *para = (HttpPara *)itr->second;
		result += "&";
		result += para->ToString();
	}

	return result;
}

HttpOper *HttpOper::CreateOper(string oper)
{
	split_vector_type paras;
	int op_get = 0;
	HttpOper *http_oper = NULL;

	split(paras, oper, is_any_of("&"), token_compress_on);
	if(paras.size() < 1)
		return NULL;

	for(int i = 0; i < paras.size(); i ++)
	{
		string para_string = paras[i];
		trim(para_string);
		if(para_string.length() <= 0)
			continue;

		HttpPara *para = HttpPara::CreatePara(para_string);
		if(!http_oper)
		{
			if(!para)
				return NULL;
			if(para->GetName().compare("op")) {
				delete para;
				return NULL;
			}
			http_oper = new HttpOper(GetHttpParaValueInt(para->GetValue()));
			delete para;
			if(!http_oper)
				return NULL;
			continue;
		}
		if(!para)
			continue;
		http_oper->AddPara(para);
	}

	return http_oper;
}

HttpBody::~HttpBody()
{
	for(int i = 0; i < m_opers.size(); i ++)
	{
		HttpOper *oper = m_opers[i];
		delete oper;
	}
}

HttpBody *HttpBody::CreateBody(string body)
{
	split_vector_type opers;
	HttpBody *http_body = NULL;

	split(opers, body, is_any_of("\n"), token_compress_on);
	if(opers.size() < 1)
		return NULL;

	for(int i = 0; i < opers.size(); i ++)
	{
		string oper_string = opers[i];
		trim(oper_string);
		if(oper_string.length() <= 0)
			continue;

		HttpOper *oper = HttpOper::CreateOper(oper_string);
		if(!oper)
			continue;
		if(!http_body)
		{
			http_body = new HttpBody();
			if(!http_body)
				return NULL;
		}
		http_body->AddOper(oper);
	}

	return http_body;
}

string HttpBody::ToString()
{
	string result = "";

	for(int i = 0; i < m_opers.size(); i ++)
	{
		if(i > 0)
			result += "\n";
		HttpOper *oper = m_opers[i];
		result += oper->ToString();
	}

	return result;
}
