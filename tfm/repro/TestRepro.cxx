#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "repro/BerkeleyDb.hxx"
#include "repro/ReproServerAuthManager.hxx"
#include "repro/monkeys/AmIResponsible.hxx"
#include "repro/monkeys/DigestAuthenticator.hxx"
#include "repro/monkeys/IsTrustedNode.hxx"
#include "repro/monkeys/LocationServer.hxx"
#include "repro/monkeys/StaticRoute.hxx"
#include "repro/monkeys/StrictRouteFixup.hxx"
#include "repro/monkeys/OutboundTargetHandler.hxx"
#include "repro/monkeys/QValueTargetHandler.hxx"
#include "repro/monkeys/SimpleTargetHandler.hxx"
#include "rutil/GeneralCongestionManager.hxx"
#include "rutil/Logger.hxx"
#include "resip/stack/InteropHelper.hxx"
#include "tfm/repro/TestRepro.hxx"
#include "repro/UserAuthGrabber.hxx"
#ifdef USE_SSL
#include "resip/stack/ssl/Security.hxx"
#endif

using namespace resip;
using namespace repro;

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::TEST

TfmProxyConfig::TfmProxyConfig(AbstractDb* db, const CommandLineParser& args)
{
   createDataStore(db);

   // Assign settings
   insertConfigValue("HttpPort", "5080");
   insertConfigValue("HttpHostname", "localhost");
   insertConfigValue("DisableIdentity", "false");
   insertConfigValue("DisableAuthInt", "false");
   insertConfigValue("RejectBadNonces", "false");
   insertConfigValue("ParallelForkStaticRoutes", "true");

   insertConfigValue("QValueBehavior", "EQUAL_Q_PARALLEL");
   insertConfigValue("QValueCancelBetweenForkGroups", "true");
   insertConfigValue("QValueWaitForTerminateBetweenForkGroups", "true");
   insertConfigValue("QValueMsBetweenForkGroups", "5000");
   insertConfigValue("QValueMsBeforeCancel", "5000");

   insertConfigValue("ForceRecordRouting", "false");
   insertConfigValue("RecordRouteUri", resip::Data::from(args.mRecordRoute));
}

static ProcessorChain&  
makeRequestProcessorChain(ProcessorChain& chain, 
                          ProxyConfig& config,
                          Dispatcher* authRequestDispatcher,
                          RegistrationPersistenceManager& regData,
                          SipStack* stack)
{
   ProcessorChain* locators = new ProcessorChain(Processor::REQUEST_CHAIN);
   
   ProcessorChain* authenticators = new ProcessorChain(Processor::REQUEST_CHAIN);
   
   IsTrustedNode* isTrusted = new IsTrustedNode(config);
   authenticators->addProcessor(std::auto_ptr<Processor>(isTrusted));

   DigestAuthenticator* da = new DigestAuthenticator(config, authRequestDispatcher);
   authenticators->addProcessor(std::auto_ptr<Processor>(da)); 

   StrictRouteFixup* srf = new StrictRouteFixup;
   locators->addProcessor(std::auto_ptr<Processor>(srf));

   AmIResponsible* isme = new AmIResponsible;
   locators->addProcessor(std::auto_ptr<Processor>(isme));
      
   StaticRoute* sr = new StaticRoute(config);
   locators->addProcessor(std::auto_ptr<Processor>(sr));
 
   LocationServer* ls = new LocationServer(config, regData, authRequestDispatcher);
   locators->addProcessor(std::auto_ptr<Processor>(ls));
 
   chain.addProcessor(std::auto_ptr<Processor>(authenticators));
   chain.addProcessor(std::auto_ptr<Processor>(locators));
   return chain;
}

static ProcessorChain&  
makeResponseProcessorChain(ProcessorChain& chain,
                           RegistrationPersistenceManager& regData) 
{
   ProcessorChain* lemurs = new ProcessorChain(Processor::RESPONSE_CHAIN);

   OutboundTargetHandler* ob = new OutboundTargetHandler(regData);
   lemurs->addProcessor(std::auto_ptr<Processor>(ob));

   chain.addProcessor(std::auto_ptr<Processor>(lemurs));
   return chain;
}

static ProcessorChain&  
makeTargetProcessorChain(ProcessorChain& chain, ProxyConfig& config) 
{
   ProcessorChain* baboons = new ProcessorChain(Processor::TARGET_CHAIN);

   QValueTargetHandler* qval =  new QValueTargetHandler(config);
   baboons->addProcessor(std::auto_ptr<Processor>(qval));
   
   SimpleTargetHandler* smpl = new SimpleTargetHandler;
   baboons->addProcessor(std::auto_ptr<Processor>(smpl));
   
   chain.addProcessor(std::auto_ptr<Processor>(baboons));
   return chain;
}

static Uri  
makeUri(const resip::Data& domain, int port)
{
   Uri uri;
   uri.host() = domain;
   if (port != 5060)
   {
      uri.port() = port;
   }
   return uri;
}


TestRepro::TestRepro(const resip::Data& name,
                     const resip::Data& host, 
                     const CommandLineParser& args, 
                     const resip::Data& nwInterface,
                     Security* security) : 
   TestProxy(name, host, args.mUdpPorts, args.mTcpPorts, args.mTlsPorts, args.mDtlsPorts, nwInterface),
#ifdef USE_SIGCOMP
   mStack(security,
            DnsStub::EmptyNameserverList,
            0,
            false,
            0,
            new Compression(Compression::DEFLATE)),
#else
   mStack(security),
#endif
   mStackThread(mStack),
   mRegistrar(),
   mProfile(new MasterProfile),
   mDb(new BerkeleyDb),
   mConfig(mDb, args),
   mAuthRequestDispatcher(new Dispatcher(std::auto_ptr<Worker>(new UserAuthGrabber(mConfig.getDataStore()->mUserStore)),
                                         &mStack, 2)),
   mRequestProcessors(Processor::REQUEST_CHAIN),
   mResponseProcessors(Processor::RESPONSE_CHAIN),
   mTargetProcessors(Processor::TARGET_CHAIN),
   mRegData(),
   mProxy(mStack, 
          mConfig,
          makeRequestProcessorChain(mRequestProcessors, mConfig, mAuthRequestDispatcher, mRegData,&mStack),
          makeResponseProcessorChain(mResponseProcessors,mRegData),
          makeTargetProcessorChain(mTargetProcessors,mConfig)),
   mDum(mStack),
   mDumThread(mDum)
{
   resip::InteropHelper::setRRTokenHackEnabled(args.mEnableFlowTokenHack);
   resip::InteropHelper::setOutboundSupported(true);
   resip::InteropHelper::setOutboundVersion(5626); // RFC 5626

   mProxy.addDomain("localhost");

   // !bwc! TODO Once we have something we _do_ support, put that here.
   mProxy.addSupportedOption("p-fakeoption");
   mStack.addAlias("localhost",5060);
   mStack.addAlias("localhost",5061);

   std::list<resip::Data> domains;
   domains.push_back("127.0.0.1");
   domains.push_back("localhost");
   
   try
   {
      mStack.addTransport(UDP, 
                           5060, 
                           V4,
                           StunDisabled,
                           nwInterface,
                           resip::Data::Empty,
                           resip::Data::Empty,
                           resip::SecurityTypes::TLSv1,
                           0);
   }
   catch(...)
   {}

   try
   {
      mStack.addTransport(TCP, 
                           5060, 
                           V4,
                           StunDisabled,
                           nwInterface,
                           resip::Data::Empty,
                           resip::Data::Empty,
                           resip::SecurityTypes::TLSv1,
                           0);
   }
   catch(...)
   {}

#ifdef RESIP_USE_SCTP
   try
   {
      mStack.addTransport(SCTP, 
                           5060, 
                           V4,
                           StunDisabled,
                           "0.0.0.0",// multihomed
                           resip::Data::Empty,
                           resip::Data::Empty,
                           resip::SecurityTypes::TLSv1,
                           0);
   }
   catch(...)
   {}
#endif

#ifdef USE_SSL
   std::list<resip::Data> localhost;
   localhost.push_back("localhost");
   
   try
   {
      mStack.addTransport(TLS, 
                           5061, 
                           V4, 
                           StunDisabled, 
                           nwInterface, 
                           "localhost",
                           resip::Data::Empty,
                           resip::SecurityTypes::TLSv1,
                           0);
   }
   catch(...)
   {}
#endif
   mProxy.addDomain(host);
   
   std::vector<Data> enumSuffixes;
   enumSuffixes.push_back(args.mEnumSuffix);
   mStack.setEnumSuffixes(enumSuffixes);

   mProxy.addSupportedOption("outbound");
   mProxy.addSupportedOption("p-fakeoption");

   mProfile->clearSupportedMethods();
   mProfile->addSupportedMethod(resip::REGISTER);
   mProfile->addSupportedScheme(Symbols::Sips);

   mDum.setMasterProfile(mProfile);
   mDum.setServerRegistrationHandler(&mRegistrar);
   mDum.setRegistrationPersistenceManager(&mRegData);
   mDum.addDomain(host);
   
   // Install rules so that the registrar only gets REGISTERs
   resip::MessageFilterRule::MethodList methodList;
   methodList.push_back(resip::REGISTER);

   resip::MessageFilterRuleList ruleList;
   ruleList.push_back(MessageFilterRule(resip::MessageFilterRule::SchemeList(),
                                        resip::MessageFilterRule::Any,
                                        methodList) );
   mDum.setMessageFilterRuleList(ruleList);
    
   SharedPtr<ServerAuthManager> authMgr(new ReproServerAuthManager(mDum, 
                                                                   mAuthRequestDispatcher,
                                                                   mConfig.getDataStore()->mAclStore, 
                                                                   true, 
                                                                   false,
                                                                   true));
   mDum.setServerAuthManager(authMgr);    

   mStack.registerTransactionUser(mProxy);

   if(args.mUseCongestionManager)
   {
      mCongestionManager.reset(new GeneralCongestionManager(
                                          GeneralCongestionManager::WAIT_TIME, 
                                          200));
      mStack.setCongestionManager(mCongestionManager.get());
   }

   if(args.mThreadedStack)
   {
      mStack.run();
   }

   mStackThread.run();
   mProxy.run();
   mDumThread.run();
}

TestRepro::~TestRepro()
{
   mDumThread.shutdown();
   mDumThread.join();
   delete mAuthRequestDispatcher;
   mStackThread.shutdown();
   mStackThread.join();
   mStack.shutdownAndJoinThreads();
   mStack.setCongestionManager(0);
   delete mDb;
}

void
TestRepro::addUser(const Data& userid, const Uri& aor, const Data& password)
{
   InfoLog (<< "Repro::addUser: " << userid << " " << aor);
   mConfig.getDataStore()->mUserStore.addUser(userid,aor.host(),aor.host(), password, true, Data::from(aor), Data::from(aor));
}

void
TestRepro::deleteUser(const Data& userid, const Uri& aor)
{
   //InfoLog (<< "Repro::delUser: " << userid);
   mConfig.getDataStore()->mUserStore.eraseUser(userid);
   mRegData.removeAor(aor);
}

void
TestRepro::deleteBindings(const Uri& aor)
{
   //InfoLog (<< "Repro::delBindings: " << aor);
   mRegData.removeAor(aor);
}

void 
TestRepro::addRoute(const resip::Data& matchingPattern,
                    const resip::Data& rewriteExpression, 
                    const resip::Data& method,
                    const resip::Data& event,
                    int priority,
                    int weight) 
{
   mConfig.getDataStore()->mRouteStore.addRoute(method, event, matchingPattern, rewriteExpression, priority);
}

void 
TestRepro::deleteRoute(const resip::Data& matchingPattern, 
                       const resip::Data& method, 
                       const resip::Data& event)
{
   mConfig.getDataStore()->mRouteStore.eraseRoute(method, event, matchingPattern);
}

bool
TestRepro::addTrustedHost(const resip::Data& host, resip::TransportType transport, short port)
{
   return mConfig.getDataStore()->mAclStore.addAcl(host, port, static_cast<const short&>(transport));
}

