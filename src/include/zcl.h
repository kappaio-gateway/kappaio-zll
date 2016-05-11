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
#ifndef ZCL_H
#define ZCL_H
#include "apsdb.h"
#include "kzdef.h"
#include "kutil.h"
#include "kglobal.h"
#include <functional>
#include <vector>

namespace aps {
	struct zclPred {
		uint8_t apsCount;
		uint8_t dstEnpoint;
		zclPred( AFMessage& o)
		:apsCount((o.asdu()[0] & 0x04) ? o.asdu()[3] : o.asdu()[1]),
		 dstEnpoint(o.srcEndpoint())
		{}
		~zclPred()
		{};
		bool operator()( AFMessage& i ) {
			return (((i.asdu()[0] & 0x04) ? i.asdu()[3] : i.asdu()[1] == apsCount) && (i.dstEndpoint() == dstEnpoint));
		}
	};

	class APDU : public kr_afmessage_hide
	{
		private:
			uint8_t* 	asdu_  		{	kr_afmessage_hide::asdu()};
			uint8_t&  asduLen_ 	{	kr_afmessage_hide::data[asduLenIdx]};
		public:
			~APDU() {}
			APDU() = delete;
			APDU(Cluster& dstCls)
			{
				Endpoint& dstEpt = *(dstCls.parent());
				Device& 	dstDev = *(dstEpt.parent());
				kr_afmessage_hide::clusterId		( dstCls.id() );
				kr_afmessage_hide::dstAddr			(	dstDev.id() );
				kr_afmessage_hide::dstEndpoint	( dstEpt.id() );
				kr_afmessage_hide::srcEndpoint	( dstEpt.localPeer()->id() );
				kr_afmessage_hide::options			( 0 );
				kr_afmessage_hide::tranSeqNum	( 0 );
				kr_afmessage_hide::radius			( 3 );
				kr_afmessage_hide::asduLen			( 0 );
			}

			APDU(Cluster& dstCls,std::function<int(uint8_t*)> zclFrame):APDU(dstCls) {
				asduLen_ += zclFrame(asdu_+asduLen_);
			}

			template <typename F>
			void send(F cb, bool block=true) { data_service::request(*this, zclPred(*this), cb); }
		};
}

namespace zcl {
	using Device 						= aps::Device;
	using Endpoint 					= aps::Endpoint;
	using Cluster 					= aps::Cluster;
	using Attribute 				= aps::Attribute;
	using NodeDescriptor_t 	= aps::NodeDescriptor_t;
	using SimpleDescriptor 	= aps::SimpleDescriptor;
	const uint8_t goodbit 								= 0x00;
	const uint8_t dstAddrOutOfRange 			= 0x01;
	const uint8_t extdstAddrOutOfRange 		= 0x02;
	const uint8_t dstEndpointOutOfRange 	= 0x04;
	const uint8_t dstClusterIdOutOfRange 	= 0x08;
	const uint8_t cmdAcrossProfile 				= 0x00;
	const uint8_t cmdClusterSpecific 			= 0x01;
	const uint8_t cmdFromClient 					= 0<<3;
	const uint8_t cmdFromServer 					= 1<<3;
	const uint8_t cmdDisableDefaultRsp 		= 1<<4;
	const uint8_t cmdManufactureSpecific	= 1<<2;

	inline int zclHeader(uint8_t frameControl, uint8_t seqNum, uint8_t cmdId, uint8_t* buf)
	{
		int i=0;
		buf[i++] = frameControl;
		buf[i++] = seqNum;
		buf[i++] = cmdId;
		return i;
	}

	struct onoff
	{
		uint8_t cmdId;
		onoff(uint8_t cmdId):cmdId{cmdId}
		{}
		int operator()(uint8_t* buf)
		{
			return zclHeader(cmdClusterSpecific | cmdFromClient, aps::cntr(), cmdId, buf);
		}
	};
	struct identify
	{
		uint8_t cmdId;
		uint16_t idTime;
		identify(uint8_t cmdId, uint16_t idTime):cmdId{cmdId}, idTime{idTime}
		{}
		int operator()(uint8_t* buf)
		{
			int i =  zclHeader(cmdClusterSpecific | cmdFromClient, aps::cntr(), cmdId, buf);
			buf[i++] = idTime;
			i += hostIntToANSI(buf+i,idTime);
			return i;
		}
	};
	struct level
	{
		uint8_t value;
		uint16_t rate;
		level(uint8_t val, uint16_t rate):value{val},rate{rate} {}
		int operator()(uint8_t* buf)
		{
			int i = zclHeader(cmdClusterSpecific | cmdFromClient, aps::cntr(), 0x04, buf);
			buf[i++] = value;
			i += hostIntToANSI(buf+i,rate);
			return i;
		}
	};

	struct color
	{
		uint16_t x;
		uint16_t y;
		uint16_t transTime;
		color(uint16_t x, uint16_t y, uint16_t transTime):x{x},y{y},transTime{transTime}
		{}
		int operator()(uint8_t* buf) {
			int i = zclHeader(cmdClusterSpecific | cmdFromClient, aps::cntr(), 0x07, buf);
			i += hostIntToANSI(buf+i,x);
			i += hostIntToANSI(buf+i,y);
			i += hostIntToANSI(buf+i,transTime);
			return i;
		}
	};

	struct getAttr {
		Cluster& clus;
		getAttr(Cluster& clus):clus(clus){}
		int operator()(uint8_t* buf) {
			using namespace zcl;
			int offset = zclHeader(cmdAcrossProfile | cmdFromClient | cmdDisableDefaultRsp, GetAPSCounter(), 0, buf);
			for (auto& attr : clus.attributes()) {
				offset += hostIntToANSI(buf + offset, attr.id());
			}
			return offset;
		}
	};

	struct setAttr
	{
		Cluster& clus;
		setAttr(Cluster& clus):clus(clus){}
		int operator()(AFMessage& msg) {
			int i = 3;
			uint8_t* pl = msg.asdu();
			while (i < msg.asduLen() ) {
				uint16_t attrId;
				i += ANSITohostInt(attrId,pl+i);
				if (pl[i++] != 0 )
					continue;
				Attribute& attr = clus.attributes(attrId);
				uint8_t typeId = pl[i++]; // you can verify the typeId here
				i += attr.value().setValue(pl+i);
			}
			return i;
		}
	};
}// namespace zcl

struct zclFrame_s
{
	uint8_t type;
	uint8_t manufacturerSpecific;
	uint8_t direction;
	uint8_t defaultRsp;
	uint16_t manufacturerCode;
	uint8_t seqNum;
	uint8_t cmdId;
	uint8_t *payload;
	size_t payloadLength;
	AFMessage *pOriginalMessage;
};

#endif
