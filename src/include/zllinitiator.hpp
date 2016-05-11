//====================================================================================
//     The MIT License (MIT)
//
//     Copyright (c) 2011 Kapparock LLC
//
//     Permission is hereby granted, free of charge, to any person obtaining a copy
//     of this software and associated documentation files (the "Software"), to deal
//     in the Software without restriction, including without limitation the rights
//     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//     copies of the Software, and to permit persons to whom the Software is
//     furnished to do so, subject to the following conditions:
//
//     The above copyright notice and this permission notice shall be included in
//     all copies or substantial portions of the Software.
//
//     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//     IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//     FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//     LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//     OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//     THE SOFTWARE.
//====================================================================================

#ifndef ZLLINITIATOR_HPP_
#define ZLLINITIATOR_HPP_
#include <chrono>
#include <thread>
namespace {
namespace zll_initiator {
	using namespace aps;
	using namespace zcl;
	using namespace std;
	using namespace kapi;
	using namespace kapi::notify;
	using 	Context = ApplicationInterface::Context;

	string uriGen(const char* relURI) { return string{"interpan/zllinitiator/"} + relURI;}
	struct scanRsp
	{
		uint64_t 	ieeeAddr		{};
		uint32_t 	transId			{};
		uint8_t 	rssi				{};
		uint8_t 	zbInfo			{};
		uint8_t 	zllInfo			{};
		uint16_t 	keyMask			{};
		uint32_t 	rspId				{};
		uint64_t 	extPanId		{};
		uint8_t 	nwkUpdateId	{};
		uint8_t 	ch					{};
		uint16_t 	panId				{};
		uint16_t 	nwkAddr			{};
		uint8_t 	numSubDev		{};
		uint8_t 	grpId				{};
		void operator()(uint8_t* buf)
		{
			int i = 3; // zll-zcl header is 3 bytes long
			i 			+= ANSITohostInt(transId, buf + i);
			rssi 		= buf[i++];
			zbInfo 	= buf[i++];
			zllInfo = buf[i++];
			i 			+= 	ANSITohostInt(keyMask, buf + i);
			i 			+=	ANSITohostInt(rspId, buf + i);
			i	 			+= 	ANSITohostInt(extPanId, buf + i);
			nwkUpdateId = buf[i++];
			ch 			= buf[i++];
			i 			+= ANSITohostInt(panId, buf + i);
			i 			+= ANSITohostInt(nwkAddr, buf + i);
			numSubDev = buf[i++];
			grpId 	= buf[i++];
		}
	};

	struct scanReq{
		uint32_t tId;
		scanReq(uint32_t tId_):tId{tId_}{}
		int operator()(uint8_t* buf)
		{
			int i = 0;
			buf[i++] =  0;
			buf[i++] =  cntr();
			buf[i++] =  0;
			i += hostIntToANSI(buf+i, tId);
			buf[i++] =	0x02;
			buf[i++] = 	0x13;
			return i;
		}
	};

	struct rstReq{
		uint32_t tId;
		rstReq(uint32_t tId_):tId{tId_}{}
		int operator()(uint8_t* buf)
		{
			int i = 0;
			buf[i++] =  0x11;
			buf[i++] =  cntr();
			buf[i++] =  0x07;  // rst
			i += hostIntToANSI(buf+i, tId);
			return i;
		}
	};

	struct idReq{
		uint32_t tId;
		idReq(uint32_t tId_):tId{tId_}{}
		int operator()(uint8_t* buf)
		{
			int i = 0;
			buf[i++] =  0x11;
			buf[i++] =  cntr();
			buf[i++] =  0x06;  // identify
			i += hostIntToANSI(buf+i, tId);
			i += hostIntToANSI(buf+i, (uint16_t)0xffff);
			return i;
		}
	};

	struct interPred
	{
		uint32_t panSId{};
		interPred( 	AFMessage_EXT& o	) {	ANSITohostInt(panSId, o.asdu()+3);	}
		interPred(	uint32_t psid_	) : panSId{psid_} {}
		~interPred() 	{};
		bool operator()( AFMessage_EXT& i )
		{
			uint32_t ipanSId;
			ANSITohostInt(ipanSId, i.asdu()+3);
			return (panSId == ipanSId);
		}
	};

	class APDU_EXT : public kr_afmessage_EXT_hide
	{
	private:
		uint8_t* 	asdu_  		{	kr_afmessage_EXT_hide::asdu()};
		uint8_t&  asduLen_ 	{	kr_afmessage_EXT_hide::data[asduLenIdx]};
	public:
		~APDU_EXT() {}
		APDU_EXT(uint16_t panId, uint64_t dstAddr, uint8_t addrMode, uint8_t srcep, uint16_t clusId, std::function<int(uint8_t*)> frame)
		{
			kr_afmessage_EXT_hide::clusterId		( clusId );
			kr_afmessage_EXT_hide::dstAddr			(	dstAddr );
			kr_afmessage_EXT_hide::dstEndpoint	( 0xfe	);
			kr_afmessage_EXT_hide::dstPanId		( panId  );
			kr_afmessage_EXT_hide::addrMode		(	addrMode);      // ieee addr
			kr_afmessage_EXT_hide::srcEndpoint	( srcep );
			kr_afmessage_EXT_hide::options			( 0 );
			kr_afmessage_EXT_hide::tranSeqNum	( 0 );
			kr_afmessage_EXT_hide::radius			( 3 );
			kr_afmessage_EXT_hide::asduLen			( 0 );
			asduLen_ += frame(asdu_+asduLen_);
		}
		//APDU_EXT(uint16_t panId, uint64_t dstAddr, uint8_t srcep, uint16_t clusId, std::function<int(uint8_t*)> frame) : APDU_EXT(panId, dstAddr, 0x03, srcep, clusId, frame) {}
		//APDU_EXT(uint16_t panId, uint16_t dstAddr, uint8_t srcep, uint16_t clusId, std::function<int(uint8_t*)> frame) : APDU_EXT(panId, dstAddr, 0x02, srcep, clusId, frame) {}
		template <typename F>
		void send(F cb) {	data_service_EXT::request(*this,interPred(*this), cb);	}
		void send() 		{	data_service_EXT::request(*this);	}
	};

	uint8_t zllinitrEp;
	void* send_beacon(void *i)
	{
		//using namespace std::chrono;
		using namespace aps::data_service_EXT;
		srand(time(nullptr));
		while (1)
		{
			uint32_t tId = rand();
			scanRsp target;

			auto handle = registerHandlers(interPred(tId), [&target](AFMessage_EXT& x){	target.ieeeAddr = x.srcAddr(); target(x.asdu());	});
			handle.arm();
			APDU_EXT{0xffff, 0xffff,	 				0x02, zllinitrEp, 0x1000, scanReq(tId)}.send(	);

			if (handle.wait_for(1000000)) {
				//APDU_EXT{0xffff, target.ieeeAddr, 0x03, zllinitrEp, 0x1000, idReq(tId)	}.send();
			}

			removeHandler(handle);
			kSleep(10);
		}
		return nullptr;
	}
	using namespace aps::data_service_EXT;

	static const uint8_t CHANNELS[] = {11,11,11,11,11,15,20,25,12,13,14,16,17,18,19,21,22,23,24};
	static const int64_t aplScanTimeBaseDuration = 250; // 0.25sec

	void init()
	{
		Endpoint& localEndpt = thisDevice().newEndpoint(2);
		zllinitrEp = localEndpt.id();
		registerEndpoint( localEndpt.id(), HUE_PROFILE_ID ,HUE_DEVICE_ID , HUE_DEVICE_VERSION );

		registerInterPanCB(localEndpt.id());
		srand(time(nullptr));
		handler(ApplicationInterface::EventTag, uriGen("scan"), [&localEndpt](Context C) {

			uint32_t tId = rand();
			int status = -1;
			JSON rsp{JSONType::JSON_OBJECT};
			JSON& devList = rsp["devList"];
			devList = JSON{JSONType::JSON_ARRAY};
			for (int i = 0; i < 18; i++) {
			//interPanSet(0x0b);
				interPanSet(CHANNELS[i]);
				//LOG_MESSAGE("switched to %02x", CHANNELS[i]);
			auto handle = registerHandlers(interPred(tId), [&devList](AFMessage_EXT& x){
				scanRsp target;
				target.ieeeAddr = x.srcAddr(); target(x.asdu());
				JSON devInfo{JSONType::JSON_OBJECT};
				devInfo["ieeeAddr"] = IntToHexStr(target.ieeeAddr);
				devInfo["channel"]  = IntToHexStr(target.ch);
				devInfo["factoryNew"] = (target.zllInfo & 0x01 == 1);
				devInfo["rssi"] = IntToHexStr(target.rssi);
				devList.newElement() = devInfo;
			});

			handle.arm();
			APDU_EXT{0xffff, 0xffff,	0x02, zllinitrEp, 0x1000, scanReq(tId)}.send(	);
			this_thread::sleep_for (chrono::milliseconds(aplScanTimeBaseDuration));
			status = 0;
			removeHandler(handle);
			}

			rsp["status"] = status;
			intraPanSet();

			C.response(rsp.stringify());
		});

		handler(ApplicationInterface::EventTag, uriGen("identify"), [&localEndpt](Context C) {
			JSON arg(C.parameter().c_str());
			uint64_t ieeeAddr = arg["ieeeAddr"].toInteger();
			uint8_t channel = arg["channel"].toInteger();

			interPanSet(channel);
			uint32_t tId = rand();
			int status = -1;
			JSON rsp{JSONType::JSON_OBJECT};
			JSON& devList = rsp["devList"];
			devList = JSON{JSONType::JSON_ARRAY};

			auto handle = registerHandlers(interPred(tId), [ieeeAddr,tId](AFMessage_EXT& x){
				if (ieeeAddr == x.srcAddr())
				{
					APDU_EXT{0xffff, ieeeAddr, 0x03, zllinitrEp, 0x1000, idReq(tId)	}.send();
				}
			});
			APDU_EXT{0xffff, 0xffff,	0x02, zllinitrEp, 0x1000, scanReq(tId)}.send(	);
			this_thread::sleep_for (chrono::milliseconds(500));
			handle.arm();

			status = 0;
			removeHandler(handle);
			rsp["status"] = status;
			intraPanSet();

			C.response(rsp.stringify());
		});

		handler(ApplicationInterface::EventTag, uriGen("reset"), [&localEndpt](Context C) {
					JSON arg(C.parameter().c_str());
					uint64_t ieeeAddr = arg["ieeeAddr"].toInteger();
					uint8_t channel = arg["channel"].toInteger();

					interPanSet(channel);
					uint32_t tId = rand();
					int status = -1;
					JSON rsp{JSONType::JSON_OBJECT};
					JSON& devList = rsp["devList"];
					devList = JSON{JSONType::JSON_ARRAY};

					auto handle = registerHandlers(interPred(tId), [ieeeAddr,tId](AFMessage_EXT& x){
						if (ieeeAddr == x.srcAddr())
						{
							APDU_EXT{0xffff, ieeeAddr, 0x03, zllinitrEp, 0x1000, rstReq(tId)	}.send();
						}
					});
					APDU_EXT{0xffff, 0xffff,	0x02, zllinitrEp, 0x1000, scanReq(tId)}.send(	);
					this_thread::sleep_for (chrono::milliseconds(500));
					handle.arm();

					status = 0;
					removeHandler(handle);
					rsp["status"] = status;
					intraPanSet();

					C.response(rsp.stringify());
				});
//		APDU_EXT{0xffff, target.ieeeAddr, 0x03, zllinitrEp, 0x1000, idReq(tId)	}.send();
	}
}
}
#endif
