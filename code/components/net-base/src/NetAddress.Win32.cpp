/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NetAddress.h"

#include <windns.h>

#pragma comment(lib, "dnsapi.lib")

namespace net
{
boost::optional<std::string> PeerAddress::LookupServiceRecord(const std::string& serviceHost, uint16_t* servicePort)
{
	// convert to a wide string for the Windows API
	fwPlatformString platformHostName(serviceHost.c_str());

	// output variable
	PDNS_RECORD dnsRecords;

	// return value
	boost::optional<std::string> retval;

	// perform the query
	if (DnsQuery(platformHostName.c_str(), DNS_TYPE_SRV, DNS_QUERY_STANDARD, nullptr, &dnsRecords, nullptr) == ERROR_SUCCESS)
	{
		// use the first DNS record for now
		if (dnsRecords)
		{
			// get the SRV data
			DNS_SRV_DATA& srvData = dnsRecords->Data.SRV;

			// set the service port
			if (servicePort)
			{
				*servicePort = srvData.wPort;
			}

			// and set the return string
			std::wstring nameString(srvData.pNameTarget);
			
			retval = std::string(nameString.begin(), nameString.end());
		}

		// free the returned structure
		DnsRecordListFree(dnsRecords, DnsFreeRecordList);
	}

	// and return the altered hostname
	return retval;
}
}