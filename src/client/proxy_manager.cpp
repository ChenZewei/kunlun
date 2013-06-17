#include <new>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connector.h"
#include "timed_stream.h"
#include "proxy_manager.h"

CProxyServerManager::CProxyServerManager(CInetAddr *proxy_addr_list, int nlist_count) : \
	m_proxy_stream_count(0), m_proxy_stream_robin(0), m_proxy_stream_container(NULL)
{
	int i, ret;
	
	try
	{
		m_proxy_stream_container = new CTimedStream *[nlist_count];
	}
	catch(std::bad_alloc)
	{
		throw ENOMEM;
	}

	for(i = 0; i < nlist_count; i++)
	{
		try
		{
			CConnector proxy_connector(proxy_addr_list[i]);

			m_proxy_stream_container[i] = new CTimedStream();
			if((ret = proxy_connector.stream_connect(m_proxy_stream_container[i])) != 0)
			{
				throw ret;
			}
			m_proxy_stream_count++;
		}
		catch(std::bad_alloc)
		{
			m_proxy_stream_container[i] = NULL;
			throw ENOMEM;
		}
		catch(int errcode)
		{
			m_proxy_stream_container[i] = NULL;
			throw errcode;
		}
	}
}

CProxyServerManager::~CProxyServerManager()
{
	int i;

	for(i = 0; i < m_proxy_stream_count; i++)
	{
		if(m_proxy_stream_container[i] != NULL)
		{
			delete m_proxy_stream_container[i];
			m_proxy_stream_container[i] = NULL;
		}
	}

	delete [] m_proxy_stream_container;
	m_proxy_stream_container = NULL;
}

CTimedStream *CProxyServerManager::get_proxy_stream_by_robin()
{
	if(m_proxy_stream_robin >= m_proxy_stream_count)
	{
		m_proxy_stream_robin = 0;
	}

	return m_proxy_stream_container[m_proxy_stream_robin++];
}