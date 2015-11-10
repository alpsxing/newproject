#include "HTTPBody.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

string HttpPara::ToString()
{
	return m_name + "=" + HttpParaValueToString(m_type, m_value);
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

	result += "\n";
	return result;
}
