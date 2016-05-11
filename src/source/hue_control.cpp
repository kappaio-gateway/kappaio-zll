//====================================================================================
//			The MIT License (MIT)
//
//			Copyright (c) 2011 Kapparock LLC
//
//			Permission is hereby granted, free of charge, to any person obtaining a copy
//			of this software and associated documentation files (the "Software"), to deal
//			in the Software without restriction, including without limitation the rights
//			to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//			copies of the Software, and to permit persons to whom the Software is
//			furnished to do so, subject to the following conditions:
//
//			The above copyright notice and this permission notice shall be included in
//			all copies or substantial portions of the Software.
//
//			THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//			IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//			FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//			AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//			LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//			OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//			THE SOFTWARE.
//====================================================================================
#include "hue_control.h"
#include "notification.h"
#include "zdo.h"
#include "apsdb.h"
#include "kjson.h"
#include "zcl.h"
#include "restful.h"
#include <vector>
#include <iostream>
#include <string>
#include <utility>
#include <initializer_list>
#include "hal.h"
#include "zllinitiator.hpp"

namespace
{
namespace hue_endpoint
{
	using namespace aps;
	using namespace zcl;
	using namespace std;
	using namespace 	kapi::notify;
	using 		JSON	= kapi::JSON;
	using 	Context = ApplicationInterface::Context;
	inline void get_attribute(Cluster& clus) { APDU{ clus , getAttr{ clus } }.send( setAttr{ clus } ); }

	void init()
	{
		/* register an endpoint, obtain the endpoint address */
		Endpoint& localEndpt = thisDevice().newEndpoint(1);

		/* register an endpoint */
		registerEndpoint( localEndpt.id(),HA_PROFILE_ID ,HUE_DEVICE_ID , HUE_DEVICE_VERSION );

		/* specifies the where should the host SW read the web widget code, if
		 * unspecified, it will search for the default location, which may or
		 * may not support the device, see /usr/lib/rsserial/widgetObj.json */

		localEndpt.setClusterLookupPath("/usr/lib/rsserial/philips_hue/hueclusters.json");
		//localEndpt.setWidgetRoot("/usr/lib/rsserial/philips_hue");
		//localEndpt.setWidgetIndexFile("hue_widget.html");

		//========= 09092014 ========

		const uint16_t OnOffClusterId = 0x0006;
		const uint16_t LevelClusterId = 0x0008;
		const uint16_t ColorClusterId = 0x0300;
		const uint16_t IdentifyClusterId = 0x0003;

		handler(ApplicationInterface::EventTag, localEndpt.uri(), [&localEndpt](Context C) {
				C.response(localEndpt.toJSON().stringify());
		});

		handler("NewJoin", localEndpt.uri(), [](Endpoint& E) {
			handler(ApplicationInterface::EventTag, E.uri() + "/states", [&E](Context C) {
				get_attribute(E.clusters( OnOffClusterId ));
				get_attribute(E.clusters( LevelClusterId ));
				get_attribute(E.clusters( ColorClusterId ));
				C.response(E.toJSON().stringify());
			});

			handler(ApplicationInterface::EventTag, E.uri() + "/toggle", [&E](Context C) {
				Cluster& cluster = E.clusters( OnOffClusterId );
				APDU{	cluster	, onoff		{0x02} 			}.send([](AFMessage& x){});
				APDU{ cluster , getAttr	{ cluster } }.send( setAttr{ cluster } );
				APDU{ E.clusters( LevelClusterId ) , getAttr	{ E.clusters( LevelClusterId ) } }.send( setAttr{ E.clusters( LevelClusterId ) } );
				APDU{ E.clusters( ColorClusterId ) , getAttr	{ E.clusters( ColorClusterId ) } }.send( setAttr{ E.clusters( ColorClusterId ) } );
				C.response(E.toJSON().stringify());
			});

			handler(ApplicationInterface::EventTag, E.uri() + "/level", [&E](Context C) {
				JSON arg(C.parameter().c_str());
				uint8_t 	val 	=  arg["val"].toInteger();
				uint16_t rate 	=  arg["rate"].toInteger();
				Cluster& cluster = E.clusters( LevelClusterId );
				APDU{	cluster	, level  {val,rate	} 	}.send([](AFMessage& x){});
				APDU{ cluster , getAttr{ cluster 	} 	}.send( setAttr{ cluster } );
				APDU{ E.clusters( OnOffClusterId ) , getAttr	{ E.clusters( OnOffClusterId ) } }.send( setAttr{ E.clusters( OnOffClusterId ) } );
				APDU{ E.clusters( ColorClusterId ) , getAttr	{ E.clusters( ColorClusterId ) } }.send( setAttr{ E.clusters( ColorClusterId ) } );
				C.response(E.toJSON().stringify());
			});

			handler(ApplicationInterface::EventTag, E.uri() + "/color", [&E](Context C) {
				JSON arg(C.parameter().c_str());
				uint16_t x 	=  arg["x"].toInteger();
				uint16_t y 	=  arg["y"].toInteger();
				uint16_t t 	=  arg["rate"].toInteger();
				Cluster& cluster = E.clusters( ColorClusterId );
				APDU{	cluster	, color  {	x,y,t		} 	}.send([](AFMessage& x){});
				APDU{ cluster , getAttr{	cluster } 	}.send( setAttr{ cluster } );
				C.response(E.toJSON().stringify());
			});
			handler(ApplicationInterface::EventTag, E.uri() + "/identify", [&E](Context C) {
				JSON arg(C.parameter().c_str());

				uint16_t t 	=  arg["identifytime"].toInteger();
				Cluster& cluster = E.clusters( IdentifyClusterId );
				APDU{	cluster	, identify  { 0, t	} 	}.send([](AFMessage& x){});
				C.response(E.toJSON().stringify());
			});
		});
		SimpleDescriptor& simpleDescriptor 	= localEndpt.simpleDescriptor();
		simpleDescriptor.profileId 					= HUE_PROFILE_ID;
		simpleDescriptor.deviceId 					= HUE_DEVICE_ID;
		simpleDescriptor.deviceVersion 			= HUE_DEVICE_VERSION;
		simpleDescriptor.outclusterList 		= {0x0006, 0x0008, 0x0300};
		return;
	}
} // namespace hue_endpoint
}

void init()
{
	hue_endpoint::init();
	zll_initiator::init();
}
