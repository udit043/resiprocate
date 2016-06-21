#if !defined(Flow_hxx)
#define Flow_hxx

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <map>
#include <rutil/TimeLimitFifo.hxx>
#include <rutil/Mutex.hxx>

#include <srtp.h>
#include <boost/shared_ptr.hpp>

#include "reTurn/client/TurnAsyncUdpSocket.hxx"
#include "reTurn/client/TurnAsyncTcpSocket.hxx"
#include "reTurn/client/TurnAsyncTlsSocket.hxx"
#include "reTurn/client/TurnAsyncSocketHandler.hxx"
#include "reTurn/StunMessage.hxx"
#include "reTurn/DataBuffer.hxx"
#include "reTurn/StunMessage.hxx"
#include "IceCandidate.hxx"
#include "FakeSelectSocketDescriptor.hxx"
#ifdef USE_SSL
#ifdef USE_DTLS
#include "dtls_wrapper/DtlsSocket.hxx"
#endif
#endif

using namespace reTurn;

namespace flowmanager
{

/**
  This class represents a Flow that is created by the Flow Manager.  A flow is a
  bi-directional stream of data for communicating with an endpoint, that may use
  UDP, TCP or TLS over TCP.  A flow may also use a Turn Allocation to transmit
  data to/from an endpoint.

  Author: Scott Godin (sgodin AT SipSpectrum DOT com)
*/
class MediaStream;
class FlowHandler;
class Flow;

class Flow : public TurnAsyncSocketHandler
{
public:

   enum FlowState
   {
      Unconnected,
      ConnectingServer,
      Connecting,
      CheckingConnectivity,
      Binding,
      Allocating,
      Connected,
      Ready
   };

   Flow(asio::io_service& ioService,
#ifdef USE_SSL
        asio::ssl::context& sslContext,
#endif
        unsigned int componentId,
        const StunTuple& localBinding, 
        MediaStream& mediaStream);
   ~Flow();

   void setHandler(FlowHandler* handler);
   
   // this can also be obtained from the FlowManager, but is here as well for convenience
   // for apps that need to do operations on the socket (setting options, setting Qos options, etc.)
   asio::io_service& getIOService() { return mIOService; }

   /// Turn Send Methods
   /// WARNING - if using Secure media, then there must be room at the 
   ///           end of the passed in buffer for the SRTP HMAC code to be appended
   ///           ***It would be good to make this safer***
   void send(boost::shared_ptr<reTurn::DataBuffer>& buffer);
   void rawSendTo(const asio::ip::address& address, unsigned short port, const char* buffer, unsigned int size);

   void asyncReceive();

   /// Used to set where this flow should be sending to
   void setActiveDestination(const resip::Data& address, unsigned short port, const std::vector<reTurn::IceCandidate>& candidates);

   void setIceRole(bool controlling);
   void setPeerReflexiveCandidatePriority(UInt32 priority);

#ifdef USE_SSL
#ifdef USE_DTLS
   /// Dtls-Srtp Methods

   /// Starts the dtls client handshake process - (must call setActiveDestination first)
   /// Call this method if this client has negotiated the "Active" role via SDP
   void startDtlsClient(const char* address, unsigned short port);

   /// This method should be called when remote fingerprint is discovered
   /// via SDP negotiation.  After this is called only dtls-srtp connections
   /// with a matching fingerprint will be maintained.
   void setRemoteSDPFingerprint(const resip::Data& fingerprint);

   /// Retrieves the stored remote SDP Fingerprint.
   const resip::Data getRemoteSDPFingerprint();
#endif
#endif

   //void setOnBeforeSocketClosedFp(boost::function<void(unsigned int)> fp);

   const StunTuple& getLocalTuple();
   unsigned int getComponentId() const { return mComponentId; }

private:
   friend class MediaStream;

   void initialize();
   void shutdown();

   void activateFlow(UInt8 allocationProps = reTurn::StunMessage::PropsNone);
   void activateFlow(UInt64 reservationToken);

   bool isReady() const { return mFlowState == Ready; }
   StunTuple getSessionTuple();  // returns either local, reflexive, or relay tuple depending on NatTraversalMode
   StunTuple getRelayTuple();
   StunTuple getReflexiveTuple();
   UInt64 getReservationToken();
   bool iceComplete() const { return mIceComplete; }
   reTurn::IceCandidate getLocalNominatedIceCandidate() const;
   reTurn::IceCandidate getRemoteNominatedIceCandidate() const;
   void setOutgoingIceUsernameAndPassword(const resip::Data& username, const resip::Data& password);
   void setLocalIcePassword(const resip::Data& password);

   /// Returns a socket descriptor that can be used in a select call
   /// WARNING - this descriptor should not be used for any other purpose
   ///         - do NOT set socket options, or send, receive from this descriptor,
   ///           instead use the Flow api's
   unsigned int getSelectSocketDescriptor();
   unsigned int getSocketDescriptor();  // returns the real socket descriptor - used to correlate callbacks

   void setHandlerImpl(FlowHandler* handler, resip::Condition& cv);
   void sendImpl(boost::shared_ptr<reTurn::DataBuffer>& buffer);
   void asyncReceiveImpl();
   void setActiveDestinationImpl(const resip::Data& address, unsigned short port, const std::vector<reTurn::IceCandidate>& candidates);
   void setIceRoleImpl(bool controlling);
   void setPeerReflexiveCandidatePriorityImpl(UInt32 priority) { mPeerRflxCandidatePriority = priority; }
   void startDtlsClientImpl(const std::string& address, unsigned short port);
   void setRemoteSDPFingerprintImpl(const resip::Data& fingerprint);

   /// Used only when ICE is enabled; should be called once the offer/answer exchange has been completed
   void scheduleConnectivityChecks();
   void onConnectivityCheckTimer(const asio::error_code& error);

   asio::io_service& mIOService;
   asio::deadline_timer mConnectivityCheckTimer;
#ifdef USE_SSL
   asio::ssl::context& mSslContext;
#endif

   enum IceRole
   {
      IceRole_Controlled,
      IceRole_Controlling,
      IceRole_Unknown
   };
   IceRole mIceRole;
   UInt32 mPeerRflxCandidatePriority;

   // Note: these member variables are set at creation time and never changed, thus
   //       they do not require mutex protection
   const unsigned int mComponentId;
   const StunTuple mLocalBinding;

   // MediaStream that this Flow belongs too
   MediaStream& mMediaStream;

   // mTurnSocket has it's own threading protection
   boost::shared_ptr<TurnAsyncSocket> mTurnSocket;

   // These are only set once, then accessed - thus they do not require mutex protection
   UInt8 mAllocationProps;
   UInt64 mReservationToken; 
   bool mActiveDestinationSet;
   bool mConnectivityChecksPending;

   // Mutex to protect the following members that may be get/set from multiple threads
   resip::Mutex mMutex;
   StunTuple mReflexiveTuple;
   StunTuple mRelayTuple;
   FlowHandler* mHandler;

   resip::Data mRemoteSDPFingerprint;
   bool mIceComplete;
   resip::Data mOutgoingIceUsername;
   resip::Data mOutgoingIcePassword;

   struct IceCandidatePair
   {
      enum State
      {
         InProgress,
         Frozen,
         Waiting,
         Failed,
         Succeeded
      };
      reTurn::IceCandidate mLocalCandidate;
      reTurn::IceCandidate mRemoteCandidate;
      State mState;
   };
   std::vector<IceCandidatePair> mIceCheckList;

#ifdef USE_SSL
#ifdef USE_DTLS
   // Map to store all DtlsSockets - in forking cases there can be more than one
   std::map<reTurn::StunTuple, dtls::DtlsSocket*> mDtlsSockets;
   dtls::DtlsSocket* getDtlsSocket(const reTurn::StunTuple& endpoint);
   dtls::DtlsSocket* createDtlsSocketClient(const StunTuple& endpoint);
   dtls::DtlsSocket* createDtlsSocketServer(const StunTuple& endpoint);
#endif 
#endif

   volatile FlowState mFlowState;
   void changeFlowState(FlowState newState);
   char* flowStateToString(FlowState state);

   class ReceivedData
   {
   public:
      ReceivedData(const asio::ip::address& address, unsigned short port, boost::shared_ptr<DataBuffer>& data) :
         mAddress(address), mPort(port), mData(data) {}
      ~ReceivedData() {}

      asio::ip::address mAddress;
      unsigned short mPort;
      boost::shared_ptr<DataBuffer> mData;
   };
   // FIFO for received data
   typedef resip::TimeLimitFifo<ReceivedData> ReceivedDataFifo;
   ReceivedDataFifo mReceivedDataFifo; 

   // Helpers to perform SRTP protection/unprotection
   bool processSendData(boost::shared_ptr<reTurn::DataBuffer>& buffer, const asio::ip::address& address, unsigned short port);
   asio::error_code processReceivedData(char* buffer, unsigned int& size, ReceivedData* receivedData, asio::ip::address* sourceAddress=0, unsigned short* sourcePort=0);

   FakeSelectSocketDescriptor mFakeSelectSocketDescriptor;

   virtual void onConnectSuccess(unsigned int socketDesc, const asio::ip::address& address, unsigned short port);
   virtual void onConnectFailure(unsigned int socketDesc, const asio::error_code& e);

   virtual void onSharedSecretSuccess(unsigned int socketDesc, const char* username, unsigned int usernameSize, const char* password, unsigned int passwordSize);
   virtual void onSharedSecretFailure(unsigned int socketDesc, const asio::error_code& e);

   virtual void onBindSuccess(unsigned int socketDesc, const StunTuple& reflexiveTuple, const StunTuple& stunServerTuple);
   virtual void onBindFailure(unsigned int socketDesc, const asio::error_code& e, const StunTuple& stunServerTuple);

   virtual void onIncomingBindRequestProcessed(unsigned int socketDest, const StunTuple& sourceTuple, const reTurn::StunMessage& bindRequest);

   virtual void onAllocationSuccess(unsigned int socketDesc, const StunTuple& reflexiveTuple, const StunTuple& relayTuple, unsigned int lifetime, unsigned int bandwidth, UInt64 reservationToken);
   virtual void onAllocationFailure(unsigned int socketDesc, const asio::error_code& e);

   virtual void onRefreshSuccess(unsigned int socketDesc, unsigned int lifetime);
   virtual void onRefreshFailure(unsigned int socketDesc, const asio::error_code& e);

   virtual void onSetActiveDestinationSuccess(unsigned int socketDesc);
   virtual void onSetActiveDestinationFailure(unsigned int socketDesc, const asio::error_code &e);
   virtual void onClearActiveDestinationSuccess(unsigned int socketDesc);
   virtual void onClearActiveDestinationFailure(unsigned int socketDesc, const asio::error_code &e);

   virtual void onChannelBindRequestSent(unsigned int socketDesc, unsigned short channelNumber) {}; 
   virtual void onChannelBindSuccess(unsigned int socketDesc, unsigned short channelNumber) {};
   virtual void onChannelBindFailure(unsigned int socketDesc, const asio::error_code& e) {};

   virtual void onReceiveSuccess(unsigned int socketDesc, const asio::ip::address& address, unsigned short port, boost::shared_ptr<DataBuffer>& data);
   virtual void onReceiveFailure(unsigned int socketDesc, const asio::error_code& e);

   virtual void onSendSuccess(unsigned int socketDesc);
   virtual void onSendFailure(unsigned int socketDesc, const asio::error_code& e);
};

}

#endif


/* ====================================================================

 Copyright (c) 2007-2008, Plantronics, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are 
 met:

 1. Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer. 

 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution. 

 3. Neither the name of Plantronics nor the names of its contributors 
    may be used to endorse or promote products derived from this 
    software without specific prior written permission. 

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ==================================================================== */
