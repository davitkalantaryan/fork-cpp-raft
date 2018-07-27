

#ifndef __common_raft_tcp_server_hpp__
#define __common_raft_tcp_server_hpp__

#include <common/common_servertcp.hpp>
#include <raft_server.h>
#include "raft_tcp_common.hpp"
#include <string>
//#include <unordered_map>
#include <raft_node.h>
#include <stdint.h>
#include "raft_macroses_and_functions.h"
#include "common/common_unnamedsemaphorelite.hpp"
#include "common/common_fifofast.hpp"
#include <vector>
#if defined(HANDLE_SIG_ACTIONS) || defined(WLAC_USED)
#include <pthread.h>
#endif
#include <cpp11+/thread_cpp11.hpp>

#define  NUMBER_OF_TOOLS_SOCKETS	2

namespace raft { namespace tcp{

namespace workRequest{enum Type{none,handleConnection,handleReceive,handleInternal};}
struct SWorkerData { 
	workRequest::Type reqType;
	RaftNode2*	pNode;
	union {
		struct{
			sockaddr_in	remAddress;
			int			sockDescriptor;
		}con;
		struct{
			int32_t		m_index;
		}rcv;
		struct{
			char		cRequest;
		}intr;
	}pear;
	/*-------------------------------*/
	SWorkerData(workRequest::Type a_reqType=workRequest::none) { this->reqType = a_reqType; this->pNode = NULL; this->pear.con.sockDescriptor = -1;memset(&this->pear.con.remAddress,0,sizeof(sockaddr_in)); }
};


namespace socketTypes {
	enum Type {raft,data1,last};
}

class NodeTools 
{
	common::SocketTCP	m_sockets[NUMBER_OF_TOOLS_SOCKETS];
	STDN::mutex			m_socketSendMutexes[NUMBER_OF_TOOLS_SOCKETS];
	STDN::mutex			m_socketRcvMutexes[NUMBER_OF_TOOLS_SOCKETS];
public:
	int writeC(int32_t index, RaftNode2* a_pNode, int32_t a_nPort, const void* data, int dataLen);
	int readC(int32_t index, void* buffer, int bufferLen);
	STDN::mutex* senderMutex(size_t index);
	STDN::mutex* receiverMutex(size_t index);
	void setSocket(int32_t index, int socketNum);
	int getSocketNum(int32_t index);
	common::SocketTCP* socketPtr(int32_t index);
	
	void*		childData;
	uint32_t	isEndianDiffer : 1;
	/*-----------------------------------------*/
	NodeTools(){ this->childData = NULL; this->isEndianDiffer = 0; }
};

#define GET_NODE_TOOLS(_node)	((raft::tcp::NodeTools*)((_node)->get_udata()))
#define GET_CLBK_DATA(_node)	(GET_NODE_TOOLS((_node))->childData)
#define NODE_KEY(_node)			((raft::tcp::NodeIdentifierKey*)((_node)->key))

#define PREPARE_SOCKET_GUARD()						common::NewLockGuard<std::mutex> __aDataSendLockGuard
#define LOCK_SEND_SOCKET_MUTEX(_node,_index)		__aDataSendLockGuard.SetAndLockMutex(GET_NODE_TOOLS((_node))->senderMutex((_index)))
#define LOCK_RCV_SOCKET_MUTEX(_node,_index)			__aDataSendLockGuard.SetAndLockMutex(GET_NODE_TOOLS((_node))->receiverMutex((_index)))
#define UNLOCK_SOCKET_MUTEX()						__aDataSendLockGuard.UnsetAndUnlockMutex()

extern int g_nApplicationRun;

class Server : protected RaftServer
{
public:
	Server();
	virtual ~Server();

	int					RunServerOnOtherThreads2(const std::vector<NodeIdentifierKey>& vectPossibleNodes, int workersCount, int raftPort=-1);
	void				StopServer();

	static void			Initialize();
	static void			Cleanup();

protected:
	virtual void		become_leader() OVERRIDE;
	virtual void		become_candidate()OVERRIDE;

	virtual void		AddAdditionalDataToNode(RaftNode2* newNode, std::string* a_pDataFromAdder, bool a_bAdder)OVERRIDE;
	virtual void		CleanNodeData(RaftNode2*, std::string* a_pDataFromLeader) OVERRIDE;
		
	virtual void		SignalHandler(int sigNum);

	virtual bool		handleNewConnectionBeforeLock(common::SocketTCP& a_socket, const sockaddr_in&a_remoteAddr, char a_cRequest, NodeIdentifierKey* a_newNodeKey,std::string* a_pDataFromClient);
	virtual RaftNode2*	handleNewConnectionLocked(common::SocketTCP& a_socket, const sockaddr_in&a_remoteAddr, char a_cRequest, NodeIdentifierKey* a_newNodeKey, std::string* a_pDataFromClient);
	virtual void		handleNewConnectionAfterLock(common::SocketTCP& a_socket, const sockaddr_in&a_remoteAddr, char a_cRequest, NodeIdentifierKey* a_newNodeKey, std::string* a_pDataFromClient, RaftNode2* a_pNodeToSkip);

	virtual bool		handleReceiveFromNodeBeforeLock(char cRequest,RaftNode2* pNode, int32_t index, NodeIdentifierKey* a_pNodeKey, std::string* a_bBufferForReceive);
	virtual bool		handleReceiveFromNodeLocked(char cRequest, RaftNode2* pNode, int32_t index, NodeIdentifierKey* a_pNodeKey, std::string* a_bBufferForReceive);
	virtual void		handleReceiveFromNodeAfterLock(char cRequest, RaftNode2* pNode, int32_t index, NodeIdentifierKey* a_pNodeKey, std::string* a_bBufferForReceive);

	virtual bool		handleInternalBeforeLock(char cRequest, RaftNode2* a_pNode, NodeIdentifierKey* a_pNodeKey, std::string* a_pBufferToSendToOthers);
	virtual bool		handleInternalLocked(char cRequest, RaftNode2* a_pNode, NodeIdentifierKey* a_pNodeKey, std::string* a_pBufferToSendToOthers);
	virtual void		handleInternalAfterLock(char cRequest, RaftNode2* a_pNode, NodeIdentifierKey* a_pNodeKey, std::string* a_pBufferToSendToOthers);

	void				ThreadFunctionFindOtherChains();
	void				ThreadFunctionListen();
	void				ThreadFunctionPeriodic();
	void				ThreadFunctionRcvFromSocket(int32_t index);
	void				ThreadFunctionWorker();
	template <typename Type>
	void				ThreadFunctionOtherPeriodic(void (Type::*a_fpClbk)(int), int a_nPeriod, Type* a_pObj, int a_nIndex);
	template <typename Type>
	void				StartOtherPriodic(void (Type::*a_fpClbk)(int), int a_nPeriod, Type* a_pObj);

	void				AddConnectForWorker(const sockaddr_in* a_pRemote, int socketDescr);
	void				AddInternalForWorker(char a_cRequest, RaftNode2* a_pNode);
	void				AddReceiveForWorker(RaftNode2* a_pNode, int32_t sockIndex);

	bool				SendInformationToNode(RaftNode2* a_pNode, int32_t a_index, char a_cRequest, const std::string* a_extraData, const NodeIdentifierKey* a_pNodeKey);
	void				SendInformationToAllNodes(int32_t a_index, char a_cRequest, const std::string* a_extraData, const NodeIdentifierKey* a_pNodeKey, RaftNode2* a_pNodeToSkip, bool a_bWait);
    void				FunctionForMultiRcv(void (Server::*a_fpRcvFnc)(RaftNode2*,int32_t),int32_t index);
	void				AddClient(common::SocketTCP& clientSock, const sockaddr_in*remoteAddr);

	NodeIdentifierKey*	TryFindClusterThredSafe(const NodeIdentifierKey& nodeInfo, std::string* a_extraDataForAndFromAdder);
	void				AddOwnNode(bool a_bIsLeader, std::string* a_pAdderInfo);

	// family of receive functions
	bool				raft_receive_fromAdder_newNode(RaftNode2* a_pNode, std::string* a_bufferForAdderData, NodeIdentifierKey* a_pNewNodeKey);
	bool				raft_receive_fromLeader_removeNode(RaftNode2* a_pNode, std::string* a_extraDataFromLeader, NodeIdentifierKey* a_pNewNodeKey);

	// utils
	void				raft_connect_toAnyNode_permanentBridge(common::SocketTCP& sock, const sockaddr_in* remoteAddr); // (index=0)->raft
	bool				raft_connect_toAnyNode_newNode(common::SocketTCP& sock, const sockaddr_in*remoteAddr, std::string* extraDataFromNewNode, NodeIdentifierKey* a_newNodeKey);
	void				raft_connect_fromClient_allNodesInfo(common::SocketTCP& sock);
	void				raft_connect_toAnyNode_otherLeaderFound(common::SocketTCP& sock);

	void				CheckAllPossibleSeeds(const std::vector<NodeIdentifierKey>& vectPossibleNodes, std::string* a_extraDataForAndFromAdder);

	void				HandleSeedClbk(RaftNode2* anyNode);

    void				InterruptPeriodicThread();
    void				InterruptReceivercThread(int32_t index);
	bool				ReceiveExtraData(common::SocketTCP& a_sockt, uint32_t a_isEndianDiffer, std::string* a_pBufForData);

	NodeIdentifierKey*	CollectAllNodesDataNotThrSafe(int32_t* pnTotalSize, int32_t* a_pnLeaderIndex);
	void				FindClusterAndInit(const std::vector<NodeIdentifierKey>& vectPossibleNodes, std::string* a_extraDataForAndFromAdder, int raftPort=-1);
	void				RunAllThreadPrivate(int32_t workersCount);

private:
	void				ReceiveFromSocketAndInform(RaftNode2* pNode, int32_t index);
	void				HandleNewConnectionPrivate(int a_nSocketDescr, const sockaddr_in&remoteAddr, NodeIdentifierKey* a_newNodeKey, std::string* a_pDataFromClient);
	void				HandleReceiveFromNodePrivate(RaftNode2* pNode, int32_t index, NodeIdentifierKey* a_pNodeKey, std::string* a_bBufferForReceive);
	void				HandleInternalPrivate(char cRequest, RaftNode2* a_pNode, NodeIdentifierKey* a_pNodeKey, std::string* a_bBufferForReceive);

	void				AddJobForWorkerPrivate(workRequest::Type a_type, char a_cRequest, RaftNode2* a_pNode, const sockaddr_in* a_pRemote, int32_t sockIndex);

protected:
	static int	SendClbkFunction(void *cb_ctx, void *udata, RaftNode2* node, int msg_type, const unsigned char *send_data, int d_len);
	static void LogClbkFunction(void *cb_ctx, void *src, const char *buf, ...);
	static int	ApplyLogClbkFunction(void *cb_ctx, void *udata, const unsigned char *d_data, int d_len);
	static void SigHandlerStatic(int a_nSigNum);

protected:
	common::ServerTCP								m_serverTcp;
	STDN::thread									m_threadFixDoubleCycles;
	STDN::thread									m_threadTcpListen;
	STDN::thread									m_threadPeriodic;
	STDN::thread									m_threadsReceives[NUMBER_OF_TOOLS_SOCKETS];
	std::vector<STDN::thread*>						m_vectThreadsWorkers;
	std::vector<STDN::thread*>						m_vectThreadsOtherPeriodic;
private:
    STDN::shared_mutex                              m_shrdMutexForNodes;
	common::UnnamedSemaphoreLite					m_semaWorker;
	common::UnnamedSemaphoreLite					m_semaForSolvingDublicates2;
	common::FifoFast<SWorkerData>					m_fifoWorker;
protected:
	volatile int									m_nWork;
	int												m_nPeriodForPeriodic;
	int												m_nPortOwn;
	RaftNode2*										m_pLeaderNode;
    volatile int									m_intrptSocketForRcv[NUMBER_OF_TOOLS_SOCKETS];

#ifdef _WIN32
	HANDLE											m_periodicThreadId;
#else
	pthread_t										m_periodicThreadId;
    pthread_t                                       m_rcvThreadIds[NUMBER_OF_TOOLS_SOCKETS];
#endif

public:
    void*                                           m_pReserved1;
#if defined(HANDLE_SIG_ACTIONS) || defined(WLAC_USED)
    pthread_t                                       m_starterThread;
#endif

	timeb											m_lastPingByLeader;
	uint64_t										m_isInited : 1;
	std::vector<NodeIdentifierKey>					m_allPossibleNodes;
};

}} // namespace raft { namespace tcp{

#include "impl.raft_tcp_server.hpp"

#endif  // #ifndef __common_raft_tcp_server_hpp__

